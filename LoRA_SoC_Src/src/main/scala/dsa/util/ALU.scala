package fgramemfp.dsa

import chisel3._
import chisel3.util._
import scala.collection.mutable.{ArrayBuffer, ListBuffer}
import fgramemfp.op._
import scala.collection.mutable
import fgramemfp.op._




/** reconfigurable arithmetic unit
 * 
 * @param width   data width
 * @param ops     operation set
 */
class ALU(width: Int, ops: ListBuffer[String]) extends Module {
//  println("ops: " + ops)
  //  val op_info = OpInfo(width)
  //  println("OPCMap: " + OpInfo.OPCMap)
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
  val OpType = ops.map(OpInfo.getOpType(_)).distinct
  val UnifiedCfgWidth = {
    if(OpType.contains("ARITH") || OpType.contains("LOGIC") || OpType.contains("SHIFT") || OpType.contains("FADDSUB")|| OpType.contains("FCMP") || OpType.contains("FEXE")){
      3
    }else{
      0
    }
  }
  val cfgDataWidth = OpInfo.BasicOPCWidth + UnifiedCfgWidth
  //println("BasicOPCWidth: " + OpInfo.BasicOPCWidth + " UnifiedCfgWidth: " + UnifiedCfgWidth + " FPDivCfgWidth: " + FPDivCfgWidth)
  val io = IO(new Bundle {
    val config = Input(UInt(cfgDataWidth.W))
    val in = Input(MixedVec(inTypes))
    val out = Output(MixedVec(outTypes))
  })
  // println("outTypes: " + outTypes)
  val UnifiedCfg = Wire(UInt(3.W))
  var offset = 0
  if(UnifiedCfgWidth != 0){
    UnifiedCfg := io.config(offset + UnifiedCfgWidth - 1, offset)
    offset += 3
  }else{
    UnifiedCfg := DontCare
  }  
  val fpmulsel = OpType.map { op =>
    OpInfo.OPCMap(OpInfo.getTypicalOp(op)).U(OpInfo.BasicOPCWidth - 1, 0) -> {
      if (op == "FMUL") {
        true.B
      } else {
        false.B
      }
    }
  }
//  println("fpmulsel: " + fpmulsel)
  val isFmul = MuxLookup(io.config(cfgDataWidth - 1, offset), false.B, fpmulsel.toSeq)
  val op_func_map = OpInfo(width).OpFuncs(io.in.toSeq, UnifiedCfg, hasMAC, isFmul)

  //  val op2res = ops.map{ op =>
  //    op.id.U -> op_func_map(op.toString)
  //  }
//    println("ops: " + ops + " maxNumOperands: " + maxNumOperands + " cfgDataWidth: " + cfgDataWidth)
  val op2res = OpType.map { op =>
//    println("op: " + op + " op UInt: " + OpInfo.OPCMap(OpInfo.getTypicalOp(op)).U)
    (OpInfo.OPCMap(OpInfo.getTypicalOp(op)).U(OpInfo.BasicOPCWidth - 1, 0) -> op_func_map(op))

  }
//  val op2resUnique = op2res.groupBy(_._1).mapValues(_.flatMap(_._2).distinct)
//    ops.map { op =>
//      println("op: "+ op + " op UInt: " + OpInfo.OPCMap(op).U)
//    }
//    println("op_func_map: " + op_func_map)
    // println("op2res: " + op2res)
//  println("op2resUnique: " + op2resUnique)
  //  val out_reg = RegInit(MixedVec(outTypes))
  io.out.zipWithIndex.foreach{ case (out, i) =>
    val cfg2res = op2res.map{ kv =>
      kv._1 -> {
        if(kv._2.size > i) kv._2(i)
        else 0.U
      }
      //      println("kv: " + kv )
    }
    out := MuxLookup(io.config(cfgDataWidth - 1, offset), 0.U, cfg2res.toSeq)
//    out := RegNext(MuxLookup(io.config(cfgDataWidth - 1, offset), 0.U, cfg2res.toSeq))

//    out := RegNext(MuxLookup(io.config, 0.U, cfg2res.toSeq))
  }
  //println("mode test: " + (-15 % 3))
}

