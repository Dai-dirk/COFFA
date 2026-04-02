package fgramemfp.dsa

import chisel3._
import chisel3.util._
import fgramemfp.common.CompileMacroVar._


// SRAM I/O ports
class SRAMIO(val dataWidth: Int, val addrWidth: Int, val hasMask: Boolean) extends Bundle {
  val en   = Input(Bool())
  val we   = Input(UInt({if(hasMask) (dataWidth/8) else 1}.W))
  val addr = Input(UInt(addrWidth.W))
  val din  = Input(UInt(dataWidth.W))
  val dout = Output(UInt(dataWidth.W))
}

//@yuan: SRAM COntrol I/O ports, which are used to control the writting of partitioned memory
class SRAMCtrlIO(val lgMaxPartitionSize: Int) extends Bundle {
//  val mode   = Input(Bool())
  val lgpartitionSize = Input(UInt(2.W)) //@yuan: for now the maxixmum Nb should less than 8 (log2(8) = 3)
  val lgpartitionBlockSize = Input(UInt(4.W)) //@yuan: log2(B)
  val bankId = Input(UInt(lgMaxPartitionSize.W)) //@yuan: partitioned memory id, that is, offset against the base bank
}

/** True dual port SRAM
  * @param width    data width
  * @param lgDepth  log2(SRAM depth)
  * @param hasMask  if has write data byte mask
  */
class TrueDualPortSRAM(width: Int, lgDepth: Int, hasMask: Boolean) extends Module {
  override def desiredName = "TrueDualPortSRAM_" + width.toString + "_" + lgDepth.toString
  val io = IO(new Bundle{
    val a = new SRAMIO(width, lgDepth, hasMask)
    val b = new SRAMIO(width, lgDepth, hasMask)
  })
  if(USE_SRAM_BLACKBOX){
//    class DPSRAMIP extends BlackBox(Map(
//      "width" -> width,
//      "lgDepth" -> lgDepth,
//      "hasMask" -> hasMask.toString
//    )){
    class DPSRAMIP extends BlackBox{
      override def desiredName = "dp_sram_" + width.toString + "_" + lgDepth.toString
      val io = IO(new Bundle{
        val clk = Input(Clock())
//        val rst = Input(Bool())
        val a = new SRAMIO(width, lgDepth, hasMask)
        val b = new SRAMIO(width, lgDepth, hasMask)
      })
    }

    val sram = Module(new DPSRAMIP)
    sram.io.clk := clock
//    sram.io.rst := reset
    sram.io.a <> io.a
    sram.io.b <> io.b
  }else{
    val ios = Array(io.a, io.b)
    if(hasMask){
      val maskWidth = width/8
      val mem = SyncReadMem(1<<lgDepth, Vec(maskWidth, UInt(8.W)))
      ios.map{ x =>
        x.dout := DontCare
        when(x.en) {
          val rwPort = mem(x.addr)
          when(x.we =/= 0.U) {
            (0 until maskWidth).map { i =>
              when(x.we(i).asBool) {
                rwPort(i) := x.din(i * 8 + 7, i * 8)
              }
            }
          }.otherwise {
            x.dout := Cat(rwPort.toSeq.reverse)
          }
        }
      }
    } else {
      val mem = SyncReadMem(1<<lgDepth, UInt(width.W))
      ios.map{ x =>
        x.dout := DontCare
        when(x.en) {
          val rwPort = mem(x.addr)
          when(x.we.asBool) {
            rwPort := x.din
          }.otherwise {
            x.dout := rwPort
          }
        }
      }
    }
  }
}


/** True dual port SRAM with asymmetric data width
  * @param aWidth    data width of a interface
  * @param aLgDepth  log2(SRAM depth of a IF)
  * @param bWidth    data width of b interface
  * @param hasMask   if has write data byte mask
  * @param lgMaxPartitionSize       log2(maximum of partitioned size N)  
  */
  //@yuan: for the partitioned memory, we only consider the width of bus is not less than the FGRA's data_width (i.e.a >= b)
