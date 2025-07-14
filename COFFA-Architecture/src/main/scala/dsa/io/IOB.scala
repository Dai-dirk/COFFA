package fgramem.dsa

import chisel3._
import chisel3.util._
import scala.collection.mutable
import scala.collection.mutable.ListBuffer
import fgramem.ir._
import fgramem.common.MacroVar._


/** IO Block
 *
 * @param attrs     module attributes
 */
class IOB(attrs: mutable.Map[String, Any]) extends Module with IR {
  //  apply(attrs)
  val width = attrs("data_width").asInstanceOf[Int]
  val addrWidthSram = attrs("addr_width_sram").asInstanceOf[Int]  // address width (+1 every data width)
  //  println("addrWidthSram: " + addrWidthSram)
  val hasMaskSram = attrs("has_mask_sram").asInstanceOf[Boolean]  // if has write data byte mask
  val addRegSram = attrs("add_reg_sram").asInstanceOf[Boolean]    // if add reg, write/read latency is 1/2; otherwise, 0/1
  val agNestLevels = attrs("ag_nest_levels").asInstanceOf[Int] // nested levels of the address generation
  val lgMaxPartition = attrs("lg_max_partition").asInstanceOf[Int] // log2( the number of coalesce banks), which is used to indicate the start of the partitioned memory
  // 0: IFIFO mode (with dout), 1: OFIFO mode(with din), 2: IOFIFO mode (with din, dout), 3: SRAM mode (with addr, din, dout), 4: Conditional LS mode ((with en, addr, din, dout))
  val mode = attrs("iob_mode").asInstanceOf[Int]
  val hasIOFG = attrs("has_io_fg").asInstanceOf[Boolean]          // if there has fine-grained io, it means that the IOB can support conditional operations
  val lgMaxStride = attrs("lg_max_stride").asInstanceOf[Int]      // log2(max address stride, n represent n*dataWidth bits)
  val lgMaxLat = attrs("lg_max_lat").asInstanceOf[Int]            // log2(max in/out latency)
  val lgMaxCycles = attrs("lg_max_cycles").asInstanceOf[Int]      // log2(max in/out cycles)
  val lgMaxII = attrs("lg_max_ii").asInstanceOf[Int]              // log2(max in/out Initialization Interval)
  // cfgParams
  val cfgDataWidth = attrs("cfg_data_width").asInstanceOf[Int]
  val cfgAddrWidth = attrs("cfg_addr_width").asInstanceOf[Int]
  val cfgBlkIndex  = attrs("cfg_blk_index").asInstanceOf[Int]     // configuration index of this block, cfg_addr[width-1 : offset]
  val cfgBlkOffset = attrs("cfg_blk_offset").asInstanceOf[Int]    // configuration offset bit of blocks
  // I/O
  // coarse-grained input/output number
  val hasTaskExit = mode == TASK_COND_EXIT_MODE
  val numOperandCG = {
    if(mode == SRAM_MODE || hasTaskExit) 2 // address and data
    else 1  // data
  }
  val numOutCG = 1  // data
  // fine-grained input/output number
  val hasFGIn = hasIOFG
  val hasFGOut = hasIOFG  // ((mode == IFIFO_MODE) || (mode == IOFIFO_MODE))
  val numOperandFG = {
    if(hasFGIn && hasTaskExit) 2
    else if(hasFGIn || hasTaskExit) 1
    else 0
  }
  val numOutFG = {if(hasFGOut) 1 else 0}
  // number of inputs per coarse-grained internal input
  val numInPerCG = attrs("num_input_per_cg").asInstanceOf[ListBuffer[Int]]
  // number of inputs per fine-grained internal input
  val numInPerFG = attrs("num_input_per_fg").asInstanceOf[ListBuffer[Int]]
  val numInCG = numInPerCG.sum
  val numInFG = numInPerFG.sum
  assert(numOperandCG == numInPerCG.size)
  assert(numOperandFG == numInPerFG.size)
  // max delay cycles of the DelayPipe for coarse/fine-grained inputs
  val maxDelayCG = attrs("max_delay_cg").asInstanceOf[Int]
  val maxDelayFG = attrs("max_delay_fg").asInstanceOf[Int]

