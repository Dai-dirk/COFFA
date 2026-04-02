package fgramemfp.dsa

import chisel3._
import chisel3.util._

import scala.collection.mutable
import scala.collection.mutable.{ArrayBuffer, ListBuffer}
import fgramemfp.op._
import fgramemfp.ir._


/** GPE: Generic Processing Element
 * 
 * @param attrs     module attributes
 */ 
class GPE(attrs: mutable.Map[String, Any]) extends Module with IR {
  val width = attrs("data_width").asInstanceOf[Int]
  // cfgParams
  val cfgDataWidth = attrs("cfg_data_width").asInstanceOf[Int]
  val cfgAddrWidth = attrs("cfg_addr_width").asInstanceOf[Int]
  val cfgBlkIndex  = attrs("cfg_blk_index").asInstanceOf[Int]     // configuration index of this block, cfg_addr[width-1 : offset]
  val cfgBlkOffset = attrs("cfg_blk_offset").asInstanceOf[Int]   // configuration offset bit of blocks
  // number of registers in Regfile for ALU
  val numRegRF4ALU = attrs("num_reg_rf_for_alu").asInstanceOf[Int]
  // number of registers in Regfile for LUT
  val numRegRF4LUT = attrs("num_reg_rf_for_lut").asInstanceOf[Int]
  // supported operations
  val ops = attrs("operations").asInstanceOf[ListBuffer[String]]
  //@hw_yuan: gpe mode
  val gpe_mode = attrs("gpe_mode").asInstanceOf[Int]
//  val ops = opsStr.map(OPC.withName(_))
//  val op_info = OpInfo(width)
  val aluOperandNum = ops.map(OpInfo.getALUOperandNum(_)).max
  val aluResNum = ops.map(OpInfo.getResNum(_)).max
  // val aluCfgWidth = log2Ceil(OPC.numOPC) // ALU Config width
  // LUT input number, 0: no LUT, k: LUT-K
  val numInLut = attrs("num_input_lut").asInstanceOf[Int]
  // coarse-grained input/output number
  val numOperandCG = 2
  val numOutCG = 1
  // fine-grained input/output number
  val aluOperandFG = aluOperandNum - numOperandCG
//  println("op: " + ops)
//  println("aluOperandNum: " + aluOperandNum + " numOperandCG: " + numOperandCG + " aluOperandFG: " + aluOperandFG)
  val aluOutFG = aluResNum - numOutCG
//  println("aluOutFG: " + aluOutFG)
  val numACCOperandFG = ops.map(OpInfo.getAccFGOperand(_)).max
//  println("numACCOperandFG: " + numACCOperandFG)
  val noLUTOperandFG = aluOperandFG + numACCOperandFG // fine-grain inputs without connect to LUT (SEL/CACC/CIACC control signal)
  println("noLUTOperandFG: " + noLUTOperandFG)
  val numOperandFG = aluOperandFG + numInLut + numACCOperandFG // ALU+LUT+ACC_ctrl
  val numOutFG = aluOutFG + {if (numInLut > 0) 1 else 0} // ALU+LUT
  // number of inputs per coarse-grained internal input
  val numInPerCG = attrs("num_input_per_cg").asInstanceOf[ListBuffer[Int]]
  // number of inputs per fine-grained internal input
  val numInPerFG = attrs("num_input_per_fg").asInstanceOf[ListBuffer[Int]]
  val numInCG = numInPerCG.sum
  val numInFG = numInPerFG.sum
//  println("numInPerCG: " + numInPerCG + " numInCG: " + numInCG)
//  println("numInPerFG: " + numInPerFG + " numInFG: " + numInFG)
  // max delay cycles of the DelayPipe for coarse/fine-grained inputs
  val maxDelayCG = attrs("max_delay_cg").asInstanceOf[Int]
  val maxDelayFG = attrs("max_delay_fg").asInstanceOf[Int]


