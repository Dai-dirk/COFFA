package fgramemfp.dsa

import chisel3._
import chisel3.util._
import fgramemfp.ir._
import fgramemfp.op._

import scala.collection.mutable
import scala.collection.mutable.ListBuffer


/** XCore: Non-linear operator unit
 *
 * @param attrs     module attributes
 */
class XCore(attrs: mutable.Map[String, Any]) extends Module with IR {
  val width = attrs("data_width").asInstanceOf[Int]
  //@yuan_hw: logwidth and maximum segments in LNS
  val logWidth = attrs("logWidth").asInstanceOf[Int]
  val segNum = attrs("maxSeg").asInstanceOf[Int]
//  assert(segNum == 6)//@hw_yuan: the default number of segment is 6 for current XCore implementation
  // println("logWidth: " + logWidth + " segNum: " + segNum)

  // cfgParams
  val cfgDataWidth = attrs("cfg_data_width").asInstanceOf[Int]
  val cfgAddrWidth = attrs("cfg_addr_width").asInstanceOf[Int]
  val cfgBlkIndex  = attrs("cfg_blk_index").asInstanceOf[Int]     // configuration index of this block, cfg_addr[width-1 : offset]
  val cfgBlkOffset = attrs("cfg_blk_offset").asInstanceOf[Int]   // configuration offset bit of blocks
 println("cfgAddrWidth: " + cfgAddrWidth + " cfgBlkIndex: " + cfgBlkIndex + " cfgBlkOffset: " + cfgBlkOffset)
  // supported operations
  val ops = attrs("operations").asInstanceOf[ListBuffer[String]]
  assert(!ops.contains("FDIV") && (!ops.contains("FMUL") || !ops.contains("MUL")))//@yuan_fp: currently, for MoPE, it can not support float-point divisor or the FMUL and MUL existing at the same time
  //@hw_yuan: gpe mode
  val gpe_mode = attrs("gpe_mode").asInstanceOf[Int]
  assert(gpe_mode == 2)//@hw_yuan: for xcore, the mode must be 2
//  val ops = opsStr.map(OPC.withName(_))
//  val op_info = OpInfo(width)
  val aluResNum = ops.map(OpInfo.getResNum(_)).max
  // val aluCfgWidth = log2Ceil(OPC.numOPC) // ALU Config width
  // coarse-grained input/output number
  val numOperandCG = attrs("numOperandCG").asInstanceOf[Int] // come from "FGRA Param"
  val numOutCG = 1
  val aluOperandNum = ops.map(OpInfo.getALUOperandNum(_)).max
//  println("aluOperandNum: " + aluOperandNum)
  // number of inputs per coarse-grained internal input
  val numInPerCG = attrs("num_input_per_cg").asInstanceOf[ListBuffer[Int]]
  val numInCG = numInPerCG.sum
  // println("numInPerCG: " + numInPerCG + " numInCG: " + numInCG)
  // println("numInPerFG: " + numInPerFG + " numInFG: " + numInFG)
  // max delay cycles of the DelayPipe for coarse/fine-grained inputs
  val maxDelayCG = attrs("max_delay_cg").asInstanceOf[Int]

  apply("data_width", width)
  apply("num_input_cg", numInCG)
  apply("num_output_cg", numOutCG)
  apply("num_operand_cg", numOperandCG)
  apply("num_input_fg", 0)
  apply("num_output_fg", 0)
  apply("num_operand_fg", 0)
  apply("cfg_blk_index", cfgBlkIndex)
  apply("operations", ops)
  apply("max_delay_cg", maxDelayCG)
  apply("max_delay_fg", 0)
  apply("gpe_mode", gpe_mode)

  val io = IO(new Bundle {
    val cfg_en = Input(Bool())
    val cfg_addr = Input(UInt(cfgAddrWidth.W))
    val cfg_data = Input(UInt(cfgDataWidth.W))
//    val start = Input(Bool()) // pulse signal, should be valid before latency 0, namely -1 @hw_yuan: maybe unified GPE doesn't need
    val en = Input(Bool())
    val in_cg = Input(Vec(numInCG, UInt(width.W)))
    val out_cg = Output(Vec(numOutCG, UInt(width.W)))
  })
  val lns = Module(new LNSTop(width, logWidth, segNum))
  val delayPipeCG = Module(new SharedDelayPipe(width, maxDelayCG, numOperandCG))
  val const = Wire(UInt(width.W))
  assert(numInPerCG.size == numOperandCG)
  val imuxsCG = numInPerCG.map{ num => Module(new Muxn(width, num+ 1)).io } // const + input
//  val opcWidth = OpInfo.ALUOPCWidth
//  val opc = Wire(UInt(opcWidth.W))

  // ======= sub_module attribute ========//
  // 1 : Constant
  // 2-n : sub-modules
  val sm_id: mutable.Map[String, Int] = mutable.Map(
    "Const" -> 1,
    "LNS" -> 2,
    "DelayPipeCG" -> 3,
    "MuxnCG" -> 4
  )

  // ======= sub_module instance attribute ========//
  // 0 : this module CG I/O
  // 1 : Constant
  // 2-n : sub-modules
  val smi_id: mutable.Map[String, List[Int]] = mutable.Map(
    "This" -> List(0),
    "Const" -> List(1),
    "LNS" -> List(2),
    "DelayPipeCG" -> List(3),
    "MuxnCG" -> (4 until numOperandCG+4).toList
  )

