package fgramem

import chisel3._
import chisel3.util._
import freechips.rocketchip.tile._
import org.chipsalliance.cde.config._
import fgramem.dsa._

//// SRAM I/O ports
//class SRAMIO(dataWidth: Int, addrWidth: Int, hasMask: Boolean) extends Bundle {
//  val en   = Input(Bool())
//  val we   = Input(UInt({if(hasMask) (dataWidth/8) else 1}.W))
//  val addr = Input(UInt(addrWidth.W))
//  val din  = Input(UInt(dataWidth.W))
//  val dout = Output(UInt(dataWidth.W))
//}
//
///** True dual port SRAM
//  * @param width    data width
//  * @param lgDepth  log2(SRAM depth)
//  * @param hasMask  if has write data byte mask
//  */
//class TrueDualPortSRAM(width: Int, lgDepth: Int, hasMask: Boolean) extends Module {
//  val io = IO(new Bundle{
//    val a = new SRAMIO(width, lgDepth, hasMask)
//    val b = new SRAMIO(width, lgDepth, hasMask)
//  })
//  val ios = Array(io.a, io.b)
//  if(hasMask){
//    val maskWidth = width/8
//    val mem = SyncReadMem(1<<lgDepth, Vec(maskWidth, UInt(8.W)))
//    ios.map{ x =>
//      x.dout := DontCare
//      when(x.en) {
//        val rwPort = mem(x.addr)
//        when(x.we =/= 0.U) {
//          (0 until maskWidth).map { i =>
//            when(x.we(i).asBool) {
//              rwPort(i) := x.din(i * 8 + 7, i * 8)
//            }
//          }
//        }.otherwise {
//          x.dout := Cat(rwPort.toSeq.reverse)
//        }
//      }
//    }
//  } else {
//    val mem = SyncReadMem(1<<lgDepth, UInt(width.W))
//    ios.map{ x =>
//      x.dout := DontCare
//      when(x.en) {
//        val rwPort = mem(x.addr)
//        when(x.we.asBool) {
//          rwPort := x.din
//        }.otherwise {
//          x.dout := rwPort
//        }
//      }
//    }
//  }
//}


// Scratchpad stream request with physical address
// include multiple addresses for broadcast
class SpadWriteStreamReq(spadAddrWidth: Int, spadAddrNum: Int, lgMaxPartitionSize: Int, lgMaxLen: Int, idWidth: Int)(implicit p: Parameters) extends CoreBundle{
  val addrs = Vec(spadAddrNum, UInt((spadAddrWidth).W))   // start addresses 
  val num = UInt(log2Ceil(spadAddrNum).W) // used address number - 1
  val len = UInt(lgMaxLen.W)         // transferring data length in bytes
  val id = UInt(idWidth.W)           // can be zero
  val mode = UInt(1.W)  //@yuan: the mode of memory partition              
  val lgPartitionSize = UInt(2.W) // @yuan: the log2(partition size)
  val lgPartitionBlockSize = UInt(4.W) // @yuan: the log2(partition block size), i.e. log2(B)
  val offsetId = Vec(spadAddrNum, UInt(lgMaxPartitionSize.W))//@yuan: partitioned memory id, that is, offset against the base bank
 }
class SpadReadStreamReq(spadAddrWidth: Int, lgMaxPartitionSize: Int, lgMaxLen: Int, idWidth: Int)(implicit p: Parameters) extends CoreBundle{
  val addrs = Vec(1 << lgMaxPartitionSize, UInt(spadAddrWidth.W))   // start addresses
  val len = UInt(lgMaxLen.W)         // transferring data length in bytes
  val id = UInt(idWidth.W)           // can be zero
  val mode = UInt(1.W)  //@yuan: the mode of memory partition  
  val lgPartitionSize = UInt(2.W) // @yuan: or the log2(partition size)
  val lgPartitionBlockSize = UInt(4.W) // @yuan: the log2(partition block size), i.e. log2(B)
  val offsetId = Vec(1 << lgMaxPartitionSize, UInt(lgMaxPartitionSize.W))//@yuan: partitioned memory id, that is, offset against the base bank
}