  val hasISel = ops.map(OpInfo.isISelection(_) < 0).reduce(_ | _) //@yuan: has initialization selection
  val hasAcc = ops.map(OpInfo.isAccumulative(_) > 0).reduce(_ | _) // has accumulate operations
  val hasNonAcc = ops.map(OpInfo.isAccumulative(_) <= 0).reduce(_ | _)// has no accumulate operations
  val hasCondIsel = ops.map{op => OpInfo.CondISelInfoMap.contains(op)}.reduce(_ | _)
  val hasCondAcc = ops.map { op =>
    OpInfo.CondAccOpInfoMap.contains(op) || OpInfo.CondInitAccOpInfoMap.contains(op)|| OpInfo.CondDualInitAccOpInfoMap.contains(op)
  }.reduce(_ | _) // has conditional (reset) accumulate operations
  val hasCondDualInitAcc = ops.map { op =>
    OpInfo.CondDualInitAccOpInfoMap.contains(op)
  }.reduce(_ | _)  
  val hasMAC = ops.map { op =>
    op == "MAC" || op == "CMAC"
  }.reduce(_ | _)  
  assert((hasMAC && hasAcc) || !hasMAC) //@yuan: make sure that having MAC operation must with ACC operation
  val extraALUInput = if(hasMAC) 1 else 0
  val useDMR = hasAcc || hasISel
  val useDualDMRInput = hasCondDualInitAcc || hasISel
//  println("hasCondDualInitAcc: " + hasCondDualInitAcc)
  // num of const input, for CDIACC, it may has 2 const inputs
  val numConst = {
    if(hasCondDualInitAcc) 2 else 1
  }
//  val numConst = 1
  
  val OpType = ops.map(OpInfo.getOpType(_)).distinct
  val UnifiedCfgWidth = {
    if (OpType.contains("ARITH") || OpType.contains("LOGIC")|| OpType.contains("SHIFT") || OpType.contains("FADDSUB")|| OpType.contains("FCMP") || OpType.contains("FEXE")) {//@yuan_fp
      3
    } else {
      0
    }
  }

  val lgMaxWI = {
    if (useDMR) attrs("lg_max_wi").asInstanceOf[Int] else 0
  } // log2(max writing Interval)
  val lgMaxLat = {
    if (useDMR) attrs("lg_max_lat").asInstanceOf[Int] else 0
  } // log2(max starting latency)
  val lgMaxCycles = {
    if (useDMR) attrs("lg_max_cycles").asInstanceOf[Int] else 0
  } // log2(max writing cycles)
  val lgMaxRepeats = {
    if (useDMR) attrs("lg_max_repeats").asInstanceOf[Int] else 0
  } // log2(max repeat number)

  apply("data_width", width)
  apply("num_input_cg", numInCG)
  apply("num_output_cg", numOutCG)
  apply("num_input_fg", numInFG)
  apply("num_output_fg", numOutFG)
  apply("num_operand_cg", numOperandCG)
  apply("num_operand_fg", numOperandFG)
  apply("num_input_lut", numInLut)
  apply("cfg_blk_index", cfgBlkIndex)
  apply("num_reg_rf_for_alu", numRegRF4ALU)
  apply("num_reg_rf_for_lut", numRegRF4LUT)
  apply("operations", ops)
  apply("max_delay_cg", maxDelayCG)
  apply("max_delay_fg", maxDelayFG)
  apply("gpe_mode", gpe_mode)