  var offset = numOperandCG+4
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
    (smi_id("LNS")(0), "LNS", 0, smi_id("This")(0), "This", 0, width)
//    (smi_id("DMR")(0), "DMR", 0, smi_id("This")(0), "This", 0, width)
  )


  offset = 0
//  println("imuxsCG size: " + imuxsCG.size)
  for(i <- 0 until numOperandCG){
    val num = numInPerCG(i)
    imuxsCG(i).in.zipWithIndex.foreach{ case (in, j) =>
      if(j == 0) {
        in := const
        connections.append((smi_id("Const")(0), "Const", 0, smi_id("MuxnCG")(i), "MuxnCG", j, width))
      }else {
//        println("CGinput: " + j )
        in := io.in_cg(offset+j-1)
        connections.append((smi_id("This")(0), "This", offset+j-1, smi_id("MuxnCG")(i), "MuxnCG", j, width))
      }
    }
    delayPipeCG.io.in(i) := imuxsCG(i).out
//    alu.io.in(i) := delayPipeCG.io.out(i)
    connections.append((smi_id("MuxnCG")(i), "MuxnCG", 0, smi_id("DelayPipeCG")(0), "DelayPipeCG", i, width))
//    connections.append((smi_id("DelayPipeCG")(0), "DelayPipeCG", i, smi_id("ALU")(0), "ALU", i, width))
    offset += num
  }
  delayPipeCG.io.en := io.en
//  dmr.io.en := io.en
//  dmr.io.in(0) := Cat(alu.io.out.toSeq.reverse)(width-1, 0)
  io.out_cg(0) := lns.io.out

//  dmr.io.en := DontCare

  lns.io.in.zipWithIndex.foreach { case (in, i) =>
    if (i < numOperandCG) {
      in := delayPipeCG.io.out(i)
      connections.append((smi_id("DelayPipeCG")(0), "DelayPipeCG", i, smi_id("LNS")(0), "LNS", i, width))
    }
  }

  apply("connections", connections.zipWithIndex.map{case (c, i) => i -> c}.toMap)

  // configuration memory
  val constCfgWidth = width // constant
//  val constInitCfgWidth = {if(hasCondDualInitAcc) width else 0}
//  val SecondConstCfgWidth =  {if(isMix) width else 0}
  val lnsCfgWidth = lns.io.config.getWidth // ALU Config width
  val rf4aluCfgWidth = 0  // RF for ALU
//  val dmrCfgWidth = dmr.io.config.getWidth  // DMR
  val delayCGCfgWidth = delayPipeCG.io.config.getWidth // CG-DelayPipe Config width
  val imuxCGCfgWidthList = imuxsCG.map{ mux => mux.config.getWidth } // input CG-Muxes
  val imuxCGCfgWidth = imuxCGCfgWidthList.sum
  val sumCfgWidth = constCfgWidth + lnsCfgWidth + rf4aluCfgWidth  + delayCGCfgWidth + imuxCGCfgWidth
//  println("GPE Unified: sumCfgWidth: " + sumCfgWidth + " aluCfgWidth: " + aluCfgWidth)
//  println("GPE Unified delayCGCfgWidth: " + delayCGCfgWidth)
  val cfg = Module(new ConfigMem(sumCfgWidth, 1, cfgDataWidth))
  cfg.io.cfg_en := io.cfg_en && (cfgBlkIndex.U === io.cfg_addr(cfgAddrWidth-1, cfgBlkOffset))
  cfg.io.cfg_addr := io.cfg_addr(cfgBlkOffset-1, 0)
  cfg.io.cfg_data := io.cfg_data
  println("cfg.cfgAddrWidth: " + cfg.cfgAddrWidth + " cfgBlkOffset: " + cfgBlkOffset)
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
//  if(rf4aluCfgWidth != 0){
//    rf4alu.io.config := cfgOut(rf4aluCfgWidth+offset-1, offset)
//    configuration += smi_id("RF4ALU")(0) -> ("RF4ALU", rf4aluCfgWidth+offset-1, offset)
//  } else if (rf4alu != null){
//    rf4alu.io.config := DontCare
//  }
//  offset += rf4aluCfgWidth
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
  if (lnsCfgWidth != 0) {
    lns.io.config := cfgOut(lnsCfgWidth + offset - 1, offset)
//    configuration += smi_id("ALU")(0) -> ("ALU", aluCfgWidth + offset - 1, offset)
    val lns_cfg_id: mutable.Map[String, Int] = mutable.Map()
    lns.cfg_idx.foreach { case (key, (idx, high, low)) =>
      configuration += (next_smi_id + idx) -> (key, high + offset, low + offset)
      lns_cfg_id += key -> (next_smi_id + idx)
    }
    apply("xcore_cfg_id_fg_flag", lns_cfg_id)
  } else {
    lns.io.config := DontCare
  }
  offset += lnsCfgWidth
//  println("dmrCfgWidth : " + dmrCfgWidth)
//  if(dmrCfgWidth != 0){
////    println("dmrCfgWidth : " + dmrCfgWidth)
////    println("offset+dmrCfgWidth-1 : " + (offset+dmrCfgWidth-1))
////    println("offset : " + offset )
//    dmr.io.config := cfgOut(offset+dmrCfgWidth-1, offset)
//    configuration += smi_id("DMR")(0) -> ("DMR", offset + dmrCfgWidth - 1, constCfgWidth + aluCfgWidth)
//  }else{
//    dmr.io.config := DontCare
//  }
//  offset+= dmrCfgWidth
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