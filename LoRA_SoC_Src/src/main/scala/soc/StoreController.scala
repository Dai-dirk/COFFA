package fgramemfp

import chisel3._
import chisel3.util._
import freechips.rocketchip.tile._
import org.chipsalliance.cde.config._
import freechips.rocketchip.rocket._
import FgramemISA._

/** Store controller, transfer data from local scratchpad (physical address) to remote memory (virtual address)
  * @param spadAddrWidth  scratchpad address width, <= 32
  * @param lgMaxPartitionSize       log2(maximum of partitioned size N)  
  * @param lgMaxDataLen   log2(the max data length in one DMA request)
  * @param dataWidth      data width in bits
  * @param fgradataWidth  fgra data width in bits
  * @param hasMask        if has mask signal in the stream interface
  * @param idWidth        width of the ID for a DMA request
  * @param streamQueDepth stream queue depth, can be zero
  */
class StoreController(spadAddrWidth: Int, lgMaxPartitionSize: Int, lgMaxDataLen: Int, dataWidth: Int, fgradataWidth: Int, hasMask: Boolean, idWidth: Int, streamQueDepth: Int)
                    (implicit p: Parameters) extends CoreModule {
  val io = IO(new Bundle {
    val core = new CoreIF(idWidth)
    // val dma = Flipped(new DMAStreamWriteIF(lgMaxDataLen, dataWidth, hasMask, idWidth))
    //@yuan: add mask to control the transmission with bias
    val dma = Flipped(new DMAStreamWriteIF(lgMaxDataLen, dataWidth, true, idWidth))
    val spad = Flipped(new SpadStreamReadIF(spadAddrWidth, lgMaxPartitionSize, lgMaxDataLen, dataWidth, hasMask, idWidth))
  })

  val maxPrtitionSize = 1 << lgMaxPartitionSize
  // decouple the DMA requests with the data streams, @yuan: for multiple store instructions
  // class SpadReq(spadAddrWidth: Int, lgMaxPartitionSize: Int, lgMaxDataLen: Int, idWidth: Int) extends Bundle{
  //   val addrs = Vec(maxPrtitionSize, UInt(spadAddrWidth.W))   // start addresses
  //   val num = UInt(lgMaxPartitionSize.W) // used address number - 1
  //   val len = UInt(lgMaxDataLen.W)
  //   val id = UInt(idWidth.W)
  // }

  val dataByte = dataWidth/8
  // val s_idle :: s_spad_req :: Nil = Enum(2)
  val s_idle :: s_spad_req :: s_pre_resp :: s_dma_req :: s_stream_pre :: s_stream :: s_exp :: s_resp :: Nil = Enum(8)
  val state = RegInit(s_idle)
  val remote_addr = RegInit(0.U(coreMaxAddrBits.W)) // remote DRAM virtual address
  // val spad_addr = RegInit(0.U(spadAddrWidth.W)) // scratchpad address from rs1
  val spad_addrs = RegInit(VecInit(Seq.fill(maxPrtitionSize){0.U(spadAddrWidth.W)})) // scratchpad address from rs1
  val bank_offsets = RegInit(VecInit(Seq.fill(maxPrtitionSize){0.U(lgMaxPartitionSize.W)})) //@yuan: bank offsets from rs2
  val partition_num = RegInit(0.U(lgMaxPartitionSize.W))
  val lgPartitionSize = RegInit(0.U(2.W)) //@yuan: for now, the maximum partition size should less than 8 (3)
  val lgPartitionBlockSize = RegInit(0.U(4.W)) // @yuan: the log2(partition block size), i.e. log2(B)
  val leftLen = RegInit(0.U(lgMaxDataLen.W)) // left data length
  val fused = RegInit(false.B) // can be fused with next store command, with the same remote_addr, total_len
//  val partitionMode = RegInit(0.U(1.W))
  val status = Reg(new MStatus)
  val id = RegInit(0.U(idWidth.W))
  val success = RegInit(true.B)
  val rs1 = io.core.req.bits.cmd.rs1.asTypeOf(new StoreRs1)
  val rs2 = io.core.req.bits.cmd.rs2.asTypeOf(new StoreRs2)
  val leftNum = RegInit(0.U(lgMaxPartitionSize.W))
  // val pre_resp = RegInit(0.U(1.W))
  // assert(maxPrtitionSize >= 1, "partition address number should >1")

  //@yuan: for the transmission with bias
  val bias_element_num_wire = rs1.remote_addr(log2Ceil(dataWidth/8) - 1, log2Ceil(fgradataWidth/8))
  val bias_element_num = RegInit(0.U(log2Ceil(dataWidth/fgradataWidth) .W))
  val shift_num_wire = rs1.remote_addr(log2Ceil(dataWidth/8) - 1, 0)
  val shift_num = RegInit(0.U(log2Ceil(dataWidth/8) .W))
  val initLeftLen = RegInit(0.U(lgMaxDataLen.W))

  switch(state){
    is(s_idle){
      when(io.core.req.fire){
        remote_addr := rs1.remote_addr
        // remote_addr := Cat(rs1.remote_addr(coreMaxAddrBits - 1, log2Ceil(128/8)), 0.U(log2Ceil(128/8).toInt.W))
        spad_addrs(partition_num) := rs2.spad_addr
        bank_offsets(partition_num) := partition_num // @yuan: bank offset
//        partitionMode := rs2.partition_mode
        lgPartitionSize := rs2.lgPartitionSize
        leftNum := partition_num
        lgPartitionBlockSize := rs2.lgPartitionBlockSize
        leftLen := rs2.total_len
        initLeftLen := rs2.total_len
        // leftLen := Mux(bias_element_num_wire === 0.U, rs2.total_len, rs2.total_len + dataByte.U) 
        bias_element_num := bias_element_num_wire
        // when(rs2.total_len < dataByte.U){
        //   shift_num := dataByte.U
        // }.otherwise{
        //   shift_num := shift_num_wire
        // }
        shift_num := shift_num_wire
        fused := rs2.fused.asBool
        status := io.core.req.bits.cmd.status
        id := io.core.req.bits.id
        when(!rs2.fused.asBool || partition_num === (maxPrtitionSize-1).U){
          when(partition_num > 0.U){
            state := s_pre_resp
          }.otherwise{
            state := s_spad_req //@yuan: next state is to send request to SPM            
          }
        }.otherwise{
          state := s_idle
          partition_num := partition_num + 1.U
        }
      }
    }
    is(s_pre_resp){
      when(io.core.resp.fire){
        leftNum := leftNum - 1.U
        when(leftNum === 1.U){
          state := s_spad_req
        }
      }
    }

    is(s_spad_req){
      when(io.spad.req.fire){
        state := s_dma_req
      }
    }
    is(s_dma_req){
      when(io.dma.req.fire){
        //@yuan: reset the partition_num
        partition_num := 0.U
        state := s_stream_pre
      }
    }
    is(s_stream_pre){
      when(io.dma.exp.req){
        state := s_exp
        success := false.B
      }.elsewhen(io.spad.stream.fire){
        leftLen := leftLen - dataByte.U
        state := Mux(leftLen <= dataByte.U, s_resp, s_stream)
      }
    }
    is(s_stream){
      when(io.dma.exp.req){
        state := s_exp
        success := false.B
      }.elsewhen(io.spad.stream.fire){
        leftLen := leftLen - dataByte.U
        state := Mux(leftLen <= dataByte.U, s_resp, s_stream)
      }
    }
    is(s_exp){
      when(io.spad.exp.ack){
        state := s_resp
      }
    }
    is(s_resp){
      when(io.core.resp.fire){
        state := s_idle
      }
    }
  }

  // switch(state){
  //   is(s_idle){
  //     when(io.core.req.fire){
  //       remote_addr := rs1.remote_addr
  //       spad_addrs(addr_num) := rs2.spad_addr
  //       leftLen := rs2.total_len
  //       fused := rs2.fused.asBool

  //       state := s_spad_req
  //       remote_addr := rs1.remote_addr
  //       // spad_addr := rs2.spad_addr
  //       status := io.core.req.bits.cmd.status
  //       id := io.core.req.bits.id
  //       success := true.B
  //     }
  //   }
  //   is(s_spad_req){
  //     when(io.spad.req.fire){
  //       state := s_dma_req
  //     }
  //   }
  //   is(s_dma_req){
  //     when(io.dma.req.fire){
  //       state := s_stream
  //     }
  //   }
  //   is(s_stream){
  //     when(io.dma.exp.req){
  //       state := s_exp
  //       success := false.B
  //     }.elsewhen(io.spad.stream.fire){
  //       leftLen := leftLen - dataByte.U
  //       state := Mux(leftLen <= dataByte.U, s_resp, s_stream)
  //     }
  //   }
  //   is(s_exp){
  //     when(io.spad.exp.ack){
  //       state := s_resp
  //     }
  //   }
  //   is(s_resp){
  //     when(io.core.resp.fire){
  //       state := s_idle
  //     }
  //   }
  // }

  io.core.req.ready := (state === s_idle)
  // Scratchpad request
  io.spad.req.valid := (state === s_spad_req)
  io.spad.req.bits.addrs := spad_addrs
  io.spad.req.bits.len := leftLen
  io.spad.req.bits.id := id
  io.spad.req.bits.lgPartitionSize := lgPartitionSize//@yuan: the size of partitioned memory banks
  io.spad.req.bits.lgPartitionBlockSize := lgPartitionBlockSize
  io.spad.req.bits.offsetId := bank_offsets
//  io.spad.req.bits.mode := partitionMode
  // DMA request
  io.dma.req.valid := (state === s_dma_req)
  io.dma.req.bits.addr := remote_addr
  io.dma.req.bits.len := leftLen
  io.dma.req.bits.id := id
  io.dma.req.bits.status := status
  // data stream
  if(streamQueDepth == 0){
    // io.spad.stream.ready := io.dma.stream.ready && (state === s_stream)
    // io.dma.stream.valid := io.spad.stream.valid && (state === s_stream)
    io.spad.stream.ready := io.dma.stream.ready && (state === s_stream || state === s_stream_pre)
    io.dma.stream.valid := io.spad.stream.valid && (state === s_stream || state === s_stream_pre)
    // io.dma.stream.bits := io.spad.stream.bits
    io.dma.stream.bits.mask := Mux(state === s_stream || state === s_stream_pre, Mux(state === s_stream_pre, ((1 << dataByte) - 1).U << shift_num , ((1 << dataByte) - 1).U), 0.U)
    io.dma.stream.bits.id := io.spad.stream.bits.id
    io.dma.stream.bits.last := io.spad.stream.bits.last
    io.dma.stream.bits.data := io.spad.stream.bits.data
  }else{
    // val que = Module(new Queue(new DMAStream(dataWidth, hasMask, idWidth), streamQueDepth,
    //   false, false, false, true))
    val que = Module(new Queue(new DMAStream(dataWidth, true, idWidth), streamQueDepth,
      false, false, false, true))
    io.spad.stream.ready := que.io.enq.ready && (state === s_stream || state === s_stream_pre)
    que.io.enq.valid := io.spad.stream.valid && (state === s_stream || state === s_stream_pre)
    // que.io.enq.bits := io.spad.stream.bits
    // que.io.enq.bits.mask := io.spad.stream.bits.mask
    //@yuan_20251114: fix bug, for some situation, if there is only one data to write back, the mask should be set correctly 
    // que.io.enq.bits.mask := Mux(state === s_stream || state === s_stream_pre, Mux(state === s_stream_pre, Mux(initLeftLen < dataByte.U, ((1.U << initLeftLen)-1.U),((1 << dataByte) - 1).U << shift_num) , ((1 << dataByte) - 1).U), 0.U)
    que.io.enq.bits.mask := Mux(state === s_stream || state === s_stream_pre, Mux(state === s_stream_pre, Mux(initLeftLen < dataByte.U, ((1.U << initLeftLen)-1.U),((1 << dataByte) - 1).U << shift_num) , Mux(leftLen < dataByte.U, ((1.U << leftLen) - 1.U), ((1 << dataByte) - 1).U)), 0.U)
    que.io.enq.bits.id := io.spad.stream.bits.id
    que.io.enq.bits.last := io.spad.stream.bits.last
    que.io.enq.bits.data := io.spad.stream.bits.data
    io.dma.stream <> que.io.deq
    que.io.flush.get := io.spad.exp.req
  }
  // exception
  io.spad.exp.req := (state === s_exp)
  io.spad.exp.id := id
  io.dma.exp.ack := io.spad.exp.ack
  // response
  io.core.resp.valid := (state === s_pre_resp) || (state === s_resp)
  io.core.resp.bits.success := success
  io.core.resp.bits.id := id
}