  val io = IO(new Bundle {
    val cfg_en = Input(Bool())
    val cfg_addr = Input(UInt(cfgAddrWidth.W))
    val cfg_data = Input(UInt(cfgDataWidth.W))
    val start = Input(Bool()) // pulse signal, should be valid before latency 0, namely -1
    val en = Input(Bool())
    val in_cg = Input(Vec(numInCG, UInt(width.W)))
    val out_cg = Output(Vec(numOutCG, UInt(width.W)))
    val in_fg = Input(Vec(numInFG, UInt(1.W)))
    val out_fg = Output(Vec(numOutFG, UInt(1.W)))
  })
  val aluOps = ops.map(OpInfo.getALUOp(_)).distinct
  val alu = Module(new ALU(width, aluOps))
  val widthRF4ALU = width + aluOutFG
  val rf4alu : RF = {if(aluOutFG > 0) Module(new RF(aluOutFG, 1, 1, 2)) else null}
//  val rf4alu = Module(if(aluOutFG > 0) new RF(aluOutFG, 1, 1, 2) else null)
  val dmr = Module(new DualModeReg(width, useDMR, useDualDMRInput, lgMaxWI, lgMaxLat, lgMaxCycles, lgMaxRepeats))
  val delayPipeCG = Module(new SharedDelayPipe(width, maxDelayCG, numOperandCG))
  val lut : LUT = { if(numInLut > 0) Module(new LUT(1, numInLut)) else null }
  val rf4lut : RF = { if(numInLut > 0) Module(new RF(1, numRegRF4LUT, 1, 2)) else null }
  val delayPipeFG : SharedDelayPipe = { if(numOperandFG > 0) Module(new SharedDelayPipe(1, maxDelayFG, numOperandFG)) else null }
//  val lut = Module(new LUT(1, numInLut))
//  val rf4lut = Module(new RF(1, numRegRF4LUT, 1, 2))
//  val delayPipeFG = Module(new SharedDelayPipe(1, maxDelayFG, numOperandFG))
  val const = Wire(UInt(width.W))
  val second_const = {if(hasCondDualInitAcc) Wire(UInt(width.W)) else null}
  assert(numInPerCG.size == numOperandCG)
  val imuxsCG = numInPerCG.map{ num => Module(new Muxn(width, num+numConst)).io } // const + input
  assert(numInPerFG.size == numOperandFG)
  val imuxsFG = numInPerFG.zipWithIndex.map{ case (num, i) =>
    val rf_out_num = {
      if(aluOutFG > 0 && numInLut > 0) 2
      else if(aluOutFG > 0 || numInLut > 0) 1
      else 0
    }
//    if(i < aluOperandFG+1)
//    if(i < noLUTOperandFG) println("num+2+rf_out_num: " + (num+2+rf_out_num) + " num: " + num + " i: " + i + " rf_out_num: " + rf_out_num)
//    if(i < noLUTOperandFG) Module(new Muxn(1, num+2+rf_out_num)).io // const-0 + const-1 + input + rf_alu_out + rf_lut_out
//    else Module(new Muxn(1, num+2+(rf_out_num - 1))).io // const-0 + const-1 + input + rf_alu_out
    if(i < noLUTOperandFG){
      if(numInLut > 0) Module(new Muxn(1, num+2+1)).io // const-0 + const-1 + input + rf_lut_out
      else Module(new Muxn(1, num+2)).io // const-0 + const-1 + input
    }else Module(new Muxn(1, num+2+(rf_out_num - 1))).io // const-0 + const-1 + input + rf_alu_out
  }
  val opcWidth = OpInfo.ALUOPCWidth
  val opc = Wire(UInt(opcWidth.W))

  // ======= sub_module attribute ========//
  // 1 : Constant
  // 2-n : sub-modules
  val sm_id: mutable.Map[String, Int] = mutable.Map(
    "Const" -> 1,
    "ALU" -> 2,
    "DMR" -> 3,
    "DelayPipeCG" -> 4,
    "MuxnCG" -> 5
  )

  // ======= sub_module instance attribute ========//
  // 0 : this module CG I/O
  // 1 : Constant
  // 2-n : sub-modules
  val smi_id: mutable.Map[String, List[Int]] = mutable.Map(
    "This" -> List(0),
    "Const" -> List(1),
    "ALU" -> List(2),
    "DMR" -> List(3),
    "DelayPipeCG" -> List(4),
    "MuxnCG" -> (5 until numOperandCG+5).toList
  )

  var offset = numOperandCG+5
  if (hasCondDualInitAcc) {
    sm_id += "Second_Const" -> (sm_id.size+1)
    smi_id += "Second_Const" -> List(offset)
    offset += 1
  }
  if(aluOutFG > 0){
    sm_id += "RF4ALU" -> (sm_id.size+1)
    smi_id += "RF4ALU" -> List(offset)
    offset += 1
  }
  if(numInLut > 0){
    sm_id += "LUT" -> (sm_id.size+1)
    sm_id += "RF4LUT" -> (sm_id.size+1)
    smi_id += "LUT" -> List(offset)
    smi_id += "RF4LUT" -> List(offset+1)
    offset += 2
  }
  if(numOperandFG > 0){
    sm_id += "Const0" -> (sm_id.size+1)
    sm_id += "Const1" -> (sm_id.size+1)
    sm_id += "DelayPipeFG" -> (sm_id.size+1)
    sm_id += "MuxnFG" -> (sm_id.size+1)
    smi_id += "Const0" -> List(offset)
    smi_id += "Const1" -> List(offset+1)
    smi_id += "DelayPipeFG" -> List(offset+2)
    smi_id += "MuxnFG" -> (offset+3 until numOperandFG+offset+3).toList
    offset += numOperandFG+3
  }
  val next_smi_id : Int = offset
  val sub_modules = sm_id.map{case (name, id) => Map(
    "id" -> id,
    "type" -> name
  )}
  apply("sub_modules", sub_modules)

