package fgramemfp.op

import chisel3._
import chisel3.util._
import fgramemfp.op.FPU._

//@yuan: according to the operation requirements from HW, this module is responsible for the addition/subtraction of exponent
class FEAS(val expWidth: Int, val precision: Int) extends Module {
  val io = IO(new Bundle() {
    val a = Input(UInt((expWidth + precision).W))
    val b = Input(UInt(expWidth.W))
    val mode = Input(UInt(1.W))// mode = 0 : addition; mode = 1 : subtraction
    val result = Output(UInt((expWidth + precision).W))
  })
  val raw_a = FloatPoint.fromUInt(io.a, expWidth, precision)
  val decode_a = raw_a.decode
  //maximum and minimum exponent
  val max_exp = ((1 << expWidth) - 1)
  val min_exp = 1 - max_exp

  val BIAS = (1 << (expWidth - 1)) - 1
  val MIN_NORMAL_EXP = 1 - BIAS

  val shift_amount = Wire(UInt(expWidth.W))
  val sticky_bit = Wire(Bool())
  val denorm_mant = Wire(UInt((precision-1).W))

  // normalized variables
  val normalized_mant = Wire(UInt((precision - 1).W))

  // result of addition or subtraction
  val result_exp = Mux(io.mode.asBool, Cat(0.U(2.W), raw_a.exp) - Cat(0.U(2.W), io.b), Cat(0.U(2.W), raw_a.exp) + Cat(0.U(2.W), io.b))

  // the new exponent
  val new_exp = Wire(UInt(expWidth.W))
  //set the flag for overflow or underflow
  val overflow = result_exp.asSInt > max_exp.asSInt
  val underflow = result_exp.asSInt < min_exp.asSInt

  //denormal
  val underflow_depth = (MIN_NORMAL_EXP.S - result_exp.asSInt).asUInt
  when(result_exp.asSInt < MIN_NORMAL_EXP.S && !decode_a.isZero) {
    // 计算需要右移的位数
    shift_amount := Mux(underflow_depth < (precision + 2).U,
      underflow_depth,
      (precision + 2).U)

    // 保留附加位用于舍入
    val extended_mant = Cat(raw_a.sig, 0.U(3.W)) // 添加保护位、舍入位和粘滞位
    val shifted = extended_mant >> shift_amount

    // 分离尾数和粘滞位
    denorm_mant := shifted(extended_mant.getWidth - 1, 3)
    sticky_bit := shifted(2, 0).orR
  }.otherwise {
    shift_amount := DontCare
    denorm_mant := 0.U
    sticky_bit := false.B
  }

  when(decode_a.isNaN){
    new_exp := max_exp.asUInt
    normalized_mant := raw_a.sig
  }.elsewhen(decode_a.isInf) {
    new_exp := max_exp.asUInt
    normalized_mant := 0.U
  }.elsewhen(decode_a.isZero){
    new_exp := 0.U
    normalized_mant := 0.U
  }.elsewhen(overflow){
    new_exp := max_exp.asUInt
    normalized_mant := 0.U
  }.elsewhen(underflow){//TODO: maybe it can be more accurate
    // when underflow happens, we set the operand a as minimum number
    new_exp := 0.U
    normalized_mant := denorm_mant | sticky_bit
  }.otherwise{
    new_exp := result_exp(expWidth - 1, 0)
    normalized_mant := raw_a.sig
  }

  io.result := RegNext(Cat(raw_a.sign, new_exp, normalized_mant))
// println("max_exp: " + max_exp)
}

object VerilogFEASGen extends App {
  (new chisel3.stage.ChiselStage).emitVerilog(new FEAS(8,24), args)
}