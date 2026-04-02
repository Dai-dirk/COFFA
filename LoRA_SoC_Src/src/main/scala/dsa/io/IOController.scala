package fgramemfp.dsa

import chisel3._
import chisel3.util._

import scala.collection.mutable
import fgramemfp.common.MacroVar._
import fgramemfp.op._

import scala.collection.mutable.ArrayBuffer


import scala.collection.mutable.ArrayBuffer


/** I/O Controller
 * @param dataWidth      data width in bits
 * @param addrWidth      address width (+1 every data width), {bank id, address}
 * @param hasMask        if has write data byte mask
 * @param mode           IFIFO, OFIFO, IOFIFO (with din/dout), SRAM mode (with addr, din, dout)
 * @param hasIOFG        if has fine-grained IO pair
 * @param lgMaxStride    log2(max address stride, n represent n*dataWidth bits)
 * @param lgMaxLat       log2(max in/out latency)
 * @param lgMaxCycles    log2(max in/out cycles. i.e. loop cycles)
 * @param lgMaxII        log2(max in/out Initialization Interval)
 * @param agNestLevels   nested levels of the address generation
 * @param addRegSram     if add reg, write/read latency is 1/2; otherwise, 0/1; for both FIFO and SRAM modes
 * @param lgMaxPartition log2( the number of coalesce banks)
 */
