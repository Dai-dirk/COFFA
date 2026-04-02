package fgramemfp.dsa

import chisel3._
import chisel3.util._
import scala.collection.mutable


//A Full Adder takes in 3 inputs (2 bits to add, and a carry bit), and outputs the sum and a carry
//Remember and use the truth table to create the combinational logic expressions for the output
class FullAdder() extends Module {
  val io = IO(new Bundle{
    val a     = Input(UInt(1.W))
    val b     = Input(UInt(1.W))
    val c_in  = Input(UInt(1.W))
    val sum   = Output(UInt(1.W))
    val c_out = Output(UInt(1.W))
  })



  // XOR gates for sum
  val sum  = io.a ^ io.b ^ io.c_in

  //and/or gates for cout ab + ac + bc
//  val cout = (io.a & io.b) | (io.a & io.c_in) | (io.b & io.c_in)
  val cout = (io.a & io.b) | ((io.a | io.b) &  io.c_in)

  // Output assignment
  io.sum  := sum
  io.c_out := cout


}

class UnifiedCom() extends Module {
  val io = IO(new Bundle{
    val a     = Input(UInt(1.W))
    val b     = Input(UInt(1.W))
    val c0  = Input(UInt(1.W))
    val c1   = Input(UInt(1.W))
    val c_out = Output(UInt(1.W))
  })



  io.c_out := (!io.c0 & !io.a & io.b) | (!io.c0 & io.a & !io.b) | (io.c1 & io.c0 & !io.a & !io.b) | (io.c0 & io.a & io.b) | (!io.c1 & !io.c0 & io.b)
}
