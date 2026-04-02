package fgramemfp.op

import chisel3._
import chisel3.util._

import scala.collection.mutable

//@yuan: the top wrapper for LNS unit
class LNSTop(width: Int, logWidth: Int, maxSeg: Int) extends Module {

  val nCfgWidth = log2Ceil(width)//@hw_yuan: this configuration is used by the fixed-point format
  val polyCoeffCfgWidth = maxSeg * logWidth * 5 // in default, there is 6 segments
  val polyOrderCfgWidth = maxSeg * 5 * 6 // each segment has 5 terms, each term has a 6-bit order width; in total there are 6 * 5 * 6 = 180 bits
  val biasCfgWidth = maxSeg * 2 // each segment requires 2-bit, 00: no bias; 01: bias is 1; 10: bias is x
  val biasData = maxSeg * width // each segment has its own bias
  val breakPointCfgWidth = (maxSeg - 1) * width
  val cfgDataWidth = biasData + polyCoeffCfgWidth + polyOrderCfgWidth + breakPointCfgWidth + biasCfgWidth + nCfgWidth + 4 // 4 bits: 1-bit float-point flag; 3-bit: operation; the order： {biasData, polyCoeff, polyOrder, break points, bias select, flag, op, n}
  println("maxSeg: " + maxSeg)
  val io = IO(new Bundle() {
    val in = Input(Vec(2, UInt(width.W)))
    val config = Input(UInt(cfgDataWidth.W))
    val out = Output(UInt(width.W))
  })
  val VEC = Wire(Bool())
  val isNoSqrt = Wire(Bool())
  val isTrg = Wire(Bool())
  val isTri = Wire(Bool())
  val isDiv = Wire(Bool())
  val isPower = Wire(Bool())
  val isLog = Wire(Bool())

  //  Config elements
  //  [name, (id, high-bit, low-bit)]
  val cfg_idx: mutable.Map[String, (Int, Int, Int)] = mutable.Map()

  var offset = 0
  var id = 0
  val nInput = io.config(nCfgWidth - 1, 0)
  cfg_idx += "n" -> (id, nCfgWidth - 1, offset)
  offset += nCfgWidth
  id += 1

  //@yuan: decode logic for expected operation
  when(io.config(offset + 2, offset) === 0.U){ //multiply
    VEC := true.B
    isNoSqrt := true.B
    isTrg := true.B
    isTri := false.B
    isDiv := false.B
    isPower := false.B
    isLog := false.B
  }.elsewhen(io.config(offset + 2, offset) === 1.U){//divide
    VEC := true.B
    isNoSqrt := true.B
    isTrg := true.B
    isTri := false.B
    isDiv := true.B
    isPower := false.B
    isLog := false.B
  }.elsewhen(io.config(offset + 2, offset) === 2.U) { //multiply, one of the operands performs sqrt
    VEC := true.B
    isNoSqrt := false.B
    isTrg := true.B
    isTri := false.B
    isDiv := false.B
    isPower := false.B
    isLog := false.B
  }.elsewhen(io.config(offset + 2, offset) === 3.U) { //divide, one of the operands performs sqrt
    VEC := true.B
    isNoSqrt := false.B
    isTrg := true.B
    isTri := false.B
    isDiv := true.B
    isPower := false.B
    isLog := false.B
  }.elsewhen(io.config(offset + 2, offset) === 4.U) { //polynomial
    VEC := false.B
    isNoSqrt := true.B
    isTrg := true.B
    isTri := true.B
    isDiv := false.B
    isPower := false.B
    isLog := false.B
  }.elsewhen(io.config(offset + 2, offset) === 5.U) { //power
    VEC := false.B
    isNoSqrt := true.B
    isTrg := false.B
    isTri := false.B
    isDiv := false.B
    isPower := true.B
    isLog := false.B
  }.elsewhen(io.config(offset + 2, offset) === 6.U) { //log
    VEC := false.B
    isNoSqrt := true.B
    isTrg := false.B
    isTri := false.B
    isDiv := false.B
    isPower := false.B
    isLog := true.B
  }.otherwise{
    VEC := true.B
    isNoSqrt := true.B
    isTrg := true.B
    isTri := false.B
    isDiv := false.B
    isPower := false.B
    isLog := false.B
  }
  cfg_idx += "opc" -> (id, offset + 2, offset)
  offset += 3
  id += 1
  //LNS unit instance
  val is_fp = io.config(offset)
  cfg_idx += "fp_flag" -> (id, offset, offset)
  id += 1
  offset += 1
  val bias_in = io.config(offset + biasCfgWidth - 1, offset)
  cfg_idx += "bias_in" -> (id, offset + biasCfgWidth - 1, offset)
  id += 1
  offset += biasCfgWidth
  val break_points = io.config(offset + breakPointCfgWidth - 1, offset)
  for(i <- 0 until maxSeg - 1){
    cfg_idx += ("break_points_" + i) -> (id, offset + width - 1, offset)
    offset += width
    id += 1
  }
  val constant_bias = io.config(offset + biasData - 1, offset)
  for (i <- 0 until maxSeg) {
    cfg_idx += ("constant_bias_" + i) -> (id, offset + width - 1, offset)
    offset += width
    id += 1
  }
//  cfg_idx += "constant_bias" -> (id, offset + biasData - 1, offset)
//  id += 1
//  offset += biasData