  val instances = smi_id.map{ case (name, ids) =>
    ids.map{ id => Map(
      "id" -> id,
      "type" -> name,
      "module_id" -> {if(name == "This") 0 else sm_id(name)}
    )}
  }.flatten
  apply("instances", instances)

  // ======= connections attribute ========//
  // apply("connection_format", ("src_id", "src_type", "src_out_idx", "dst_id", "dst_type", "dst_in_idx", "bit_width"))
  // This:src_out_idx is the input index
  // This:dst_in_idx is the output index
  val connections = ListBuffer(
    (smi_id("ALU")(0), "ALU", 0, smi_id("DMR")(0), "DMR", 0, width),
    (smi_id("DMR")(0), "DMR", 0, smi_id("This")(0), "This", 0, width)
  )


  offset = 0
//  println("imuxsCG size: " + imuxsCG.size)
  for(i <- 0 until numOperandCG){
    val num = numInPerCG(i)
    imuxsCG(i).in.zipWithIndex.foreach{ case (in, j) =>
      if(j == 0) {
        in := const
        connections.append((smi_id("Const")(0), "Const", 0, smi_id("MuxnCG")(i), "MuxnCG", j, width))
      } else if(j == 1 && hasCondDualInitAcc){
        in := second_const
        connections.append((smi_id("Second_Const")(0), "Second_Const", 0, smi_id("MuxnCG")(i), "MuxnCG", j, width))
      }
      else{
//        println("CGinput: " + j )
        in := io.in_cg(offset+j-numConst)
        connections.append((smi_id("This")(0), "This", offset+j-numConst, smi_id("MuxnCG")(i), "MuxnCG", j, width))
      }
//      else {
//        in := rf4alu.io.out(1)(width-1, 0)
//        connections.append((smi_id("RF4ALU")(0), "RF4ALU", 1, smi_id("MuxnCG")(i), "MuxnCG", j, width))
//      }
    }
    delayPipeCG.io.in(i) := imuxsCG(i).out
//    alu.io.in(i) := delayPipeCG.io.out(i)
    connections.append((smi_id("MuxnCG")(i), "MuxnCG", 0, smi_id("DelayPipeCG")(0), "DelayPipeCG", i, width))
//    connections.append((smi_id("DelayPipeCG")(0), "DelayPipeCG", i, smi_id("ALU")(0), "ALU", i, width))
    offset += num
  }
  delayPipeCG.io.en := io.en
  dmr.io.en := io.en
  dmr.io.in(0) := Cat(alu.io.out.toSeq.reverse)(width-1, 0)
  io.out_cg(0) := dmr.io.out(0)
  if(useDualDMRInput){
    dmr.io.in(1) := delayPipeCG.io.out(1)
//    connections.append((smi_id("DelayPipeCG")(0), "DelayPipeCG", 1, smi_id("DMR")(0), "DMR", 1, width))
  }

  dmr.io.en := DontCare
  dmr.io.init := DontCare
  if (!(hasAcc || hasISel)) {
    dmr.io.start := DontCare
    dmr.io.mode := DontCare
  } else {
    dmr.io.start := io.start
    dmr.io.mode := OpInfo.getAccMode(opc)
    if (hasCondAcc || hasCondIsel) { // do not record connections
//      val dmrCtrlidx = {
//        if(aluOperandFG > 0)  aluOperandFG
//        else 0
//      }
      dmr.io.en := delayPipeFG.io.out(0)
      connections.append((smi_id("DelayPipeFG")(0), "DelayPipeFG", 0, smi_id("DMR")(0), "DMR", 1, 1))
      if (numACCOperandFG > 1) {
        dmr.io.init := delayPipeFG.io.out(1)
        connections.append((smi_id("DelayPipeFG")(0), "DelayPipeFG", 1, smi_id("DMR")(0), "DMR", 2, 1))
      }
    }
  }

