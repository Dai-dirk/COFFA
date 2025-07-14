package fgramem.op

// divisor

import chisel3._
import chisel3.util._
import fgramem.common.CompileMacroVar._


/** Multiplier
 *
 * @param width   data width
 */
class Mul(width: Int) extends Module {
  val io = IO(new Bundle {
    val op0 = Input(UInt(width.W))
    val op1 = Input(UInt(width.W))
    val res = Output(UInt(width.W))
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