// class ALU(width: Int, ops: ListBuffer[String]) extends Module {//@yuan: for test
// //  val op_info = OpInfo(width)
//   val maxNumOperands = ops.map(OpInfo.getOperandNum(_)).max
//   val maxNumRes = ops.map(OpInfo.getResNum(_)).max
//   val inTypes = {
//     if(maxNumOperands > 2) Seq(width, width, 1)
//     else Seq(width, width)
//   }.map(w => UInt(w.W))
//   val outTypes = {
//     if(maxNumRes > 1) Seq(width, 1)
//     else Seq(width)
//   }.map(w => UInt(w.W))
//   val cfgDataWidth = OpInfo.BasicOPCWidth
//   val io = IO(new Bundle {
//     val config = Input(UInt(cfgDataWidth.W))
//     val in = Input(MixedVec(inTypes))
//     val out = Output(MixedVec(outTypes))
//   })
//   println("ops: " + ops)
// println("outTypes: " + outTypes)
//   val op_func_map = OpInfo(width).OpFuncs(io.in.toSeq, 0.U, false)
// //  val op2res = ops.map{ op =>
// //    op.id.U -> op_func_map(op.toString)
// //  }
//   val op2res = ops.map { op =>
//     (OpInfo.OPCMap(op).U -> op_func_map(op))
//   }
//   println("op2res: " + op2res)
//   io.out.zipWithIndex.foreach{ case (out, i) =>
//     val cfg2res = op2res.map{ kv =>
//       kv._1 -> {
//         if(kv._2.size > i) kv._2(i)
//         else 0.U
//       }
//     }
// //    println("io.out.i : " + i)
//     out := MuxLookup(io.config, 0.U, cfg2res.toSeq)
//   }

// }

/** MIMD-ALU that supports op-level parallelism
 * @param Width data witdh
 * @param ops   operation set
 *
 */
class ALU_Unified(Width: Int, ops: ListBuffer[String], numRegRF4ALU: Int) extends Module{
  val numOriginalOpCG = 2
  val numEUOutCG = 1
  val numInCG = {
    val isMix = !isPow2(ops.map(OpInfo.getOpTypeId(_)).reduce(_ | _))
    if (isMix) {
      4
    } else {
      2
    }
  }
  val numInFG = {// depends on operation (e.g., SEL)
    val aluOperandNum = ops.map(OpInfo.getALUOperandNum(_)).max
    aluOperandNum - numOriginalOpCG
  }

  val OpType = ops.map(OpInfo.getOpType(_)).distinct
  println("OpType: " + OpType)

  val numEU = OpType.size

  val numPerOpCG = numInCG + numEU - 2 // the number of inputs for each EU's Operand, inputs (the first two inputs of ALU are connected to each EU's first two operand respectively) + other EU's feedback

  val numEUhasFGOut : Int = {
    var num = 0
    OpType.map { op =>
      if (op == "SHIFT" || op == "FEAS" || op == "FADDSUB") {
        num += 0
      } else if (op == "ARITH" || op == "LOGIC" || op == "FCMP") {
        num += 1
      } else {
        val resNum = OpInfo.getResNum(op)
        num += (resNum - numEUOutCG)
      }
    }
      num
  }
  val numOutFG = {if(numEUhasFGOut > 0) 1 else 0}

  
  val numOpFG = {
    if(numInFG > 0){
      numInFG + numEUhasFGOut  //@yuan_hw: for ARITH unit, it can receive and generate fine-grained signals
    }else{
      0
    }
  }
//  println("numOpFG: " + numOpFG + " numInFG: " + numInFG + " numEUhasFGOut: " + numEUhasFGOut)

  val eus = new ArrayBuffer[EU]() // the collection of executing unit
  val EUhasFGInId: ListBuffer[Int] = ListBuffer()