  if (!(hasAcc || hasISel)) { // no accumulative operation, i.e. do not use DMR
    alu.io.in.zipWithIndex.foreach { case (in, i) =>
      if(i < numOperandCG) {
        in := delayPipeCG.io.out(i)
        connections.append((smi_id("DelayPipeCG")(0), "DelayPipeCG", i, smi_id("ALU")(0), "ALU", i, width))
      }
    }
  } else if (!hasNonAcc) { // no basic (non-acc) operation, only has acc operation      
      if(hasMAC){
        when(OpInfo.isMACop(opc)) { // mac operation
          alu.io.in(0) := delayPipeCG.io.out(0)
          alu.io.in(1) := delayPipeCG.io.out(1)
          alu.io.in(2) := dmr.io.out(1)
        }.otherwise{// normal acc operation
          alu.io.in(0) := delayPipeCG.io.out(0)
          alu.io.in(1) := dmr.io.out(1)
          alu.io.in(2) := DontCare
        }
        alu.io.in.zipWithIndex.foreach { case (in, i) =>
          if (i < numOperandCG) {
            connections.append((smi_id("DelayPipeCG")(0), "DelayPipeCG", i, smi_id("ALU")(0), "ALU", i, width))
          }
        }      
      }else{// no MAC operation, coarse-grained operands < 3
        alu.io.in(0) := delayPipeCG.io.out(0)
        alu.io.in(1) := dmr.io.out(1)
        //@yuan: should keep consistent with DFG?
        connections.append((smi_id("DelayPipeCG")(0), "DelayPipeCG", 0, smi_id("ALU")(0), "ALU", 0, width))
      }
  } else { // has both basic and acc operations    
    if(hasMAC){
      when(OpInfo.isnotAccOp(opc)) { // normal operation, using both the 2 operands
        alu.io.in(0) := delayPipeCG.io.out(0)
        alu.io.in(1) := delayPipeCG.io.out(1)
      }.otherwise{
        alu.io.in(0) := delayPipeCG.io.out(0)
        alu.io.in(1) := dmr.io.out(1)
      }
      when(OpInfo.isMACop(opc)){ // MAC operation, the third operand is used as the feedback from DMR
        alu.io.in(2) := dmr.io.out(1)
      }.otherwise{
        alu.io.in(2) := DontCare
      }
    }else {
      when(OpInfo.isnotAccOp(opc)) {
        //      println("test")
        alu.io.in(0) := delayPipeCG.io.out(0)
        alu.io.in(1) := delayPipeCG.io.out(1)
      }.otherwise {
        alu.io.in(0) := dmr.io.out(1)
        alu.io.in(1) := delayPipeCG.io.out(0)
      }
    }
    alu.io.in.zipWithIndex.foreach { case (in, i) =>
      if (i < numOperandCG) {
        connections.append((smi_id("DelayPipeCG")(0), "DelayPipeCG", i, smi_id("ALU")(0), "ALU", i, width))
      }
    }
    //    connections.append((smi_id("DMR")(0), "DMR", 1, smi_id("ALU")(0), "ALU", 0))
  }

  if (aluOutFG > 0) {
    rf4alu.io.en := io.en
    rf4alu.io.in(0) := Cat(alu.io.out.toSeq.reverse)(width - 1 + aluOutFG) // ALU fine-grained output to RF
    connections.append((smi_id("ALU")(0), "ALU", 1, smi_id("RF4ALU")(0), "RF4ALU", 0, 1))
  }