// Scratchpad Stream read interface
class SpadStreamReadIF(spadAddrWidth: Int, lgMaxPartitionSize: Int, lgMaxLen: Int, dataWidth: Int, hasMask: Boolean, idWidth: Int)
                      (implicit p: Parameters) extends CoreBundle{
  // val req = Flipped(Decoupled(new SpadStreamReq(spadAddrWidth, 1, lgMaxLen, idWidth)))
  val req = Flipped(Decoupled(new SpadReadStreamReq(spadAddrWidth, lgMaxPartitionSize, lgMaxLen, idWidth))) //@yuan: spadAddrNum is used by partitioned memory reading operation 
  val stream = Decoupled(new DMAStream(dataWidth, hasMask, idWidth))
  val exp = Flipped(new ExceptionIF(idWidth))
}

// Scratchpad Stream write interface
class SpadStreamWriteIF(spadAddrWidth: Int, spadAddrNum: Int, lgMaxPartitionSize: Int, lgMaxLen: Int, dataWidth: Int, hasMask: Boolean, idWidth: Int)
                       (implicit p: Parameters) extends CoreBundle{
  val req = Flipped(Decoupled(new SpadWriteStreamReq(spadAddrWidth, spadAddrNum, lgMaxPartitionSize, lgMaxLen, idWidth)))
  val stream = Flipped(Decoupled(new DMAStream(dataWidth, hasMask, idWidth)))
  val exp = Flipped(new ExceptionIF(idWidth))
}
/** Programable memory interface for reading data from Scratchpad
 * @param BaseAddrWidth  address index width, the address is used to indicate the bank and offset
 * @param lgMaxPartitionSize       log2(maximum of partitioned size N)
 * @param dataWidth      data width in bits, typically, it's 128, which is the same as the width of bus
 * @param dataWidthSram  data width of the srams port, typically, it's 32, which is the same as FGRA data_width
 * @param nBanks         scratchpad bank number except the last bank
 */
class SpadReadInterface(BaseAddrWidth: Int, lgMaxPartitionSize: Int, dataWidth: Int, dataWidthSram: Int, nBanks: Int) extends Module {
  val maxPartitionSize = 1 << lgMaxPartitionSize
  val io = IO(new Bundle {
    // sram bank interfaces
    val sramDatas = Input(Vec(maxPartitionSize, UInt(dataWidth.W)))
    // enable  signal
    val enable = Input(Bool())
    // partition mode
//    val mode = Input(UInt(1.W))
    // the address is used to indicate the bank and offset
    val addrs = Input(UInt((BaseAddrWidth + lgMaxPartitionSize).W))
    // the partition size
    val lgPartitionSize = Input(UInt(2.W))
    // the partition block size
    val lgPartitionBlockSize = Input(UInt(4.W))

    val dout = Output(UInt(dataWidth.W))
  })

  if(dataWidth / dataWidthSram < 1){
    io.dout := io.sramDatas(0)
  }else{
    val N = dataWidth / dataWidthSram
    val lgN = log2Ceil(N)
    val dataGroup = Wire(Vec(N * maxPartitionSize, UInt(dataWidthSram.W)))
    val outputGroup = Wire(Vec(N, UInt(dataWidthSram.W)))
    val baseAddr = Wire(UInt((BaseAddrWidth + lgN + lgMaxPartitionSize).W))
    baseAddr := io.addrs << lgN
    when(io.enable) {
      (0 until maxPartitionSize).foreach{ i =>
        (0 until N).foreach{ j =>
          dataGroup(i*N + j) := io.sramDatas(i)((j+1)*dataWidthSram -1, j*dataWidthSram)
        }
      }
      (0 until N).foreach{ i =>
        val bankOffset = Wire(UInt(lgMaxPartitionSize.W))
        val offset = Wire(UInt(lgN.W))
        bankOffset := ((baseAddr + i.U) >> io.lgPartitionBlockSize).asUInt & ((1.U << io.lgPartitionSize).asUInt - 1.U) //@yuan: bankId = (x / B) % N
        offset := ((((((baseAddr + i.U) >> (io.lgPartitionSize + io.lgPartitionBlockSize)) << io.lgPartitionBlockSize).asUInt) + ((baseAddr + i.U) & ((1.U << io.lgPartitionBlockSize).asUInt - 1.U)))) & (N - 1).asUInt  //@yuan: exactoffset = ((x/(N*B))*B + x % B) % (M)
        val data_sel = (0 until N * maxPartitionSize).map{ j =>
          bankOffset * N.asUInt + offset === j.U
        }
        val dataMap = data_sel.zipWithIndex.map{ case (v, j) => v -> dataGroup(j)}
        outputGroup(i) := PriorityMux(dataMap)
      }
      io.dout := Cat(outputGroup.reverse)
    }.otherwise {
      io.dout := 0.U
      dataGroup := DontCare
      outputGroup := DontCare
    }
  }
}