  for(i <- 0 until OpType.size){
      val op = OpType(i)
    if(OpInfo.OPCMap.contains(op)){
      if (OpInfo.getOperandNum(op) > 2) {
        EUhasFGInId += eus.size
        eus += Module(new EU(Width, op, 2*numPerOpCG, numOpFG, numRegRF4ALU))
      }else{
        eus += Module(new EU(Width, op, 2*numPerOpCG, 0, numRegRF4ALU))
      }
    }else{
      eus += Module(new EU(Width, op, 2*numPerOpCG, 0, numRegRF4ALU))
    }
  }

  val EUCfgWidthList = eus.map(eu => eu.io.config.getWidth)
  val EUCfgWidth = EUCfgWidthList.sum

  //@hw_yuan: connect the EU's output to mux
  //coarse-grained
  val outMuxCG = Module(new Muxn(Width, eus.size))
  outMuxCG.io.in.zipWithIndex.foreach{ case (in, j) =>
    in := eus(j).io.out_cg(0)
  }

  //fine-grained
  val outMuxFG = {
      if(numEUhasFGOut > 0){
        Module(new Muxn(1, numEUhasFGOut))
      }else{
        null
      }
  }
  val EUhasFGOutId: ListBuffer[Int] = ListBuffer()
  if(numEUhasFGOut > 0){
    eus.zipWithIndex.foreach{case (eu, j) =>
      if(eu.io.out_fg.getWidth > 0){
        EUhasFGOutId += j
      }
    }//@hw_yuan: the size of EUhasFGOutId should equals to the number of outMuxFG's input
    assert(EUhasFGOutId.size == numEUhasFGOut)
    outMuxFG.io.in.zipWithIndex.foreach{ case(in, j) =>
      in := eus(EUhasFGOutId(j)).io.out_fg(0)
    }
  }

  //configuration width
  val outMuxCGCfgWidth = outMuxCG.io.config.getWidth
  val outMuxFGCfgWidth = {if(numEUhasFGOut > 0) outMuxFG.io.config.getWidth else 0}

  val cfgDataWidth = EUCfgWidth + outMuxCGCfgWidth + outMuxFGCfgWidth

  //ALU's IO
  val io = IO(new Bundle {
    val config = Input(UInt(cfgDataWidth.W))
    val in_cg = Input(Vec(numInCG, UInt(Width.W)))
    val out_cg = Output(Vec(numEUOutCG, UInt(Width.W)))
    val in_fg = Input(Vec(numInFG, UInt(1.W)))
    val out_fg = Output(Vec(numOutFG, UInt(1.W)))
  })


  //EU's input
  //coarse-grained: ALU input + other EU's feedback
  eus.zipWithIndex.foreach{ case (eu, i) =>
    eu.io.in_cg.zipWithIndex.foreach{ case (in, j) =>
      if((j % numPerOpCG) < numInCG - 1){// the first ports connected to ALU's input
        if((j % numPerOpCG) == 0){
          in := io.in_cg(j / numPerOpCG)
        }else{
          in := io.in_cg((j % numPerOpCG) + 1)
        }
      }else{
        val feedBackBaseIdx = (j % numPerOpCG) - (numInCG - 1)
        if(feedBackBaseIdx >= i){ // skip itself
          in := eus(feedBackBaseIdx + 1).io.out_cg(0)
        }else{
          in := eus(feedBackBaseIdx).io.out_cg(0)
        }
      }
    }
    // println("eu: " + eu.operation)
  }

  //fine-grained: ALU input + other EU's feedback
  if(numInFG > 0){// only parts of EU has fine-grained input
    EUhasFGInId.zipWithIndex.foreach{ case(id, i) =>
      eus(id).io.in_fg.zipWithIndex.foreach{ case(in, j) =>
        if(j < numInFG){
          in := io.in_fg(j)
        }else{// the fact is, the EU requires FG input doesn't generate FG output
          in := eus(EUhasFGOutId(j - numInFG)).io.out_fg(0)
        }
      }
    }
  }
  