  apply("data_width", width)
  apply("num_input_cg", numInCG)
  apply("num_output_cg", numOutCG)
  apply("num_input_fg", numInFG)
  apply("num_output_fg", numOutFG)
  apply("num_operand_cg", numOperandCG)
  apply("num_operand_fg", numOperandFG)
  apply("max_delay_cg", maxDelayCG)
  apply("max_delay_fg", maxDelayFG)
  apply("cfg_blk_index", cfgBlkIndex)
  apply("iob_mode", mode)
  apply("ag_nest_levels", agNestLevels)
  //  println("ag_nest_levels: " + agNestLevels)

  val io = IO(new Bundle {
    val cfg_en   = Input(Bool())
    val cfg_addr = Input(UInt(cfgAddrWidth.W))
    val cfg_data = Input(UInt(cfgDataWidth.W))
    val start = Input(Bool()) // pulse signal, should be valid before latency 0, namely -1
    val done = Output(Bool()) // transfer done, keep true until next start
    val en = Input(Bool())
    val in_cg = Input(Vec(numInCG, UInt(width.W)))
    val out_cg = Output(Vec(numOutCG, UInt(width.W)))
    val in_fg = Input(Vec(numInFG, UInt(1.W)))
    val out_fg = Output(Vec(numOutFG, UInt(1.W)))
    val sram = Flipped(new SRAMIO(width, addrWidthSram, hasMaskSram))
  })

  val hasDelayPipe = (numOperandCG + numOperandFG) > 1 // if more than one input, add delayPipe
  val ioCtrl = Module(new IOController(width, addrWidthSram, hasMaskSram, mode, hasIOFG,
    lgMaxStride, lgMaxLat, lgMaxCycles, lgMaxII, agNestLevels, addRegSram, lgMaxPartition))
  val delayPipeCG : SharedDelayPipe = {
    if (hasDelayPipe && (numOperandCG > 0)) Module(new SharedDelayPipe(width, maxDelayCG, numOperandCG))
    else null
  }
  val delayPipeFG : SharedDelayPipe = {
    if(hasDelayPipe && (numOperandFG > 0)) Module(new SharedDelayPipe(1, maxDelayFG, numOperandFG))
    else null
  }
  val imuxsCG = numInPerCG.map{ num => Module(new Muxn(width, num)).io } // input MUXs
  val imuxsFG = numInPerFG.map{ num => Module(new Muxn(1, num)).io } // input MUXs

  // ======= sub_module attribute ========//
  // 1 : IOController
  // 2-n : other modules
  val sm_id: mutable.Map[String, Int] = mutable.Map(
    "IOController" -> 1
  )

  // ======= sub_module instance attribute ========//
  // 0 : this module
  // 1-n : sub-modules
  val smi_id: mutable.Map[String, List[Int]] = mutable.Map(
    "This" -> List(0),
    "IOController" -> List(1)
  )

  var sm_offset = 2
  var smi_offset = 2
  if(hasDelayPipe) {
    if(numOperandCG > 0){
      sm_id += ("DelayPipeCG" -> sm_offset)
      sm_offset += 1
      smi_id += ("DelayPipeCG" -> List(smi_offset))
      smi_offset += 1
    }
    if(numOperandFG > 0){
      sm_id += ("DelayPipeFG" -> sm_offset)
      sm_offset += 1
      smi_id += ("DelayPipeFG" -> List(smi_offset))
      smi_offset += 1
    }
  }
  if(numOperandCG > 0){
    sm_id += "MuxnCG" -> sm_offset
    sm_offset += 1
    smi_id += "MuxnCG" -> (smi_offset until numOperandCG+smi_offset).toList
    smi_offset += numOperandCG
  }
  if(numOperandFG > 0){
    sm_id += "MuxnFG" -> sm_offset
    sm_offset += 1
    smi_id += "MuxnFG" -> (smi_offset until numOperandFG+smi_offset).toList
    smi_offset += numOperandFG
  }
  val next_smi_id = smi_offset