class TrueDualPortSRAMAsym(aWidth: Int, aLgDepth: Int, bWidth: Int, hasMask: Boolean, lgMaxPartitionSize: Int) extends Module {
//  assert(isPow2(aWidth) && isPow2(bWidth))
//  assert(aWidth % bWidth == 0 || bWidth % aWidth == 0)
  val bLgDepth = log2Ceil(aWidth * (1 << aLgDepth) / bWidth)
  val io = IO(new Bundle {
    val a = new SRAMIO(aWidth, aLgDepth + lgMaxPartitionSize, hasMask)
    val b = new SRAMIO(bWidth, bLgDepth, hasMask)
    val ctrl = new SRAMCtrlIO(lgMaxPartitionSize)
  })
  val maxWdith = math.max(aWidth, bWidth)
  val minWdith = math.min(aWidth, bWidth)
  val minLgDepth = math.min(aLgDepth, bLgDepth)
  val maxLgDepth = math.max(aLgDepth, bLgDepth)
  val N = maxWdith/minWdith
  val lgN = log2Ceil(N)
  assert(isPow2(N))
  val mems = Seq.fill(N){Module(new TrueDualPortSRAM(minWdith, minLgDepth, hasMask)).io}
  val maskWidth = minWdith/8
  if(aWidth > minWdith){
    val dataInGroup = Wire(Vec(N, UInt(minWdith.W)))
//    val par_addr_width = lgMaxPartitionSize + lgN //@yuan: the width of the partition address, which is used to determine the bank and offset within the bank
    val baseAddr = Wire(UInt((maxLgDepth + lgMaxPartitionSize).W))
    baseAddr := io.a.addr << lgN // @yuan: low lgN bit is used to identify the SRAM, since one bank may contains multiple SRAMs
    val exactBankId = Wire(Vec(N, UInt(lgMaxPartitionSize.W)))
    val offsets = Wire(Vec(N, UInt((maxLgDepth + lgMaxPartitionSize).W)))
    val exactSRAMOffset = Wire(Vec(N, UInt(lgN.W)))
    val exactAddr = Wire(Vec(N, UInt(minLgDepth.W)))
    val realbankId = Wire(UInt(lgMaxPartitionSize.W))
    when(io.ctrl.lgpartitionSize === 0.U){
        realbankId := 0.U
    }.otherwise{
        realbankId := io.ctrl.bankId
    }
    for(i <- 0 until N){
      dataInGroup(i) := io.a.din((i+1)*minWdith-1, i*minWdith) //@yuan: partition the long-bit-width bus data
      exactBankId(i) := ((baseAddr + i.U) >> io.ctrl.lgpartitionBlockSize).asUInt & ((1.U << io.ctrl.lgpartitionSize).asUInt - 1.U)  //@yuan: bankId = (x / B) % N
      offsets(i) := (((baseAddr + i.U) >> (io.ctrl.lgpartitionSize + io.ctrl.lgpartitionBlockSize)) << io.ctrl.lgpartitionBlockSize).asUInt +  ((baseAddr + i.U) & ((1.U << io.ctrl.lgpartitionBlockSize).asUInt - 1.U)) //@yuan: bankId = (x / N *B) * B + x%B
      exactSRAMOffset(i) := offsets(i) & (N - 1).asUInt //@yuan: offset % N
      exactAddr(i) := offsets(i) >> lgN
      //      offsets(i) := (baseAddr + i.U) >> io.ctrl.lgpartitionSize //@yuan: offset = addr / partition_size
    }
    for(i <- 0 until N){
      val bankSelected = exactBankId.map( _ === realbankId).reduce(_ || _) // @yuan: selecting current bank
      val sramSelected = exactSRAMOffset.map( _ === i.U).reduce(_ || _) 
      mems(i).a.en :=  (bankSelected && sramSelected && io.a.en)
      if(hasMask){
        mems(i).a.we := io.a.we((i+1)*maskWidth-1, i*maskWidth)
      }else{
        mems(i).a.we := bankSelected && sramSelected && io.a.we.asBool//@yuan: writting enable is asserted based on the exactly mode
      }
      mems(i).a.addr := exactAddr(i) //@yuan: select the address according to the mode, for the partitioned memory, the compiler ensure the selStart is 0
      // mems(i).a.addr := io.a.addr
      mems(i).a.din := MuxCase(dataInGroup(i), (0 until N).map(j => ((exactBankId(j)=== realbankId) && (exactSRAMOffset(j) === i.U)) -> dataInGroup(j)))//@yuan: the bankId and offset should be match simutaneously
      // mems(i).a.din := io.a.din((i+1)*minWdith-1, i*minWdith)
      mems(i).b.en := io.b.en && (io.b.addr(lgN-1, 0) === i.U)
      mems(i).b.we := io.b.we
      mems(i).b.addr := io.b.addr(bLgDepth-1, lgN)
      mems(i).b.din := io.b.din
    }
    io.a.dout := Cat(mems.map(_.a.dout).reverse.toSeq)
    val benRegs = mems.map{ m => RegNext(m.b.en) }
    io.b.dout := MuxCase(0.U, mems.zipWithIndex.map{ case (m, i) => benRegs(i) -> m.b.dout} )
  }else{
    for(i <- 0 until N){
      mems(i).b.en := io.b.en
      if(hasMask){
        mems(i).b.we := io.b.we((i+1)*maskWidth-1, i*maskWidth)
      }else{
        mems(i).b.we := io.b.we
      }
      mems(i).b.addr := io.b.addr
      mems(i).b.din := io.b.din((i+1)*minWdith-1, i*minWdith)
      mems(i).a.en := io.a.en && (io.a.addr(lgN-1, 0) === i.U)
      mems(i).a.we := io.a.we
      mems(i).a.addr := io.a.addr(bLgDepth-1, lgN)
      mems(i).a.din := io.a.din
    }
    io.b.dout := Cat(mems.map(_.b.dout).reverse.toSeq)
    val aenRegs = mems.map{ m => RegNext(m.a.en) }
    io.a.dout := MuxCase(0.U, mems.zipWithIndex.map{ case (m, i) => aenRegs(i) -> m.a.dout} )
  }

}
// class TrueDualPortSRAMAsym(aWidth: Int, aLgDepth: Int, bWidth: Int, hasMask: Boolean) extends Module {
//   assert(isPow2(aWidth) && isPow2(bWidth))
//   assert(aWidth%bWidth == 0 || bWidth%aWidth == 0)