  if(numOperandFG > 0){
    offset = 0
    for(i <- 0 until numOperandFG){
      val num = numInPerFG(i)
      println("numOperandFG: " + numOperandFG + " num: " + num + " numInFG: " + numInFG)
      println("imuxsFG(i).in size: " + imuxsFG(i).in.size)
      imuxsFG(i).in.zipWithIndex.foreach { case (in, j) =>
        if (j < 2) { // constant 0/1
          in := j.U
          val name = "Const" + j.toString
          connections.append((smi_id(name)(0), name, 0, smi_id("MuxnFG")(i), "MuxnFG", j, 1))
        } else if (j < num + 2) {
          in := io.in_fg(offset + j - 2)
          connections.append((smi_id("This")(0), "This", offset + j - 2, smi_id("MuxnFG")(i), "MuxnFG", j, 1))
        } else if (j == num + 2) {
          if (aluOutFG > 0 && i >= noLUTOperandFG) {
            in := rf4alu.io.out(1)
//             println("~~~")
            connections.append((smi_id("RF4ALU")(0), "RF4ALU", 1, smi_id("MuxnFG")(i), "MuxnFG", j, 1))
          } else {
            in := rf4lut.io.out(1)
            connections.append((smi_id("RF4LUT")(0), "RF4LUT", 1, smi_id("MuxnFG")(i), "MuxnFG", j, 1))
          }
        } else {
          in := rf4lut.io.out(1)
//          println("***")
          connections.append((smi_id("RF4LUT")(0), "RF4LUT", 1, smi_id("MuxnFG")(i), "MuxnFG", j, 1))
        }
      }
//      println("imuxsFG(i).in size: " + imuxsFG(i).in.size)

      delayPipeFG.io.in(i) := imuxsFG(i).out
      connections.append((smi_id("MuxnFG")(i), "MuxnFG", 0, smi_id("DelayPipeFG")(0), "DelayPipeFG", i, 1))
      if(noLUTOperandFG > 0){
        if(i == numACCOperandFG && aluOperandFG > 0){ // ALU has fine-grained input, idx start from the idx connect to DMR
          alu.io.in(numOperandCG + extraALUInput) := delayPipeFG.io.out(i)
          connections.append((smi_id("DelayPipeFG")(0), "DelayPipeFG", i, smi_id("ALU")(0), "ALU", numOperandCG, 1))
        }else if( i >= noLUTOperandFG){ // connect to LUT
          lut.io.in(i-noLUTOperandFG) := delayPipeFG.io.out(i)
          connections.append((smi_id("DelayPipeFG")(0), "DelayPipeFG", i, smi_id("LUT")(0), "LUT", i-noLUTOperandFG, 1))
        }
      } else { // only LUT has fine-grained input
        lut.io.in(i) := delayPipeFG.io.out(i)
        connections.append((smi_id("DelayPipeFG")(0), "DelayPipeFG", i, smi_id("LUT")(0), "LUT", i, 1))
      }
//      if(aluOperandFG > 0){ // ALU has fine-grained input
//        if(i == 0){
//          alu.io.in(numOperandCG) := delayPipeFG.io.out(i)
//          connections.append((smi_id("DelayPipeFG")(0), "DelayPipeFG", i, smi_id("ALU")(0), "ALU", numOperandCG, 1))
//        }else{
//          lut.io.in(i-1) := delayPipeFG.io.out(i)
//          connections.append((smi_id("DelayPipeFG")(0), "DelayPipeFG", i, smi_id("LUT")(0), "LUT", i-1, 1))
//        }
//      }else{ // only LUT has fine-grained input
//        lut.io.in(i) := delayPipeFG.io.out(i)
//        connections.append((smi_id("DelayPipeFG")(0), "DelayPipeFG", i, smi_id("LUT")(0), "LUT", i, 1))
//      }
      offset += num
    }
    delayPipeFG.io.en := io.en
  }
  if(numInLut > 0){ // has LUT
    rf4lut.io.en := io.en
    rf4lut.io.in(0) := lut.io.out
    connections.append((smi_id("LUT")(0), "LUT", 0, smi_id("RF4LUT")(0), "RF4LUT", 0, 1))
    if(aluOutFG > 0) { // ALU has fine-grained output
      io.out_fg(0) := rf4alu.io.out(0)
      io.out_fg(1) := rf4lut.io.out(0)
      connections.append((smi_id("RF4ALU")(0), "RF4ALU", 0, smi_id("This")(0), "This", 0, 1))
      connections.append((smi_id("RF4LUT")(0), "RF4LUT", 0, smi_id("This")(0), "This", 1, 1))
    }else{
      io.out_fg(0) := rf4lut.io.out(0)
      connections.append((smi_id("RF4LUT")(0), "RF4LUT", 0, smi_id("This")(0), "This", 0, 1))
    }
  }else if(aluOutFG > 0){ // ALU has fine-grained output
    io.out_fg(0) := rf4alu.io.out(0)
    connections.append((smi_id("RF4ALU")(0), "RF4ALU", 0, smi_id("This")(0), "This", 0, 1))
  }

  apply("connections", connections.zipWithIndex.map{case (c, i) => i -> c}.toMap)