  val sub_modules = sm_id.map{case (name, id) => Map(
    "id" -> id,
    "type" -> name
  )}
  apply("sub_modules", sub_modules)

  val instances = smi_id.map{case (name, ids) =>
    ids.map{id => Map(
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
  val connections = ListBuffer[(Int, String, Int, Int, String, Int, Int)]()

  ioCtrl.io.sram <> io.sram
  ioCtrl.io.start := io.start
  io.done := ioCtrl.io.done
  io.out_cg.zipWithIndex.foreach{ case (out, i) =>
    out := ioCtrl.io.out_cg(i)
    connections.append((smi_id("IOController")(0), "IOController", i, smi_id("This")(0), "This", i, width))
  }
  io.out_fg.zipWithIndex.foreach{ case (out, i) =>
    out := ioCtrl.io.out_fg(i)
    connections.append((smi_id("IOController")(0), "IOController", i, smi_id("This")(0), "This", i, 1))
  }

  if (hasDelayPipe && (numOperandCG > 0)){
    delayPipeCG.io.en := io.en
  }
  if (hasDelayPipe && (numOperandFG > 0)){
    delayPipeFG.io.en := io.en
  }

  var offset = 0
  numInPerCG.zipWithIndex.foreach { case (num, i) =>
    imuxsCG(i).in.zipWithIndex.foreach { case (in, j) =>
      in := io.in_cg(offset+j)
      connections.append((smi_id("This")(0), "This", offset+j, smi_id("MuxnCG")(i), "MuxnCG", j, width))
    }
    if(hasDelayPipe){
      delayPipeCG.io.in(i) := imuxsCG(i).out
      ioCtrl.io.in_cg(i) := delayPipeCG.io.out(i)
      connections.append((smi_id("MuxnCG")(i), "MuxnCG", 0, smi_id("DelayPipeCG")(0), "DelayPipeCG", i, width))
      connections.append((smi_id("DelayPipeCG")(0), "DelayPipeCG", i, smi_id("IOController")(0), "IOController", i, width))
    }else{
      ioCtrl.io.in_cg(i) := imuxsCG(i).out
      connections.append((smi_id("MuxnCG")(i), "MuxnCG", 0, smi_id("IOController")(0), "IOController", i, width))
    }
    offset += num
  }

  offset = 0
  numInPerFG.zipWithIndex.foreach { case (num, i) =>
    imuxsFG(i).in.zipWithIndex.foreach { case (in, j) =>
      in := io.in_fg(offset+j)
      connections.append((smi_id("This")(0), "This", offset+j, smi_id("MuxnFG")(i), "MuxnFG", j, 1))
    }
    if(hasDelayPipe){
      delayPipeFG.io.in(i) := imuxsFG(i).out
      ioCtrl.io.in_fg(i) := delayPipeFG.io.out(i)
      connections.append((smi_id("MuxnFG")(i), "MuxnFG", 0, smi_id("DelayPipeFG")(0), "DelayPipeFG", i, 1))
      connections.append((smi_id("DelayPipeFG")(0), "DelayPipeFG", i, smi_id("IOController")(0), "IOController", i, 1))
    }else{
      ioCtrl.io.in_fg(i) := imuxsFG(i).out
      connections.append((smi_id("MuxnFG")(i), "MuxnFG", 0, smi_id("IOController")(0), "IOController", i, 1))
    }
    offset += num
  }

  apply("connections", connections.zipWithIndex.map{case (c, i) => i -> c}.toMap)

  // configuration memory
  val ioCtrlCfgWidth = ioCtrl.io.config.getWidth
  val delayCGCfgWidth = { // CG-DelayPipe Config width
    if(hasDelayPipe && (numOperandCG > 0)) delayPipeCG.io.config.getWidth
    else 0
  }
  val delayFGCfgWidth = { // FG-DelayPipe Config width
    if(hasDelayPipe && (numOperandFG > 0)) delayPipeFG.io.config.getWidth
    else 0
  }
  val imuxCGCfgWidthList = imuxsCG.map{ mux => mux.config.getWidth } // input CG-Muxes
  val imuxCGCfgWidth = imuxCGCfgWidthList.sum
  val imuxFGCfgWidthList = imuxsFG.map{ mux => mux.config.getWidth } // input FG-Muxes
  val imuxFGCfgWidth = imuxFGCfgWidthList.sum
  val sumCfgWidth = ioCtrlCfgWidth + delayCGCfgWidth + delayFGCfgWidth + imuxCGCfgWidth + imuxFGCfgWidth

  val cfg = Module(new ConfigMem(sumCfgWidth, 1, cfgDataWidth))
  cfg.io.cfg_en := io.cfg_en && (cfgBlkIndex.U === io.cfg_addr(cfgAddrWidth-1, cfgBlkOffset))
  cfg.io.cfg_addr := io.cfg_addr(cfgBlkOffset-1, 0)
  cfg.io.cfg_data := io.cfg_data
  assert(cfg.cfgAddrWidth <= cfgBlkOffset)
  assert(cfgBlkIndex < (1 << (cfgAddrWidth-cfgBlkOffset)))
  val cfgOut = cfg.io.out(0)

  // ======= configuration attribute ========//
  val configuration = mutable.Map( // id : type, high, low
    smi_id("This")(0) -> ("This", sumCfgWidth-1, 0)
  )
  // IO Controller config IDs
  val ioc_cfg_id: mutable.Map[String, Int] = mutable.Map()
  ioCtrl.io.config := cfgOut(ioCtrlCfgWidth-1, 0)
  //  configuration += smi_id("IOController")(0) -> ("IOController", ioCtrlCfgWidth-1, 0)
  offset = ioCtrlCfgWidth
  ioCtrl.ioc_cfg_idx.foreach{ case (key, (offset, high, low)) =>
    configuration += (next_smi_id+offset) -> (key, high, low)
    ioc_cfg_id += key -> (next_smi_id+offset)
    //    println("next_smi_id: " + next_smi_id + " offset: " + offset)
  }
  apply("io_controller_cfg_id", ioc_cfg_id)

  if(delayCGCfgWidth != 0){
    delayPipeCG.io.config := cfgOut(delayCGCfgWidth+offset-1, offset)
    configuration += smi_id("DelayPipeCG")(0) -> ("DelayPipeCG", delayCGCfgWidth+offset-1, offset)
  } else {
    delayPipeCG.io.config := DontCare
  }
  offset += delayCGCfgWidth

  if(delayFGCfgWidth != 0){
    delayPipeFG.io.config := cfgOut(delayFGCfgWidth+offset-1, offset)
    configuration += smi_id("DelayPipeFG")(0) -> ("DelayPipeFG", delayFGCfgWidth+offset-1, offset)
  } else if(numOperandFG > 0){
    delayPipeFG.io.config := DontCare
  }
  offset += delayFGCfgWidth

  imuxCGCfgWidthList.zipWithIndex.foreach{ case (w, i) =>
    if(w != 0){
      imuxsCG(i).config := cfgOut(w+offset-1, offset)
      configuration += smi_id("MuxnCG")(i) -> ("MuxnCG", w+offset-1, offset)
    } else {
      imuxsCG(i).config := DontCare
    }
    offset += w
  }

  imuxFGCfgWidthList.zipWithIndex.foreach{ case (w, i) =>
    if(w != 0){
      imuxsFG(i).config := cfgOut(w+offset-1, offset)
      configuration += smi_id("MuxnFG")(i) -> ("MuxnFG", w+offset-1, offset)
    } else {
      imuxsFG(i).config := DontCare
    }
    offset += w
  }

  apply("configuration", configuration)
  //  apply("io_controller_cfg_format", ("[isIn, latency, baseAddr]")) // TODO: generate header file for software

}