//   val bLgDepth = log2Ceil(aWidth * (1 << aLgDepth) / bWidth)
//   val io = IO(new Bundle{
//     val a = new SRAMIO(aWidth, aLgDepth, hasMask)
//     val b = new SRAMIO(bWidth, bLgDepth, hasMask)
//   })
//   val maxWdith = math.max(aWidth, bWidth)
//   val minWdith = math.min(aWidth, bWidth)
//   val minLgDepth = math.min(aLgDepth, bLgDepth)
//   val maxLgDepth = math.max(aLgDepth, bLgDepth)
//   val ios = Array(io.a, io.b)

//   if(hasMask){
//     val maskWidth = maxWdith / 8
//     val mem = SyncReadMem(1 << minLgDepth, Vec(maskWidth, UInt(8.W)))
//     ios.zipWithIndex.foreach{ case (x, i) =>
//       val high = { if( i == 0) (aLgDepth - 1) else (bLgDepth - 1) }
//       val low = { if( i == 0) (aLgDepth - minLgDepth) else (bLgDepth - minLgDepth) }
//       val width = { if( i == 0) aWidth else bWidth }
//       val offset = {
//         if(low == 0){ 0.U } else{ Cat(x.addr(low-1, 0), 0.U(log2Ceil(width/8).W)) }
//       }
//       val dataOffset = {
//         if(low == 0){ 0.U }else{ Cat(x.addr(low-1, 0), 0.U(log2Ceil(width).W)) }
//       }
//       val dataOffsetR = RegInit(0.U(dataOffset.getWidth.W))
//       x.dout := DontCare
//       when(x.en) {
//         val rwPort = mem(x.addr(high, low))
//         when(x.we =/= 0.U) {
//           (0 until width/8).map { i =>
//             when(x.we(i).asBool) {
//               rwPort(i.U+offset) := x.din(i * 8 + 7, i * 8)
//             }
//           }
//         }.otherwise {
//           dataOffsetR := dataOffset
//           x.dout := Cat(rwPort.toSeq.reverse) >> dataOffsetR
//         }
//       }
//     }
//   } else {
//     val maskWidth = maxWdith / minWdith
//     val mem = SyncReadMem(1<<minLgDepth, Vec(maskWidth, UInt(minWdith.W)))
//     ios.zipWithIndex.foreach{ case (x, i) =>
//       val width = { if( i == 0) aWidth else bWidth }
//       val high = { if( i == 0) (aLgDepth - 1) else (bLgDepth - 1) }
//       val low = { if( i == 0) (aLgDepth - minLgDepth) else (bLgDepth - minLgDepth) }
//       //      val dout = RegInit(0.U(width.W))
//       val rSel : UInt = {if(low > 0) RegInit(0.U(low.W)) else 0.U}
//       x.dout := DontCare
//       when(x.en) {
//         val rwPort = mem(x.addr(high, low))
//         if(width == maxWdith){
//           when(x.we.asBool) {
//             (0 until maskWidth).map { i =>
//               rwPort(i) := x.din((i + 1) * minWdith - 1, i * minWdith)
//             }
//           }.otherwise{
//             x.dout := Cat(rwPort.toSeq.reverse)
//           }
//         }else{
//           val offset = x.addr(low-1, 0)
//           when(x.we.asBool) {
//             rwPort(offset) := x.din
//           }.otherwise{
//             rSel := offset
//             x.dout := rwPort(rSel)
//           }
//         }
//       }
//       //      x.dout := dout
//     }
//   }
// }