  //EU's configuration
  //  Config elements
  //  [name, (id, high-bit, low-bit, hasFGOut, imuxFGWidth, imuxCGWidth, CoreCfgWidth)]
  val cfg_idx: mutable.Map[String, (Int, Int, Int, Boolean, Int, Int, Int)] = mutable.Map()
  var offset = 0
  var id = 0
  EUCfgWidthList.zipWithIndex.foreach{ case (w, i) =>
    if(w != 0){
      eus(i).io.config := io.config(w + offset - 1, offset)
      cfg_idx += eus(i).operation -> (id, w+offset-1, offset, EUhasFGOutId.contains(i), eus(i).imuxFGCfgWidth, eus(i).imuxCGCfgWidth, eus(i).CorecfgWidth)
      id += 1
    }else{
      eus(i).io.config := DontCare
    }
    offset += w
  }

  //outMuxCG's configuration
  if(outMuxCGCfgWidth != 0){
    outMuxCG.io.config := io.config(offset + outMuxCGCfgWidth - 1, offset)
    cfg_idx += "outMuxCG" -> (id, outMuxCGCfgWidth + offset - 1, offset, false, 0, 0, 0)
    id += 1
    offset += outMuxCGCfgWidth
  }else{
    outMuxCG.io.config := DontCare
  }

  //outMuxFG's configuration
  if(outMuxFGCfgWidth != 0){
    outMuxFG.io.config := io.config(offset + outMuxFGCfgWidth - 1, offset )
    cfg_idx += "outMuxFG" -> (id, outMuxFGCfgWidth + offset - 1, offset, false, 0, 0, 0)
    id += 1
  }else{
    if(numEUhasFGOut > 0){
      outMuxFG.io.config := DontCare
    }
  }

  //output
  io.out_cg(0) := outMuxCG.io.out
  if(numOutFG > 0){
    io.out_fg(0) := outMuxFG.io.out
  }



}

/** Executing Unit (EU), which includes Operator Core for different operations
 * @param Witdh data width
 * @param op     operation
 * @param numInCG     number of coarse-grained input ports
 * @param numInFG     number of fine-grained input ports
 * @param numRegRF4ALU number of register for ALU
 * */

class EU(Width: Int, op: String, numInCG: Int, numInFG: Int, numRegRF4ALU: Int) extends Module{
  assert(numRegRF4ALU >= 1) //@hw_yuan: at least 1 reg
  val numOutCG = 1
  val numOpCG = 2
  val operation = op
  val numOutFG = {//@yuan_fp
    if (OpInfo.OPCMap.contains(op)) {
      val aluResNum = OpInfo.getResNum(op)
      aluResNum - numOutCG
    }else{
      if (op == "SHIFT" || op == "FADDSUB"|| op == "FMUL" || op == "FEAS") { // SHIFT doesn't have 1-bit output
        0
      } else{
        1
      }
    }
  }
  val numOpFG = {
    if (OpInfo.OPCMap.contains(op)) {
      val maxNumOperands = OpInfo.getOperandNum(op)
      if (maxNumOperands > 2) {
        maxNumOperands - numOpCG
      } else {
        0
      }
    }else{//@hw_yuan: unified operator core doesn't have fine-grained input
      0
    }
  }
  assert(((numOpFG == 0) && (numInFG == 0)) || ((numOpFG > 0) && (numInFG > 0)))
  val imuxsCG = {
    (0 until numOpCG).map { i =>
      Module(new Muxn(Width, numInCG/2)).io
    }
  }
  val imuxsFG = {
    (0 until numOpFG).map{ i =>
      Module(new Muxn(1, numInFG)).io
    }
  }
  val CorecfgWidth = { //@hw_yuan: currently, each unified execute unit has 3-bit configuration
    if (OpInfo.OPCMap.contains(op)) {
      0
    } else { //@hw_yuan: for unified unit
      3
    }
  }
  val imuxCGCfgWidthList = imuxsCG.map { mux => mux.config.getWidth } // input CG-Muxes
  val imuxFGCfgWidthList = imuxsFG.map { mux => mux.config.getWidth } // input FG-Muxes
  val cfgDataWidth = CorecfgWidth + imuxCGCfgWidthList.sum + imuxFGCfgWidthList.sum
  val io = IO(new Bundle {
    val config = Input(UInt(cfgDataWidth.W))
    val in_cg = Input(Vec(numInCG, UInt(Width.W)))
    val out_cg = Output(Vec(numOutCG, UInt(Width.W)))
    val in_fg = Input(Vec(numInFG, UInt(1.W)))
    val out_fg = Output(Vec(numOutFG, UInt(1.W)))
  })
  //sub-module instance
  val opCore = Module(new Operator_Core(Width, op))
//  val rfCG = Module(new RF(Width, numRegRF4ALU, 1, 1))
//  val rfFG : RF = {if(numOutFG > 0) Module(new RF(1, numRegRF4ALU, 1, 1)) else null}


