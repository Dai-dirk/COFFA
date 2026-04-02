//package op.XCore
//
//import chisel3._
//import chisel3.util._
//
//import scala.collection.mutable
//
////@yuan: the top wrapper for LNS unit
//class LNSTop(width: Int, logWidth: Int, maxSeg: Int) extends Module {
//
//  val nCfgWidth = log2Ceil(width)
//  val polyCoeffCfgWidth = maxSeg * logWidth * 4 // in default, there is 5 segments
//  val polyOrderCfgWidth = maxSeg * 4 * 6 // each segment has 4 terms, each term has a 6-bit order width; in total there are 5 * 4 * 6 = 120 bits
//  val biasDfgWidth = maxSeg * 2 // each segment requires 2-bit, 00: no bias; 01: bias is 1; 10: bias is x
//  val breakPointCfgWidth = (maxSeg - 1) * width
//  val cfgDataWidth = polyCoeffCfgWidth + polyOrderCfgWidth + breakPointCfgWidth + biasDfgWidth + nCfgWidth + 5 // 5 bits: 1-bit float-point flag; 4-bit: operation; {polyCoeff, polyOrder, break points, bias select, flag, op, n}
//
//  val io = IO(new Bundle() {
//    val in = Input(Vec(2, UInt(width.W)))
//    val config = Input(UInt(cfgDataWidth.W))
//    val out = Output(UInt(width.W))
//  })
//  val VEC = Wire(Bool())
//  val isNoSqrt = Wire(Bool())
//  val isTrg = Wire(Bool())
//  val isTri = Wire(Bool())
//  val isDiv = Wire(Bool())
//  val isPower = Wire(Bool())
//  val isLog = Wire(Bool())
//  val triSelect = Wire(UInt(3.W))
//
//  //  Config elements
//  //  [name, (id, high-bit, low-bit)]
//  val cfg_idx: mutable.Map[String, (Int, Int, Int)] = mutable.Map()
//
//  var offset = 0
//  var id = 0
//  val nInput = io.config(nCfgWidth - 1, 0)
//  cfg_idx += "n" -> (id, nCfgWidth - 1, offset)
//  offset += nCfgWidth
//  id += 1
//
//  //@yuan: decode logic for expected operation
//  when(io.config(offset + 3, offset) === 0.U){ //multiply
//    VEC := true.B
//    isNoSqrt := true.B
//    isTrg := true.B
//    isTri := false.B
//    isDiv := false.B
//    isPower := false.B
//    isLog := false.B
//    triSelect := 0.U
//  }.elsewhen(io.config(offset + 3, offset) === 1.U){//divide
//    VEC := true.B
//    isNoSqrt := true.B
//    isTrg := true.B
//    isTri := false.B
//    isDiv := true.B
//    isPower := false.B
//    isLog := false.B
//    triSelect := 0.U
//  }.elsewhen(io.config(offset + 3, offset) === 2.U) { //multiply, one of the operands performs sqrt
//    VEC := true.B
//    isNoSqrt := false.B
//    isTrg := true.B
//    isTri := false.B
//    isDiv := false.B
//    isPower := false.B
//    isLog := false.B
//    triSelect := 0.U
//  }.elsewhen(io.config(offset + 3, offset) === 3.U) { //divide, one of the operands performs sqrt
//    VEC := true.B
//    isNoSqrt := false.B
//    isTrg := true.B
//    isTri := false.B
//    isDiv := true.B
//    isPower := false.B
//    isLog := false.B
//    triSelect := 0.U
//  }.elsewhen(io.config(offset + 3, offset) === 4.U) { //sin
//    VEC := false.B
//    isNoSqrt := true.B
//    isTrg := true.B
//    isTri := true.B
//    isDiv := false.B
//    isPower := false.B
//    isLog := false.B
//    triSelect := 1.U
//  }.elsewhen(io.config(offset + 3, offset) === 5.U) { //cos
//    VEC := false.B
//    isNoSqrt := true.B
//    isTrg := true.B
//    isTri := true.B
//    isDiv := false.B
//    isPower := false.B
//    isLog := false.B
//    triSelect := 2.U
//  }.elsewhen(io.config(offset + 3, offset) === 6.U) { //arcsin
//    VEC := false.B
//    isNoSqrt := true.B
//    isTrg := true.B
//    isTri := true.B
//    isDiv := false.B
//    isPower := false.B
//    isLog := false.B
//    triSelect := 3.U
//  }.elsewhen(io.config(offset + 3, offset) === 7.U) { //arccos
//    VEC := false.B
//    isNoSqrt := true.B
//    isTrg := true.B
//    isTri := true.B
//    isDiv := false.B
//    isPower := false.B
//    isLog := false.B
//    triSelect := 4.U
//  }.elsewhen(io.config(offset + 3, offset) === 8.U) { //polynomial
//    VEC := false.B
//    isNoSqrt := true.B
//    isTrg := true.B
//    isTri := true.B
//    isDiv := false.B
//    isPower := false.B
//    isLog := false.B
//    triSelect := 5.U
//  }.elsewhen(io.config(offset + 3, offset) === 9.U) { //power
//    VEC := false.B
//    isNoSqrt := true.B
//    isTrg := false.B
//    isTri := false.B
//    isDiv := false.B
//    isPower := true.B
//    isLog := false.B
//    triSelect := 0.U
//  }.elsewhen(io.config(offset + 3, offset) === 10.U) { //log
//    VEC := false.B
//    isNoSqrt := true.B
//    isTrg := false.B
//    isTri := false.B
//    isDiv := false.B
//    isPower := false.B
//    isLog := true.B
//    triSelect := 0.U
//  }.otherwise{
//    VEC := true.B
//    isNoSqrt := true.B
//    isTrg := true.B
//    isTri := false.B
//    isDiv := false.B
//    isPower := false.B
//    isLog := false.B
//    triSelect := 0.U
//  }
//  cfg_idx += "opc" -> (id, offset + 3, offset)
//  offset += 4
//  id += 1
//  //LNS unit instance
//  val is_fp = io.config(offset)
//  cfg_idx += "fp_flag" -> (id, offset, offset)
//  id += 1
//  offset += 1
//  val bias_in = io.config(offset + biasDfgWidth - 1, offset)
//  cfg_idx += "bias_in" -> (id, offset + biasDfgWidth - 1, offset)
//  id += 1
//  offset += biasDfgWidth
//  val break_points = io.config(offset + breakPointCfgWidth - 1, offset)
//  cfg_idx += "bias_in" -> (id, offset + breakPointCfgWidth - 1, offset)
//  id += 1
//  offset += breakPointCfgWidth
//
//  class LNS_Top extends BlackBox(Map(
//    "WIDTH" -> logWidth,
//    "WIDTH32" -> width,
//  )) {
//    val io = IO(new Bundle {
//      val clk = Input(Clock())
//      val rstn = Input(Bool())
//      val n = Input(UInt(nCfgWidth.W))
//      val x_0 = Input(UInt(width.W))
//      val y_0 = Input(UInt(width.W))
//      val TRI_select = Input(UInt(3.W))
//      val VEC = Input(Bool())
//      val qi = Input(Bool())
//      val Div = Input(Bool())
//      val float_flag = Input(Bool())
//      val logc_in = Input(Vec(maxSeg, UInt((logWidth * 4).W)))
//      val K_in = Input(Vec(maxSeg, UInt(24.W)))
//      val bias_sel = Input(UInt(biasDfgWidth.W))
//      val break_points = Input(UInt(((maxSeg - 1) * width).W))
//      val TRG = Input(Bool())
//      val power = Input(Bool())
//      val TRi = Input(Bool())
//
//      //output ports
//      val TRG_overflow = Output(Bool())
//      val VEC_overflow = Output(Bool())
//      val channel0_log_result = Output(UInt(width.W))
//      val channel2_tri_result = Output(UInt(width.W))
//      val channel0_VEC_Power_result = Output(UInt(width.W))
//    })
//  }
//
//  val lns = Module(new LNS_Top)
//  lns.io.clk := clock
//  lns.io.rstn := !reset.asBool
//  lns.io.n := nInput
//  lns.io.x_0 := io.in(0)
//  lns.io.y_0 := io.in(1)
//  lns.io.TRI_select := triSelect
//  lns.io.VEC := VEC
//  lns.io.qi := isNoSqrt
//  lns.io.Div := isDiv
//  lns.io.float_flag := is_fp
//  lns.io.bias_sel := bias_in
//  lns.io.break_points := break_points
//  lns.io.TRG := isTrg
//  lns.io.power := isPower
//  lns.io.TRi := isTri
//
//  //@yuan: we connect the polynomial coefficient and order here
//  cfg_idx += "bias_in" -> (id, offset + polyOrderCfgWidth - 1, offset)
//  id += 1
//  lns.io.K_in.zipWithIndex.foreach{ case (in, i) =>
//        in := io.config(offset + 23, offset)
//        offset += 24
//  }
//  cfg_idx += "logc" -> (id, offset + polyCoeffCfgWidth - 1, offset)
//  lns.io.logc_in.zipWithIndex.foreach{ case (in, i) =>
//        in := io.config(offset + logWidth * 4 - 1, offset )
//        offset += logWidth * 4
//  }
//
//  io.out := Mux1H(
//    Seq(VEC|isPower, isTri, isLog),
//    Seq(
//      lns.io.channel0_VEC_Power_result,
//      lns.io.channel2_tri_result,
//      lns.io.channel0_log_result
//    )
//  )
//}
//
//object VerilogLNSTopGen extends App {
//  (new chisel3.stage.ChiselStage).emitVerilog(new LNSTop(32, 26, 5), args)
//}