/** Convert SRAM data width
  * @param sDataWidth    data width in slave side
  * @param sAddrWidth    address width in slave side
  * @param sHasMask      if has write data byte mask in slave side
  * @param mDataWidth    data width in master side, 2^n * sDataWidth
  */
class SRAMWidthConverter(sDataWidth: Int, sAddrWidth: Int, sHasMask: Boolean, mDataWidth: Int) extends Module {
  assert(mDataWidth%sDataWidth == 0 && isPow2(mDataWidth/sDataWidth))
  val offsetWidth = log2Ceil(mDataWidth/sDataWidth)
  val mAddrWidth = sAddrWidth - offsetWidth
  val io = IO(new Bundle{
    val s = new SRAMIO(sDataWidth, sAddrWidth, sHasMask)
    val m = Flipped(new SRAMIO(mDataWidth, mAddrWidth, true))
  })
  assert(sDataWidth%8 == 0 && isPow2(sDataWidth/8))
  val sDataByte = sDataWidth/8
  //  val sLgDataByte = log2Ceil(sDataByte)
  val offset = { if(offsetWidth > 0) io.s.addr(offsetWidth-1, 0) else 0.U }
  io.m.en := io.s.en
  io.m.addr := io.s.addr(sAddrWidth-1, offsetWidth)
  val we = {if(sHasMask) io.s.we else Mux(io.s.we.asBool, ((1 << sDataByte) - 1).U, 0.U)}
  //  io.m.we := we << Cat(offset, 0.U(sLgDataByte.W))
  //  io.m.din := io.s.din << Cat(offset, 0.U(log2Ceil(sDataWidth).W))
  //  io.s.dout := io.m.dout >> Cat(offset, 0.U(log2Ceil(sDataWidth).W))
  io.m.we := 0.U
  io.m.din := 0.U
  io.s.dout := 0.U
  for( i <- 0 until 1 << offsetWidth){
    when(offset === i.U){
      io.m.we := we << (i*sDataByte).U
      io.m.din := io.s.din << (i*sDataWidth).U
      io.s.dout := io.m.dout >> (i*sDataWidth).U
    }
  }
}

// object VerilogGen extends App {
//   (new chisel3.stage.ChiselStage).emitVerilog(new LUT(2, 4),args)
// }
