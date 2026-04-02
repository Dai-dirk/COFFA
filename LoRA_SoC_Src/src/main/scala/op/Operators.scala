package fgramemfp.op

// divisor

import chisel3._
import chisel3.util._
import fgramemfp.common.CompileMacroVar._
import fgramemfp.dsa.UnifiedCom


/** Multiplier
 *
 * @param width   data width
 */
class Mul(width: Int) extends Module {
  val io = IO(new Bundle {
    val op0 = Input(UInt(width.W))
    val op1 = Input(UInt(width.W))
    val res = Output(UInt((2*width).W))
  })
  io.res := io.op0 * io.op1
}

/** Adder
 *
 * @param width   data width
 */
class Add(width: Int) extends Module {
  val io = IO(new Bundle {
    val op0 = Input(UInt(width.W))
    val op1 = Input(UInt(width.W))
    val res = Output(UInt(width.W))
  })
  io.res := io.op0 + io.op1
}

/**
 *A unified ALU design, which is based on Tan Cheng's paper
 *
 */
class ARITH(Width: Int) extends Module {
  val cfgDataWidth = 3
  val inTypes = {
    Seq(Width, Width)
  }.map(w => UInt(w.W))
  val outTypes = {
    Seq(Width, 1)
  }.map(w => UInt(w.W))
  val io = IO(new Bundle {
    val config = Input(UInt(cfgDataWidth.W))
    val in = Input(MixedVec(inTypes))
    val out = Output(MixedVec(outTypes))
  })
  val op1 = io.in(0) ^ Fill(Width, io.config(1))
  val op2 = io.in(1) ^ Fill(Width, io.config(2))
  val carry = io.config(2) | io.config(1)
  val sum = op1 + op2 + carry
  val MSB = sum(Width-1) ^ io.config(0) //@yuan: first item is used to indicate >= and < =
  io.out := Seq(sum, MSB)

}

// class ARITH(Width: Int) extends Module {
//   val cfgDataWidth = 3
//   val inTypes = {
//     Seq(Width, Width, 1)//@yuan: for addition with carrier, it requires 3-input
//   }.map(w => UInt(w.W))
//   val outTypes = {
//     Seq(Width, 1)
//   }.map(w => UInt(w.W))
//   val io = IO(new Bundle {
//     val config = Input(UInt(cfgDataWidth.W))
//     val in = Input(MixedVec(inTypes))
//     val out = Output(MixedVec(outTypes))
//   })
//   val op1 = Cat(0.U(1.W),io.in(0) ^ Fill(Width, io.config(1)))
//   val op2 = Cat(0.U(1.W),io.in(1) ^ Fill(Width, io.config(2)))  
//   val carry = Mux(io.config(2) | io.config(1), io.config(2) | io.config(1), io.config(0) & io.in(2))
//   val sum = op1 + op2 + carry
//   val MSB = Mux(io.config === 0.U, sum(Width), sum(Width-1) ^ io.config(0)) //@yuan: first item is used to indicate >= and < =; @yuan_fp: MSB can be carry bit when performing addition
//   io.out := Seq(sum, MSB)

// }

/**
 *A unified ALU design, which combine the most number of logic operations
 *
 */
class LOGIC(Width: Int) extends Module {
  val cfgDataWidth = 3
  val inTypes = {
    Seq(Width, Width)
  }.map(w => UInt(w.W))
  val outTypes = {
    Seq(Width, 1)
  }.map(w => UInt(w.W))
  val io = IO(new Bundle {
    val config = Input(UInt(cfgDataWidth.W))
    val in = Input(MixedVec(inTypes))
    val out = Output(MixedVec(outTypes))
  })
  val sums = VecInit(Seq.fill(Width)(0.U(1.W)))

  for(i <-(Width - 1) to  0 by -1){
    val logicUnit = Module(new UnifiedCom)
    logicUnit.io.a := io.in(1)(i)
    logicUnit.io.b := io.in(0)(i)
    logicUnit.io.c0 := io.config(0)
    logicUnit.io.c1 := io.config(1)
    sums(i) := logicUnit.io.c_out
  }
  val sum = Cat(sums.reverse)
  val isZero = sum.orR ^ io.config(2)
  io.out := Seq(sum, isZero)

}

/**
 *A unified ALU design, which combine the most number of logic operations
 *
 */
class SHIFT(Width: Int) extends Module {
  val cfgDataWidth = 3
  val inTypes = {
    Seq(Width, Width)
  }.map(w => UInt(w.W))
  val outTypes = {
    Seq(Width)
  }.map(w => UInt(w.W))
  val io = IO(new Bundle {
    val config = Input(UInt(cfgDataWidth.W))
    val in = Input(MixedVec(inTypes))
    val out = Output(MixedVec(outTypes))
  })
  val signBit = io.in(0)(Width - 1)
  val shn = io.in(1)(log2Ceil(Width) - 1, 0)
  val unshift = (Width.U - shn).asUInt
  val SHL_op1 = Mux(io.config(0).asBool, shn, unshift(log2Ceil(Width) - 1, 0))
  val SHL_op2 = Mux(io.config(2).asBool, Fill(Width, signBit), io.in(0))
  val SHL = (SHL_op2 << SHL_op1).asUInt
  val LSHR_op = Mux(io.config(0).asBool, unshift(log2Ceil(Width) - 1, 0), shn)
  val LSHR = (io.in(0) >> LSHR_op).asUInt
  //ASHR
//  val mask = Fill(Width, signBit) & SHL_op2
//  val ASHR = LSHR | mask

  val ASHR_or_CSH = LSHR | SHL //@: CSHL or CSHR or ASHR

  val LSH = Mux(io.config(0).asBool, SHL, LSHR)

  val SHR_res = Mux(io.config(1).asBool, LSH, ASHR_or_CSH)

//  val SHR_res = Mux(io.config(2).asBool, ASHR, SH_res)

  io.out := Seq(SHR_res)

}