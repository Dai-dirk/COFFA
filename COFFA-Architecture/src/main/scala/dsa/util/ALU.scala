package fgramem.dsa

import chisel3._
import chisel3.util._
import scala.collection.mutable.ListBuffer
import fgramem.op._


/** reconfigurable arithmetic unit
 * 
 * @param width   data width
 * @param ops     operation set
 */
class ALU(width: Int, ops: ListBuffer[String]) extends Module {
//  val op_info = OpInfo(width)
  val maxNumOperands = ops.map(OpInfo.getOperandNum(_)).max
  val maxNumRes = ops.map(OpInfo.getResNum(_)).max
  val hasMAC = ops.map { op =>
    op == "MAC" || op == "CMAC"
  }.reduce(_ | _)
  val inTypes = {//@yuan: for MAC operation, which requires 3 inputs (mul_in1, mul_in2, feedback)
    if(maxNumOperands > 2 && hasMAC) Seq(width, width, width, 1)
    else if(maxNumOperands > 2) Seq(width, width, 1)
    else if(hasMAC) Seq(width, width, width)
    else Seq(width, width)
  }.map(w => UInt(w.W))
  val outTypes = {
    if(maxNumRes > 1) Seq(width, 1)
    else Seq(width)
  }.map(w => UInt(w.W))
  val cfgDataWidth = OpInfo.BasicOPCWidth
  val io = IO(new Bundle {
    val config = Input(UInt(cfgDataWidth.W))
    val in = Input(MixedVec(inTypes))
    val out = Output(MixedVec(outTypes))
  })
// println("outTypes: " + outTypes)
  val op_func_map = OpInfo(width).OpFuncs(io.in.toSeq, hasMAC)
//  val op2res = ops.map{ op =>
//    op.id.U -> op_func_map(op.toString)
//  }
//  println("ops: " + ops + " maxNumOperands: " + maxNumOperands)
  val op2res = ops.map { op =>
    (OpInfo.OPCMap(op).U -> op_func_map(op))
  }
  // println("op_func_map: " + op_func_map)
  // println("op2res: " + op2res)
  io.out.zipWithIndex.foreach{ case (out, i) =>
    val cfg2res = op2res.map{ kv =>
      kv._1 -> {
        if(kv._2.size > i) kv._2(i)
        else 0.U
      }
//      println("kv: " + kv )
    }
//    println("io.out.i : " + i + " cfg2res: " + cfg2res)
    out := MuxLookup(io.config, 0.U, cfg2res.toSeq)
  }

}

// object VerilogGen extends App {
//   (new chisel3.stage.ChiselStage).emitVerilog(new ALU(32, ListBuffer(OPC.ADD, OPC.SUB, OPC.ULT)),args)
// }