  // configuration memory
  val constCfgWidth = width // constant
  val constInitCfgWidth = {if(hasCondDualInitAcc) width else 0}
//  val constInitCfgWidth = 0
  val aluCfgWidth = alu.io.config.getWidth // ALU Config width
  val rf4aluCfgWidth = {if(aluOutFG > 0) rf4alu.io.config.getWidth else 0}  // RF for ALU
  val dmrCfgWidth = dmr.io.config.getWidth  // DMR
  val delayCGCfgWidth = delayPipeCG.io.config.getWidth // CG-DelayPipe Config width
  val imuxCGCfgWidthList = imuxsCG.map{ mux => mux.config.getWidth } // input CG-Muxes
  val imuxCGCfgWidth = imuxCGCfgWidthList.sum
  val lutCfgWidth = {if(numInLut > 0) lut.io.config.getWidth else 0}// lut Config width
  val rf4lutCfgWidth = {if(numInLut > 0) rf4lut.io.config.getWidth else 0}  // RF for LUT
  val delayFGCfgWidth = {if(numOperandFG > 0) delayPipeFG.io.config.getWidth  else 0}// FG-DelayPipe Config width
  val imuxFGCfgWidthList = imuxsFG.map{ mux => mux.config.getWidth } // input FG-Muxes
  val imuxFGCfgWidth = imuxFGCfgWidthList.sum
  val sumCfgWidth = constCfgWidth + constInitCfgWidth + opcWidth + UnifiedCfgWidth + rf4aluCfgWidth + dmrCfgWidth + delayCGCfgWidth + imuxCGCfgWidth +
                    lutCfgWidth + rf4lutCfgWidth + delayFGCfgWidth + imuxFGCfgWidth
 println("sumCfgWidth: " + sumCfgWidth)
//  println("dmrCfgWidth: " + dmrCfgWidth)
  val cfg = Module(new ConfigMem(sumCfgWidth, 1, cfgDataWidth))
  cfg.io.cfg_en := io.cfg_en && (cfgBlkIndex.U === io.cfg_addr(cfgAddrWidth-1, cfgBlkOffset))
  cfg.io.cfg_addr := io.cfg_addr(cfgBlkOffset-1, 0)
  cfg.io.cfg_data := io.cfg_data
//  println("cfg.cfgAddrWidth: " + cfg.cfgAddrWidth + " cfgBlkOffset: " + cfgBlkOffset)
  assert(cfg.cfgAddrWidth <= cfgBlkOffset)
  assert(cfgBlkIndex < (1 << (cfgAddrWidth-cfgBlkOffset)))

  // ======= configuration attribute ========//
  val configuration = mutable.Map( // id : type, high, low
    smi_id("This")(0) -> ("This", sumCfgWidth-1, 0),
    smi_id("Const")(0) -> ("Const", constCfgWidth-1, 0)
  )