/** Multi-bank scratchpad composed of multiple true-dual-port SRAMs
  * @param spadAddrWidth  scratchpad address width, <= 32, high bits are used to select the correct banks
  * @param spadAddrNum    scratchpad address number, be multiple to support broadcast write
  * @param lgMaxDataLen   log2(the max data length in one DMA request)
  * @param dataWidth      data width in bits, typically, it's 128, which is the same as the width of bus
  * @param hasMask        if has mask signal in the stream interface
  * @param idWidth        width of the ID for a DMA request
  * @param nBanks         scratchpad bank number except the last bank
  * @param lgSizeBank     log2(single bank size in byte), used for IOB
  * @param lgSizeLastBlock log2(last block size in byte), used for config
  * @param dataWidthSram  data width of the srams port, typically, it's 32, which is the same as FGRA data_width
  * @param lgMaxPartitionSize       log2(maximum of partitioned size N)  
  */
class Scratchpad(spadAddrWidth: Int, spadAddrNum: Int, lgMaxDataLen: Int, dataWidth: Int, hasMask: Boolean, idWidth: Int, nBanks: Int,
                 lgSizeBank: Int, lgSizeLastBlock: Int, dataWidthSram: Int, lgMaxPartitionSize: Int)
                (implicit p: Parameters) extends CoreModule {
  val dataByte = dataWidth/8 // 128/8 = 16
  val lgDataByte = log2Ceil(dataByte) // 4
  assert(isPow2(dataByte), "Data width in bytes should be power 2 values.")
  assert(spadAddrWidth >= lgMaxDataLen, "spadAddrWidth should be larger than lgMaxDataLen.")
  val lgDepthSram = lgSizeBank - log2Ceil(dataWidthSram/8) // lgSizeBank - log(32/8) = lgSizeBank - 2, SRAM depth in FGRA_data_width
  val lgDepth = lgSizeBank - lgDataByte  // SRAM depth in FGRA_Bus_width(128, 64)
  val lgDepthLast = lgSizeLastBlock - lgDataByte// Config SRAM depth in FGRA_Bus_Width(128, 64)
  val maxPartitionSize = 1 << lgMaxPartitionSize
//  val N = { //@yuan: multiple memory access only considered when bus width > FGRA width
//    if(dataWidth  / dataWidthSram > 0)
//      dataWidth  / dataWidthSram
//    else
//      1
//  }
//  val lgN = log2Ceil(N)
//  val AddrIndexWidth = lgN + lgMaxPartitionSize
  val io = IO(new Bundle {
    // stream interfaces
    val read = new SpadStreamReadIF(spadAddrWidth, lgMaxPartitionSize, lgMaxDataLen, dataWidth, hasMask, idWidth) //@yuan: spadAddrNum is used by partitioned memory reading operation
    val write = new SpadStreamWriteIF(spadAddrWidth, spadAddrNum, lgMaxPartitionSize, lgMaxDataLen, dataWidth, hasMask, idWidth)
    // sram bank interfaces
    val srams = Vec(nBanks, new SRAMIO(dataWidthSram, lgDepthSram, hasMask))
    // last sram block interface
    val sram_last = new SRAMIO(dataWidth, lgDepthLast, hasMask)
  })

  val spad_banks_io = Seq.fill(nBanks){Module(new TrueDualPortSRAMAsym(dataWidth, lgDepth, dataWidthSram, hasMask, lgMaxPartitionSize)).io}
  val spad_last_bank_io = Module(new TrueDualPortSRAM(dataWidth, lgDepthLast, hasMask)).io
  val spad_if_io = Module(new SpadReadInterface(lgDepth, lgMaxPartitionSize, dataWidth, dataWidthSram, nBanks)).io //@yuan: programable memory interface


  io.srams.zipWithIndex.foreach{ case (sram, i) =>
    sram <> spad_banks_io(i).b
  }
  io.sram_last <> spad_last_bank_io.b

//  (0 until nBanks).foreach{ i =>
//    spad_if_io.sramDatas(i) := spad_banks_io(i).a.dout
//  } //@yuan: connect the sratchpads with the memory interface


  val s_write_idle :: s_write_data :: Nil = Enum(2)
  val writeState = RegInit(s_write_idle)
//  val writeAddr = RegInit(0.U(spadAddrWidth.W))
//  val writeBankIdx = writeAddr(spadAddrWidth-1, lgSizeBank)
  val wirteBankOffsets = RegInit(VecInit(Seq.fill(spadAddrNum){0.U(lgMaxPartitionSize.W)}))//@yuan: each bank offset in one data transmission
  val writePartitionSize= RegInit(0.U(2.W)) //@yuan: log2(N)
  val writePartitionBlockSize = RegInit(0.U(4.W))//@yuan: log2(B)
//  val writePartitionMode = RegInit(0.U(1.W))
  val writeAddrs = RegInit(VecInit(Seq.fill(spadAddrNum){0.U((spadAddrWidth + lgMaxPartitionSize).W)}))
  val writeBankIdxs = writeAddrs.map{ addr => addr(spadAddrWidth-1 + lgMaxPartitionSize, lgSizeBank + lgMaxPartitionSize)}
  val writeMaxAddr = RegInit(0.U((spadAddrWidth + lgMaxPartitionSize).W))
  val writeId = RegInit(0.U(idWidth.W))
  val writeAddrNum = RegInit(0.U(log2Ceil(spadAddrNum).W))//@yuan: the number of bank in this data transmission
  io.write.req.ready := (writeState === s_write_idle)
  switch(writeState){
    is(s_write_idle){
      when(io.write.req.fire){
        writeState := s_write_data
        wirteBankOffsets := io.write.req.bits.offsetId
        writePartitionSize := io.write.req.bits.lgPartitionSize //@yuan: the number of partitioned memory
        writePartitionBlockSize := io.write.req.bits.lgPartitionBlockSize
        (0 until spadAddrNum).foreach{ i =>
          writeAddrs(i) := io.write.req.bits.addrs(i)(spadAddrWidth-1, lgSizeBank) << (lgSizeBank + lgMaxPartitionSize).asUInt + io.write.req.bits.addrs(i)(lgSizeBank - 1, 0)
        }
//        writePartitionMode := io.write.req.bits.mode
        // writeAddrs := io.write.req.bits.addrs((spadAddrWidth-1 + lgMaxPartitionSize, lgSizeBank + lgMaxPartitionSize)) << (lgSizeBank + lgMaxPartitionSize) + io.write.req.bits.addrs(lgSizeBank + lgMaxPartitionSize - 1, 0)
        writeAddrNum := io.write.req.bits.num
        // writeMaxAddr := io.write.req.bits.addrs(0)(spadAddrWidth-1, lgSizeBank) << (lgSizeBank + lgMaxPartitionSize).asUInt + io.write.req.bits.addrs(0)(lgSizeBank - 1, 0) + Mux(io.write.req.bits.len > dataByte.U, io.write.req.bits.len - dataByte.U, 0.U) // these address/len are both in byte
        writeMaxAddr := (io.write.req.bits.addrs(0)(spadAddrWidth-1, lgSizeBank) << (lgSizeBank + lgMaxPartitionSize).asUInt + io.write.req.bits.addrs(0)(lgSizeBank - 1, 0)) + Mux(io.write.req.bits.len > dataByte.U, io.write.req.bits.len - dataByte.U, 0.U) // these address/len are both in byte
        writeId := io.write.req.bits.id
      }
    }
    is(s_write_data){
      when(io.write.exp.req){
        writeState := s_write_idle
      }.elsewhen(io.write.stream.fire){
        (0 until spadAddrNum).foreach{ i =>
          writeAddrs(i) := writeAddrs(i) + dataByte.U
        }
        writeState := Mux(writeAddrs(0) >= writeMaxAddr, s_write_idle, s_write_data)
      }
    }
  }
  val write_exp_ack = RegInit(false.B)
  when(writeState === s_write_data && io.write.exp.req){
    write_exp_ack := true.B
  }.elsewhen(writeState === s_write_idle){
    write_exp_ack := false.B
  }
  io.write.exp.ack := write_exp_ack

  val s_read_idle :: s_read_data :: s_read_data_last :: Nil = Enum(3)
  val readState = RegInit(s_read_idle)
  val readBankOffsets = RegInit(VecInit(Seq.fill(maxPartitionSize){0.U(lgMaxPartitionSize.W)}))//@yuan: each bank offset in one data transmission
  val readPartitionSize = RegInit(0.U(2.W))//@yuan: log2(N)
  val readPartitionBlockSize = RegInit(0.U(4.W))//@yuan: log2(B)
//  val readPartitionMode = RegInit(0.U(1.W))
  val readAddrs = RegInit(VecInit(Seq.fill(maxPartitionSize){0.U((spadAddrWidth + lgMaxPartitionSize).W)}))
  val readBankIdxs = readAddrs.map{ addr => addr(spadAddrWidth-1 + lgMaxPartitionSize, lgSizeBank + lgMaxPartitionSize)}
  val readMaxAddr = RegInit(0.U((spadAddrWidth + lgMaxPartitionSize).W))
  val readId = RegInit(0.U(idWidth.W))
  val readBankIdx = readAddrs(0)(spadAddrWidth + lgMaxPartitionSize -1, lgSizeBank + lgMaxPartitionSize)//@yuan: just used by access last bank
  val rdataValid = RegInit(false.B)
  val readBankIdxSyn = RegInit(0.U((spadAddrWidth-lgSizeBank).W))
  // val readAddrNum = RegInit(0.U(lgMaxPartitionSize.W))
  io.read.req.ready := (readState === s_read_idle)
  switch(readState){
    is(s_read_idle){
      rdataValid := false.B
      when(io.read.req.fire){
        readState := s_read_data
        readBankOffsets := io.read.req.bits.offsetId
        readPartitionSize := io.read.req.bits.lgPartitionSize
        readPartitionBlockSize := io.read.req.bits.lgPartitionBlockSize
//        readPartitionMode := io.read.req.bits.mode
        (0 until maxPartitionSize).foreach{ i =>
          readAddrs(i) := io.read.req.bits.addrs(i)(spadAddrWidth-1, lgSizeBank) << (lgSizeBank + lgMaxPartitionSize).asUInt + io.read.req.bits.addrs(i)(lgSizeBank - 1, 0)
        }
        readMaxAddr := (io.read.req.bits.addrs(0)(spadAddrWidth-1, lgSizeBank) << (lgSizeBank + lgMaxPartitionSize).asUInt + io.read.req.bits.addrs(0)(lgSizeBank - 1, 0)) + Mux(io.read.req.bits.len > dataByte.U, io.read.req.bits.len - dataByte.U, 0.U)
        readId := io.read.req.bits.id
      }
    }
    is(s_read_data){
      when(io.read.exp.req){
        readState := s_read_idle
      }.elsewhen(io.read.stream.ready){
        rdataValid := true.B
//        readBankIdxSyn := readBankIdx
        readBankIdxSyn := Mux(readBankIdx < nBanks.U, 0.U, readBankIdx - nBanks.U)
        (0 until maxPartitionSize).foreach{ i =>
          readAddrs(i) := readAddrs(i) + dataByte.U
        }
        readState := Mux(readAddrs(0) >= readMaxAddr, s_read_data_last, s_read_data)
      }
    }
    is(s_read_data_last){ // last data
      when(io.read.stream.ready){
        readState := s_read_idle
        rdataValid := false.B
      }
    }
  }
  val read_exp_ack = RegInit(false.B)
  when(readState === s_read_data && io.read.exp.req){
    read_exp_ack := true.B
  }.elsewhen(readState === s_read_idle){
    read_exp_ack := false.B
  }
  io.read.exp.ack := read_exp_ack

  // stream read and write cannot access the same bank simultaneously
  // the requester should make sure no access conflicts
  // access to spad banks
  spad_banks_io.zipWithIndex.foreach{ case (sp, i) =>
//    val en = (0 until spadAddrNum).map{ j =>
//      (writeAddrNum >= j.U && writeBankIdxs(j) === i.U)
//    }.reduce(_|_)
    val write_sel = (0 until spadAddrNum).map{ j => (writeAddrNum >= j.U && writeBankIdxs(j) === i.U) } // check the selected bank index
    val write_en = write_sel.reduce(_|_)
    val read_sel = (0 until maxPartitionSize).map{ j => ((1.U << readPartitionSize).asUInt >= j.U && readBankIdxs(j) === i.U) }
    val read_en = read_sel.reduce(_|_)
    when(writeState === s_write_data && write_en){
//      val addrMap = (0 until spadAddrNum).map{ j =>
//        (writeAddrNum >= j.U && writeBankIdxs(j) === i.U) -> writeAddrs(j)(lgSizeBank-1, lgDataByte)
//      }
      val addrMap = write_sel.zipWithIndex.map{ case (v, j) => v -> writeAddrs(j)(lgSizeBank + lgMaxPartitionSize - 1, lgDataByte) }
      val offsetMap = write_sel.zipWithIndex.map{case (v, j) => v -> wirteBankOffsets(j) }
      sp.a.en := io.write.stream.fire
      sp.a.we := { if(hasMask) io.write.stream.bits.mask else 1.U }
      sp.a.addr := PriorityMux(addrMap)
      sp.a.din := io.write.stream.bits.data
      sp.ctrl.bankId := PriorityMux(offsetMap)
      sp.ctrl.lgpartitionSize := writePartitionSize
      sp.ctrl.lgpartitionBlockSize := writePartitionBlockSize
//      sp.ctrl.mode := writePartitionMode.asBool
    // }.elsewhen(readState === s_read_data && readBankIdx === i.U){
    }.elsewhen(readState === s_read_data && read_en){
      val addrMap = read_sel.zipWithIndex.map{ case (v, j) => v -> readAddrs(j)(lgSizeBank + lgMaxPartitionSize -1, lgDataByte) }
      val offsetMap = read_sel.zipWithIndex.map{case (v, j) => v -> readBankOffsets(j) }
      sp.a.en := io.read.stream.ready
      sp.a.we := 0.U
      sp.a.addr := PriorityMux(addrMap)
      sp.a.din := DontCare
      sp.ctrl.bankId := PriorityMux(offsetMap)
      sp.ctrl.lgpartitionSize := readPartitionSize
      sp.ctrl.lgpartitionBlockSize := readPartitionBlockSize
//      sp.ctrl.mode := readPartitionMode.asBool
    }.otherwise{
      sp.a.en := false.B
      sp.a.we := 0.U
      sp.a.addr := DontCare
      sp.a.din := DontCare
      sp.ctrl.bankId := DontCare
      sp.ctrl.lgpartitionSize := DontCare
      sp.ctrl.lgpartitionBlockSize := DontCare
//      sp.ctrl.mode := DontCare
    }
  }
  // access to last spad block, can only have one address
  when(writeState === s_write_data && writeBankIdxs(0) >= nBanks.U){
    spad_last_bank_io.a.en := io.write.stream.fire
    spad_last_bank_io.a.we := { if(hasMask) io.write.stream.bits.mask else 1.U }
    spad_last_bank_io.a.addr := writeAddrs(0)(lgSizeBank-1, lgDataByte)
    spad_last_bank_io.a.din := io.write.stream.bits.data
  }.elsewhen(readState === s_read_data && readBankIdx >= nBanks.U){
    spad_last_bank_io.a.en := io.read.stream.ready
    spad_last_bank_io.a.we := 0.U
    spad_last_bank_io.a.addr := readAddrs(0)(lgSizeBank-1, lgDataByte)
    spad_last_bank_io.a.din := DontCare
  }.otherwise{
    spad_last_bank_io.a.en := false.B
    spad_last_bank_io.a.we := 0.U
    spad_last_bank_io.a.addr := DontCare
    spad_last_bank_io.a.din := DontCare
  }

  //@yuan: config the memory interface
  val spad_if_enable = RegInit(0.U(1.U))
  when(readState === s_read_data && readBankIdx < nBanks.U){
    spad_if_enable := true.B
//    spad_if_io.mode := readPartitionMode
  }.otherwise{
    spad_if_enable := false.B
//    spad_if_io.mode := DontCare
  }
  spad_if_io.enable := spad_if_enable

  val readBankIdxsSyn = RegInit(VecInit(Seq.fill(maxPartitionSize){0.U(spadAddrWidth.W)}))
  val readBankOffsetsSyn = RegInit(VecInit(Seq.fill(maxPartitionSize){0.U(lgMaxPartitionSize.W)}))
  (0 until maxPartitionSize).foreach{ i =>
    readBankIdxsSyn(i) := RegNext(readBankIdxs(i))
    readBankOffsetsSyn(i) := RegNext(readBankOffsets(i))
    val bank_sel = (0 until nBanks).map { j =>
      (0 until maxPartitionSize).map { k => ((1.U << readPartitionSize).asUInt > k.U && readBankIdxsSyn(k) === j.U && readBankOffsetsSyn(k) === i.U)}.reduce(_|_)
    }//@yuan: the selected bank will be "1", in this map
    val doutMap = bank_sel.zipWithIndex.map{case (v, j) => v -> spad_banks_io(j).a.dout}
    spad_if_io.sramDatas(i) := PriorityMux(doutMap)
  }
  spad_if_io.addrs := RegNext(readAddrs(0)(lgSizeBank + lgMaxPartitionSize -1, lgDataByte)) //@yuan: the compiler will ensure that all the start address of multiple memory access is 0
  spad_if_io.lgPartitionSize := readPartitionSize //@yuan: the number of partitioned memory banks
  spad_if_io.lgPartitionBlockSize := readPartitionBlockSize

  io.write.stream.ready := (writeState === s_write_data)

  val lastBanks = {
    if(lgSizeLastBlock <= lgSizeBank) 1
    else 1 << (lgSizeLastBlock - lgSizeBank)
  }
//  val doutTable1 = spad_banks_io.zipWithIndex.map{ case (sp, i) => (i.U -> sp.a.dout) }
  val doutTable1 = (0 to 0).map{ i => i.U -> spad_if_io.dout}
  val doutTable2 = (1 to lastBanks).map{ i => (i.U -> spad_last_bank_io.a.dout) }
  val doutTable = doutTable1 ++ doutTable2
  val Stream_data = MuxLookup(readBankIdxSyn, 0.U, doutTable)
  val Stream_data_reg = RegInit(0.U(dataWidth.W)) //@yuan: keep the spad output data for longer time
  val s_output_switch :: s_output_keep :: Nil = Enum(2)
  val SpadState = RegInit(s_output_switch)
  switch(SpadState) {
    is(s_output_switch) {
      Stream_data_reg := Stream_data
      when(!io.read.stream.ready) {
        SpadState := s_output_keep
      }
    }
    is(s_output_keep) {
      Stream_data_reg := Stream_data_reg
      when(io.read.stream.ready) {
        SpadState := s_output_switch
      }
    }
  }
  io.read.stream.valid := rdataValid
  io.read.stream.bits.data := Mux(SpadState === s_output_switch , Stream_data, Stream_data_reg)
  io.read.stream.bits.id := readId
  if(hasMask) io.read.stream.bits.mask := ((1 << dataByte)-1).U
}