  class LNS_Top extends BlackBox(Map(
    "WIDTH" -> logWidth,
    "WIDTH32" -> width,
  )) with HasBlackBoxResource{
    val io = IO(new Bundle {
      val clk = Input(Clock())
      val rstn = Input(Bool())
      val n = Input(UInt(nCfgWidth.W))
      val x_0 = Input(UInt(width.W))
      val y_0 = Input(UInt(width.W))
      val VEC = Input(Bool())
      val qi = Input(Bool())
      val Div = Input(Bool())
      val float_flag = Input(Bool())
      val logc_in = Input(Vec(maxSeg, UInt((logWidth * 5).W)))
      val K_in = Input(Vec(maxSeg, UInt(30.W)))
      val bias_sel = Input(UInt(biasCfgWidth.W))
      val constant_bias_in = Input(UInt(biasData.W))
      val break_points = Input(UInt(((maxSeg - 1) * width).W))
      val TRG = Input(Bool())
      val power = Input(Bool())
      val TRi = Input(Bool())

      //output ports
      val TRG_overflow = Output(Bool())
      val VEC_overflow = Output(Bool())
      val CPA_float_overflow = Output(Bool())
      val CPA_float_underflow = Output(Bool())
      val CPA_cpa_overflow = Output(Bool())
      val channel0_log_result = Output(UInt(width.W))
      val channel2_tri_result = Output(UInt(width.W))
      val channel0_VEC_Power_result = Output(UInt(width.W))
    })
    addResource("/vsrc/LNS_Top.v")  
    addResource("/vsrc/anticonverter.v")  
    addResource("/vsrc/converter.v")  
    addResource("/vsrc/CPA_tree_with_MAD.v")  
    addResource("/vsrc/CSA_tree.v")  
    addResource("/vsrc/segSel.v")  
  }

  val lns = Module(new LNS_Top)
  lns.io.clk := clock
  lns.io.rstn := !reset.asBool
  lns.io.n := nInput
  lns.io.x_0 := io.in(0)
  lns.io.y_0 := io.in(1)
  lns.io.VEC := VEC
  lns.io.qi := isNoSqrt
  lns.io.Div := isDiv
  lns.io.float_flag := is_fp
  lns.io.bias_sel := bias_in
  lns.io.break_points := break_points
  lns.io.constant_bias_in := constant_bias
  lns.io.TRG := isTrg
  lns.io.power := isPower
  lns.io.TRi := isTri

  //@yuan: we connect the polynomial coefficient and order here
  lns.io.K_in.zipWithIndex.foreach{ case (in, i) =>
        cfg_idx += ("K_in_" + i) -> (id, offset + 29, offset)
        id += 1
        in := io.config(offset + 29, offset)
        offset += 30
  }
  lns.io.logc_in.zipWithIndex.foreach{ case (in, i) =>
        in := io.config(offset + logWidth * 5 - 1, offset)
        for(j <- 0 until 5){
          cfg_idx += ("logc_" + i + "_" + j) -> (id, offset + logWidth - 1, offset)
          offset += logWidth
          id += 1
        }
  }

  io.out := Mux1H(
    Seq(VEC|isPower, isTri, isLog),
    Seq(
      lns.io.channel0_VEC_Power_result,
      lns.io.channel2_tri_result,
      lns.io.channel0_log_result
    )
  )
}

object VerilogLNSTopGen extends App {
  (new chisel3.stage.ChiselStage).emitVerilog(new LNSTop(32, 26, 4), args)
}