  val cfgOut = Wire(UInt(sumCfgWidth.W))
  cfgOut := cfg.io.out(0)
  const := cfgOut(constCfgWidth-1, 0)
  offset = constCfgWidth
  if (hasCondDualInitAcc) {
    second_const := cfgOut(constInitCfgWidth+offset-1, offset)
    configuration += smi_id("Second_Const")(0) -> ("Second_Const", constInitCfgWidth+offset-1, offset)
  }
  offset += constInitCfgWidth
  if(aluCfgWidth != 0){    
    alu.io.config := cfgOut(aluCfgWidth+offset-1, offset)
    opc := cfgOut(offset+opcWidth+UnifiedCfgWidth-1, offset + UnifiedCfgWidth)
    configuration += smi_id("ALU")(0) -> ("ALU", opcWidth+UnifiedCfgWidth+offset-1, offset)
  } else {
    alu.io.config := DontCare
  }
  offset += opcWidth+UnifiedCfgWidth
  if(rf4aluCfgWidth != 0){
    rf4alu.io.config := cfgOut(rf4aluCfgWidth+offset-1, offset)
    configuration += smi_id("RF4ALU")(0) -> ("RF4ALU", rf4aluCfgWidth+offset-1, offset)
  } else if (rf4alu != null){
    rf4alu.io.config := DontCare
  }
  offset += rf4aluCfgWidth
  if(delayCGCfgWidth != 0){
    delayPipeCG.io.config := cfgOut(delayCGCfgWidth+offset-1, offset)
    configuration += smi_id("DelayPipeCG")(0) -> ("DelayPipeCG", delayCGCfgWidth+offset-1, offset)
  } else {
    delayPipeCG.io.config := DontCare
  }
  offset += delayCGCfgWidth
  imuxCGCfgWidthList.zipWithIndex.foreach{ case (w, i) =>
    if(w != 0){
      imuxsCG(i).config := cfgOut(w+offset-1, offset)
      configuration += smi_id("MuxnCG")(i) -> ("MuxnCG", w+offset-1, offset)
    } else {
      imuxsCG(i).config := DontCare
    }
    offset += w
  }
  if(lutCfgWidth != 0){
    lut.io.config := cfgOut(lutCfgWidth+offset-1, offset)
    configuration += smi_id("LUT")(0) -> ("LUT", lutCfgWidth+offset-1, offset)
  } else if(numInLut > 0){
    lut.io.config := DontCare
  }
  offset += lutCfgWidth
  if(rf4lutCfgWidth != 0){
    rf4lut.io.config := cfgOut(rf4lutCfgWidth+offset-1, offset)
    configuration += smi_id("RF4LUT")(0) -> ("RF4LUT", rf4lutCfgWidth+offset-1, offset)
  } else if(numInLut > 0){
    rf4lut.io.config := DontCare
  }
  offset += rf4lutCfgWidth
  if(delayFGCfgWidth != 0){
    delayPipeFG.io.config := cfgOut(delayFGCfgWidth+offset-1, offset)
    configuration += smi_id("DelayPipeFG")(0) -> ("DelayPipeFG", delayFGCfgWidth+offset-1, offset)
  } else if(numOperandFG > 0){
    delayPipeFG.io.config := DontCare
  }
  offset += delayFGCfgWidth
  imuxFGCfgWidthList.zipWithIndex.foreach{ case (w, i) =>
    if(w != 0){
      imuxsFG(i).config := cfgOut(w+offset-1, offset)
      configuration += smi_id("MuxnFG")(i) -> ("MuxnFG", w+offset-1, offset)
    } else {
      imuxsFG(i).config := DontCare
    }
    offset += w
  }
  if(dmrCfgWidth != 0){
//    println("dmrCfgWidth : " + dmrCfgWidth)
//    println("offset+dmrCfgWidth-1 : " + (offset+dmrCfgWidth-1))
//    println("offset : " + offset )
    dmr.io.config := cfgOut(offset+dmrCfgWidth-1, offset)
    // AffineCtrlReg Config Ids
    val acr_cfg_id: mutable.Map[String, Int] = mutable.Map()
    dmr.cfg_idx.foreach { case (key, (idx, high, low)) =>
      configuration += (next_smi_id + idx) -> (key, high + offset, low + offset)
      acr_cfg_id += key -> (next_smi_id + idx)
    }
    apply("affine_ctrl_reg_cfg_id", acr_cfg_id)
  }else{
    dmr.io.config := DontCare
  }
  offset+= dmrCfgWidth
  apply("configuration", configuration)
//  println("one PE~~~~~~~~~~")
  // val outFilename = "test_ru n_dir/my_cgra_test.json"
  // printIR(outFilename)
}






// object petest extends App {
//   val attrs: mutable.Map[String, Any] = mutable.Map(
//     "data_width" -> 32,
//     "cfg_data_width" -> 32,
//     "cfg_addr_width" -> 12,
//     "cfg_blk_index" -> 1,
//     "cfg_blk_offset" -> 4,
//     "num_reg_rf_for_alu" -> 1,
//     "num_reg_rf_for_lut" -> 1,
//     "operations" -> ListBuffer("PASS", "ADD", "SUB", "SEL", "ULE"),
//     "num_input_lut" -> 3,
//     "num_input_per_cg" -> ListBuffer(4, 4),
//     "num_input_per_fg" -> ListBuffer(4, 4, 4, 4),
//     "max_delay_cg" -> 4,
//     "max_delay_fg" -> 8
//   )
//
//   (new chisel3.stage.ChiselStage).emitVerilog(new GPE(attrs),args)
////   ppa.ppa_gpe.getgpearea(ListBuffer("ADD"),ListBuffer(4, 4),4)
// }