class IOController(dataWidth: Int, addrWidth: Int, hasMask: Boolean, mode: Int, hasIOFG: Boolean,
                   lgMaxStride: Int, lgMaxLat: Int, lgMaxCycles: Int, lgMaxII: Int, agNestLevels: Int, addRegSram: Boolean, lgMaxPartition: Int) extends Module {
  // coarse-grained input/output number
  val hasTaskExit = mode == TASK_COND_EXIT_MODE
  val numInCG = {
    if(mode == SRAM_MODE || hasTaskExit) 2 // address and data
    else 1  // data
  }
  val numOutCG = 1 // data
  // fine-grained input/output number
  val hasFGIn = hasIOFG
  val hasFGOut = hasIOFG  // ((mode == IFIFO_MODE) || (mode == IOFIFO_MODE))
  val numInFG = {
    if (hasFGIn && hasTaskExit) 2
    else if (hasFGIn || hasTaskExit) 1
    else 0
  }
  val numOutFG = {if(hasFGOut) 1 else 0}
//  println("IOController addrWidth: " + addrWidth)
  val lgpartitionBankSize = log2Ceil(addrWidth - lgMaxPartition)
  val partitionBankSize = addrWidth - lgMaxPartition
  val cfgWidth = addrWidth + (lgMaxStride + lgMaxCycles)*agNestLevels + lgMaxII + lgMaxLat + 2*(lgMaxPartition) + lgpartitionBankSize + 5
  // ----------- base_addr ---- stride ---- cycles ----- II ---- latency ---- partitioned_memory_start-bank ---- number_of_partitioned_memory(N) ---- partitionSize(B) ---- isLoad - useFGIn - useFGOut - useAddr - useTaskToken-
  val io = IO(new Bundle{
    val sram = Flipped(new SRAMIO(dataWidth, addrWidth, hasMask))
    val start = Input(Bool()) // pulse signal, should be valid before latency 0, namely -1
    val done = Output(Bool()) // transfer done, keep true until next start
    val config = Input(UInt(cfgWidth.W))
    val in_cg = Input(Vec(numInCG, UInt(dataWidth.W)))
    val out_cg = Output(Vec(numOutCG, UInt(dataWidth.W)))
    val in_fg = Input(Vec(numInFG, UInt(1.W)))
    val out_fg = Output(Vec(numOutFG, UInt(1.W)))
  })

  val dataByte = dataWidth/8
  val lgDataByte = log2Ceil(dataByte)
  assert(dataWidth%8 == 0 && isPow2(dataWidth/8))
  val wmask = {if(hasMask) (1 << dataByte) - 1 else 1}
  // IO Controller config
  // [name, (id, high-bit, low-bit)]
  val ioc_cfg_idx: mutable.Map[String, (Int, Int, Int)] = mutable.Map()
  // io.config should keep constant during io.en is true
  var offset = 0
  var id = 0
  val baseAddr = io.config(addrWidth+offset-1, offset)
  ioc_cfg_idx += "BaseAddr" -> (id, addrWidth+offset-1, offset)
  offset += addrWidth
  id += 1
  val stride = new ArrayBuffer[SInt]()
  val cycles = new ArrayBuffer[UInt]()
  for (i <- 0 until agNestLevels) {
    stride += io.config(lgMaxStride + offset - 1, offset).asSInt // address stride
    ioc_cfg_idx += ("Stride" + i.toString) -> (id, lgMaxStride + offset - 1, offset)
    offset += lgMaxStride
    cycles += io.config(lgMaxCycles + offset - 1, offset)
    ioc_cfg_idx += ("Cycles" + i.toString) -> (id + 1, lgMaxCycles + offset - 1, offset)
    offset += lgMaxCycles
    id += 2
  }
//  val stride = io.config(lgMaxStride+offset-1, offset) // address stride
//  ioc_cfg_idx += "Stride" -> (1, lgMaxStride+offset-1, offset)
//  offset += lgMaxStride
//  val cycles = io.config(lgMaxCycles+offset-1, offset)
//  ioc_cfg_idx += "Cycles" -> (2, lgMaxCycles+offset-1, offset)
//  offset += lgMaxCycles
  val II = io.config(lgMaxII+offset-1, offset)
  ioc_cfg_idx += "II" -> (id, lgMaxII+offset-1, offset)
  offset += lgMaxII
  id += 1
  val latency = io.config(lgMaxLat+offset-1, offset)  // the latency of starting input or output
  ioc_cfg_idx += "Latency" -> (id, lgMaxLat+offset-1, offset)
  offset += lgMaxLat
  id += 1
  val startBankIdx = io.config(lgMaxPartition+offset-1, offset) // the start bank of serial partitioned memory banks
  ioc_cfg_idx += "startBankIdx" -> (id, lgMaxPartition+offset-1, offset)
  offset += lgMaxPartition
  id += 1
  val logN = io.config(lgMaxPartition + offset - 1, offset) // the start bank of serial partitioned memory banks
  ioc_cfg_idx += "logN" -> (id, lgMaxPartition + offset - 1, offset)
  offset += lgMaxPartition
  id += 1
//  val parMode = io.config(offset+1, offset) // the start bank of serial partitioned memory banks
//  ioc_cfg_idx += "parMode" -> (id, 1 + offset, offset)
//  offset += 2
//  id += 1
  val logB = io.config(lgpartitionBankSize + offset - 1, offset) // the start bank of serial partitioned memory banks
  ioc_cfg_idx += "logB" -> (id, lgpartitionBankSize + offset - 1, offset)
  offset += lgpartitionBankSize
  id += 1
  val isLoad = io.config(offset).asBool // whether load data from SRAM or store data to SRAM
  val useFGIn = io.config(offset+1).asBool // use the fine-grained input
  val useFGOut = io.config(offset+2).asBool // use the fine-grained output
  val useAddr = io.config(offset+3).asBool // use the address port (in_cg(1))
  val useTask = io.config(offset + 4).asBool // use the task end token
  ioc_cfg_idx += "IsLoad" -> (id, offset, offset)
  ioc_cfg_idx += "UseFGIn" -> (id+1, offset+1, offset+1)
  ioc_cfg_idx += "UseFGOut" -> (id+2, offset+2, offset+2)
  ioc_cfg_idx += "UseAddr" -> (id+3, offset+3, offset+3)
  ioc_cfg_idx += "useTask" -> (id + 4, offset + 4, offset + 4)

  val s_idle :: s_pre_lat :: s_data :: Nil = Enum(3)
  val state = RegInit(s_idle)
  val latCnt = RegInit(0.U(lgMaxLat.W))
  val cycleCnt = Seq.fill(agNestLevels) {RegInit(0.U(lgMaxCycles.W))}
  val iiCnt = RegInit(0.U(lgMaxII.W))
  val done = RegInit(false.B)
  val iiEnd = (iiCnt + 1.U >= II)
  val cycleCntEnd = (0 until agNestLevels).map { i =>
    (0 to i).map { j => cycleCnt(j) + 1.U >= cycles(j) }.reduce(_ & _)
  }
  val cntEnd = cycleCntEnd(agNestLevels - 1)
  val taskEnd = WireInit(0.U(1.W))
  if(hasFGIn && hasTaskExit){
    taskEnd := io.in_fg(1)
  }else if(!hasFGIn && hasTaskExit){
    taskEnd := io.in_fg(0)
  }else{
    taskEnd := DontCare
  }
  val realTaskEnd = taskEnd & useTask
  io.done := done
  switch(state){
    is(s_idle){
      latCnt := 0.U
      when(io.start && latency === 0.U){
        state := s_data
        done := false.B // done keep true until the next start
      }.elsewhen(io.start && latency =/= 0.U){
        state := s_pre_lat
        done := false.B
      }
    }
    is(s_pre_lat){ // the latency before starting input or output
      when(latCnt + 1.U === latency){
        state := s_data
      }
      latCnt := latCnt + 1.U
    }
    is(s_data){
      when((cntEnd && iiEnd) || realTaskEnd === 1.U) {
        state := s_idle
        done := true.B
      }
//      when(iiCnt === 0.U && realTaskEnd === 1.U){
//        state := s_task
//      }.elsewhen(cntEnd && iiEnd){
//        state := s_idle
//        done := true.B
//      }
    }
//    is(s_task){
//      when(iiEnd){ // @yuan: just one more memory access
//        state := s_idle
//        done := true.B
//      }
//    }
  }

//  val launch = (state === s_data) || (state === s_task)
  val launch = (state === s_data)
  when(state === s_idle){
    iiCnt := 0.U
  }.elsewhen(state === s_data){
    iiCnt := Mux(iiEnd, 0.U, iiCnt + 1.U)
  }

  for (i <- 0 until agNestLevels) {
    when(state === s_idle) {
      cycleCnt(i) := 0.U
    }.elsewhen(launch) {
      when(iiEnd && !cycleCntEnd(i)) {
        cycleCnt(i) := cycleCnt(i) + {
          if (i == 0) 1.U // add 1 every II cycle
          else cycleCntEnd(i - 1)
        }
      }.elsewhen(iiEnd) {
        cycleCnt(i) := 0.U
      }
    }
  }

  // write real valid: the last level with stride = 0 and cycleCntEnd
  val wRealValid = (0 until agNestLevels).map { i =>
    (stride(i) === 0.S && cycleCnt(i) + 1.U >= cycles(i)) || stride(i) =/= 0.S
  }.reduce(_ & _)

  // read real valid: the last level with stride = 0 and cycleCnt == 0
  //@yuan: it seems like done't work
//  val rRealValid = (0 until agNestLevels).map { i =>
//    (stride(i) === 0.S && cycleCnt(i) === 0.U) || stride(i) =/= 0.S
//  }.reduce(_ & _)

  // in -> sram
  val wValid = !isLoad && launch && (iiCnt === 0.U)
  val wData = io.in_cg(0) // { if(mode == FIFO_MODE) io.in(0) else io.in(1) }
  // sram -> out
  val raValid = isLoad && launch && (iiCnt === 0.U)  // read address valid
  val addr = Wire(UInt(addrWidth.W))
  val genAddrReg = RegInit(0.S((addrWidth + 1).W))

  val strideMap = (agNestLevels - 1 to 0 by -1).map { i =>
    if (i == 0) {
      true.B -> stride(i)
    } else {
      cycleCntEnd(i - 1) -> stride(i)
      //      (0 until i).map(cycleCntEnd(_)).reduce(_&_) -> stride(i)
    }
  }

//  val realEn = (wValid && (wRealValid || useAddr)) || (raValid && (rRealValid || useAddr))
  val realEn = (wValid && (wRealValid || useAddr)) || (raValid)

  // data input/output only during the s_data state, but may not read/write SRAM (called launch) all the time
  // case 1: use in_fg to indicate launching
  // case 2: directly launch
  // SRAM Ports
  val wen = Wire(Bool())
  val wdata = Wire(UInt(dataWidth.W))
  val ren = Wire(Bool())
//  val addr = Wire(UInt(addrWidth.W))
//  val addrRegTmp = RegInit(0.U(addrWidth.W))
  //@yuan: this signal is used ti generate the first enable signal when there is a backedge between output and input nodes
  val first_en = RegInit(0.U(1.W))
  when(io.start && latency === 0.U && first_en === 0.U){
    first_en := 1.U
  }.elsewhen(realEn && first_en === 1.U){
    first_en := 0.U
  }.otherwise{
    first_en := first_en
  }
  if(mode == FIFO_MODE){ // FIFO mode: send/receive
    if(hasFGIn){
      wen := Mux(useFGIn, realEn & (!isLoad) & io.in_fg(0), realEn & (!isLoad))
      ren := Mux(useFGIn, realEn & isLoad & (io.in_fg(0) | first_en), realEn & isLoad)
    }else{
      wen := realEn & (!isLoad) // & (iiCnt === 0.U)
      ren := realEn & isLoad
    }
    wdata := wData
  }else{ // Load/Store//@yuan: task_end only emerge with load/store and fine-grained inputs
    if(hasFGIn){
      wen := Mux(useFGIn, realEn & (!isLoad) & io.in_fg(0) & (!realTaskEnd), realEn & (!isLoad)& (!realTaskEnd))
      ren := Mux(useFGIn, realEn & isLoad & (io.in_fg(0) | first_en) & (!realTaskEnd), realEn & isLoad& (!realTaskEnd))
    }else{
      wen := realEn & (!isLoad) & (!realTaskEnd) // & (iiCnt === 0.U)
      ren := realEn & isLoad & (!realTaskEnd)
    }
    wdata := wData // 0: address, 1: data
  }

  when(!launch){
    genAddrReg := Cat(0.S(1.W), baseAddr).asSInt
  }.elsewhen(iiEnd) {
    genAddrReg := genAddrReg + PriorityMux(strideMap)
  }
  if(mode == SRAM_MODE || mode == TASK_COND_EXIT_MODE){
    val BankIdx = Wire(UInt((lgMaxPartition + 1).W))
    val BankOffset = Wire(UInt(partitionBankSize.W))
    BankIdx := DontCare
    BankOffset := DontCare
    val addr_temp = Wire(UInt(addrWidth.W))
    when(useAddr && isLoad){
      addr_temp := baseAddr + io.in_cg(0)(dataWidth-1, lgDataByte)
    }.elsewhen(useAddr & !isLoad){
      addr_temp := baseAddr + io.in_cg(1)(dataWidth - 1, lgDataByte)
    }.otherwise{
      addr_temp := genAddrReg.asUInt
    }
    BankIdx := (addr_temp.asUInt >> logB).asUInt & ((1.U << logN).asUInt - 1.U) // [x/B] % N
    BankOffset := ((addr_temp.asUInt >> (logN + logB).asUInt) << logB).asUInt + (addr_temp.asUInt & ((1.U << logB).asUInt - 1.U)) // (x / N *B) * B + x%B
    when(logN === 0.U){
      addr := addr_temp
    }.otherwise{
      addr := ((startBankIdx + BankIdx) << partitionBankSize).asUInt + BankOffset
    }
    
  
    // .otherwise { //@yuan: memory partition only be considered in stencil-based memory access
    //   BankIdx := (genAddrReg.asUInt >> logB).asUInt & ((1.U << logN).asUInt - 1.U) // [x/B] % N
    //   BankOffset := ((genAddrReg.asUInt >> (logN + logB).asUInt) << logB).asUInt + (genAddrReg.asUInt & ((1.U << logB).asUInt - 1.U)) // (x / N *B) * B + x%B
    //   addr := ((startBankIdx + BankIdx) << partitionBankSize).asUInt + BankOffset
    //   //      when(parMode === 1.U){ // parallel mode: 0: normal, without partition; 1: partition by column ({0,1,2,3}; {4, 5, 6, 7}); 2: partition by row ({0, 2, 4 6}; {1, 3, 5, 7})
    //   //        BankIdx := (genAddrReg.asUInt >> logB).asUInt & ((1.U << logN).asUInt - 1.U) // [x/B] % N
    //   //        BankOffset := genAddrReg.asUInt & ((1.U << logB).asUInt - 1.U) // x % B
    //   //        addr := ((startBankIdx + BankIdx) << partitionBankSize).asUInt + BankOffset
    //   //      }.elsewhen(parMode === 2.U){
    //   //        BankIdx := genAddrReg.asUInt & ((1.U << logN).asUInt - 1.U) // x % N
    //   //        BankOffset := genAddrReg.asUInt >> logN // x / N
    //   //        addr := ((startBankIdx + BankIdx) << partitionBankSize).asUInt + BankOffset
    //   //      }.otherwise{
    //   //        addr := genAddrReg.asUInt
    //   //      }
    // }
  }else{
    addr := genAddrReg.asUInt
  }

  if(addRegSram){ // add reg, write/read latency is 1/3
    val addrReg = RegInit(0.U(addrWidth.W))
    when(wen || ren) {
      addrReg := addr
    }
    val wDataReg = RegInit(0.U(dataWidth.W))
    when(wen){
      wDataReg := wdata
    }
    when(isLoad){//@yuan: for load operation, the latency is 2
      io.sram.en := wen | ren
      io.sram.we := Mux(wen, wmask.U, 0.U)
      io.sram.addr := addr
      io.sram.din := wdata
      io.out_cg(0) := RegNext(io.sram.dout)
    }.otherwise{//@yuan: otherwise, for the store operation, the latency is 1
      io.sram.en := RegNext(wen | ren)
      io.sram.we := RegNext(Mux(wen, wmask.U, 0.U))
      io.sram.addr := addrReg
      io.sram.din := wDataReg
      io.out_cg(0) := DontCare
    }
    // io.sram.en := RegNext(wen | ren)
    // io.sram.we := RegNext(Mux(wen, wmask.U, 0.U))
    // io.sram.addr := addrReg
    // io.sram.din := wDataReg
    // io.out_cg(0) := RegNext(io.sram.dout)
  }else{ // not add reg, write/read latency is 0/2
    io.sram.en := wen | ren
    io.sram.we := Mux(wen, wmask.U, 0.U)
    io.sram.addr := addr
    io.sram.din := wdata
    io.out_cg(0) := RegNext(io.sram.dout)
  }

  // io.out_cg(0) := io.sram.dout

  if(hasFGOut){
    io.out_fg(0) := RegNext(io.sram.en === 0.U & (io.sram.we === 0.U) & useFGOut)
  }
}