  //operator connections
  var offset = 0
  for(i <- 0 until numOpCG){
    imuxsCG(i).in.zipWithIndex.foreach{ case (in, j) =>
      in := io.in_cg(offset + j)
    }
    opCore.io.in(i) := imuxsCG(i).out
    offset += numInCG/2
  }
  if(numOpFG > 0){
    offset = 0
    for(i <- 0 until numOpFG){
      imuxsFG(i).in.zipWithIndex.foreach { case (in, j) =>
        in := io.in_fg(offset + j)
      }
      opCore.io.in(i + numOpCG) := imuxsFG(i).out
      offset += numInFG
    }
  }

  // connected to RF
  io.out_cg(0) := RegNext(Cat(opCore.io.out.toSeq.reverse)(Width-1, 0))

  if(numOutFG > 0){
    io.out_fg(0) := RegNext(Cat(opCore.io.out.toSeq.reverse)(Width - 1 + numOutFG))
  }


  // configuration connection.
  offset = 0
  if(CorecfgWidth > 0) {
    opCore.io.config := io.config(CorecfgWidth - 1, 0)
    offset += CorecfgWidth
  }else{
    opCore.io.config := DontCare
  }

  imuxCGCfgWidthList.zipWithIndex.foreach { case (w, i) =>
    if (w != 0) {
      imuxsCG(i).config := io.config(w + offset - 1, offset)
    }else{
      imuxsCG(i).config := DontCare
    }
    offset += w
  }  
  val imuxCGCfgWidth = imuxCGCfgWidthList.sum
  imuxFGCfgWidthList.zipWithIndex.foreach{ case (w, i) =>
    if (w != 0) {
      imuxsFG(i).config := io.config(w + offset - 1, offset)
    } else {
      imuxsFG(i).config := DontCare
    }
    offset += w
  }  
  val imuxFGCfgWidth = imuxFGCfgWidthList.sum

}

/** Operator Core, which can support different kind of operation
 * @param Witdh data width
 * @param op     operation
 * */

class Operator_Core(Width: Int, op: String) extends Module{
  //@yuan_fp: get the precision of the float-point number
  def WidthGetFloat(width: Int): (Int, Int) = {
    if (width == 16) {
      (5, 11)
    } else if (width == 32) {
      (8, 24)
    } else {
      (11, 53)
    }
  }

  val inTypes = {
    if (OpInfo.OPCMap.contains(op)) {
      val maxNumOperands = OpInfo.getOperandNum(op)
      if (maxNumOperands > 2) Seq(Width, Width, 1)
      else Seq(Width, Width)
    } else { //@hw_yuan: for unified unit
      Seq(Width, Width)
    }
  }.map(w => UInt(w.W))
  val outTypes = {
    if (OpInfo.OPCMap.contains(op)) {
      val maxNumRes = OpInfo.getResNum(op)
      if (maxNumRes > 1) Seq(Width, 1)
      else Seq(Width)
    } else { //@hw_yuan: for unified unit
      if(op == "ARITH" || op == "LOGIC" || op == "FCMP"){// ARITH operation has ULE/ULT ...; LOGIC operation has EQ/N;
        Seq(Width, 1)
      }else{// SHIFT doesn't have 1-bit output
        Seq(Width)
      }
    }
  }.map(w => UInt(w.W))
  val cfgDataWidth = {//@hw_yuan: currently, each unified execute unit has 3-bit configuration
    if (OpInfo.OPCMap.contains(op)) {
      0
    } else { //@hw_yuan: for unified unit
      3
    }
  }
  val io = IO(new Bundle {
    val config = Input(UInt(cfgDataWidth.W))
    val in = Input(MixedVec(inTypes))
    val out = Output(MixedVec(outTypes))
  })
  if(OpInfo.OPCMap.contains(op)){// normal operations
    val isFmul = {//@yuan_fp
      if(op == "FMUL"){
        true.B
      }else{
        false.B
      }
    }
    val op_func_map = OpInfo(Width).OpFuncs(io.in.toSeq, 0.U, false, isFmul)//@yuan: since the FDIV may be harmful for packing, there is no need to transimit the enable signal
    val res = op_func_map(op)
    io.out := res.toSeq
  }else if (op == "ARITH"){
    val arith = Module(new ARITH(Width))
//    println("io.in size: " + io.in.size + " arith.io.in size: " + arith.io.in.size)
    arith.io.in.zipWithIndex.foreach { case (in, j) =>
      in := io.in(j)
    }
    arith.io.config := io.config
    io.out.zipWithIndex.foreach{ case (out, j) =>
      out := arith.io.out(j)
    }
  }else if (op == "LOGIC"){
    val arith = Module(new LOGIC(Width))
    arith.io.in.zipWithIndex.foreach { case (in, j) =>
      in := io.in(j)
    }
    arith.io.config := io.config
    io.out.zipWithIndex.foreach { case (out, j) =>
      out := arith.io.out(j)
    }
  }else if(op == "SHIFT"){// SHIFT
    val arith = Module(new SHIFT(Width))
    arith.io.in.zipWithIndex.foreach { case (in, j) =>
      in := io.in(j)
    }
    arith.io.config := io.config
    io.out.zipWithIndex.foreach { case (out, j) =>
      out := arith.io.out(j)
    }
  } else if (op == "FADDSUB") { //
    val floatWidth = WidthGetFloat(Width)
    val fadd = Module(new FADD(floatWidth._1, floatWidth._2))
    fadd.io.a := io.in(0)
    fadd.io.b := io.in(1)
    fadd.io.rm := 0.U
    fadd.io.mode := io.config
    io.out.zipWithIndex.foreach { case (out, j) =>
      out := fadd.io.result
    }
  } else if (op == "FCMP") {
    val floatWidth = WidthGetFloat(Width)
    val fcmp = Module(new FCMP(floatWidth._1, floatWidth._2))
    fcmp.io.a := io.in(0)
    fcmp.io.b := io.in(1)
    fcmp.io.mode := io.config
    io.out.zipWithIndex.foreach { case (out, j) =>
      out := fcmp.io.result
    }
  } else if (op == "FEAS") { // SHIFT
    val floatWidth = WidthGetFloat(Width)
    val feas = Module(new FEAS(floatWidth._1, floatWidth._2))
    feas.io.a := io.in(0)
    feas.io.b := io.in(1)
    feas.io.mode := io.config
    io.out.zipWithIndex.foreach { case (out, j) =>
      out := feas.io.result
    }
  }
}


// object VerilogGen extends App {
//   (new chisel3.stage.ChiselStage).emitVerilog(new ALU(32, ListBuffer(OPC.ADD, OPC.SUB, OPC.ULT)),args)
// }
