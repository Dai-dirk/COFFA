package fgramemfp.dsa

import scala.collection.mutable
import scala.collection.mutable.ListBuffer
import chisel3.util._
import fgramemfp.common.MacroVar._
import fgramemfp.op._
import fgramemfp.spec.{GibSpec, GpeSpec, IobSpec}

// GPE parameters
class GpeParam(num_reg_rf_for_alu_ : Int = 1, num_reg_rf_for_lut_ : Int = 1,
               max_delay_cg_ : Int = 4, max_delay_fg_ : Int = 8,
               num_input_lut_ : Int = 3,
               gpe_mode_ : Int = 0,
               operations_ : ListBuffer[String] = ListBuffer("PASS", "ADD", "SUB"),
               from_dir_ : List[Int] =  List(NORTHWEST, NORTHEAST, SOUTHWEST, SOUTHEAST),
               to_dir_ : List[Int] = List(NORTHWEST, NORTHEAST, SOUTHWEST, SOUTHEAST)) {
  var num_reg_rf_for_alu: Int = num_reg_rf_for_alu_  // not heterogeneous parameter, constant as 1
  var num_reg_rf_for_lut  = num_reg_rf_for_lut_      // not heterogeneous parameter, constant as 1
  var max_delay_cg  = max_delay_cg_ // do not affect type decision
  var max_delay_fg  = max_delay_fg_ // do not affect type decision
  var num_input_lut = num_input_lut_
  var gpe_mode = gpe_mode_
  var operations = operations_
  // println("operations: " + operations)
  var from_dir = from_dir_ // not heterogeneous parameter, constant
  var to_dir = to_dir_     // not heterogeneous parameter, constant
  val numOperandCG = {
    if(gpe_mode == 0 || gpe_mode == 2){
      2
    }else{
      val isMix = !isPow2(operations.map(OpInfo.getOpTypeId(_)).reduce(_|_))
      if(isMix){
        4
      }else{
        2
      }
    }
  }
//  println("numOperandCG: " + numOperandCG)
  var num_input_per_cg = ListBuffer.fill(numOperandCG){from_dir.size} //@hw_yuan: original
  //@yuan: if one PE has more than one kind of operation, it can have 4 ports
//  var num_input_per_cg = {
//    val isMix = isPow2(operations.map(OpInfo.getOpType(_)).reduce(_|_))
//    if(isMix){
//      ListBuffer.fill(2){from_dir.size}
//    }else{
//      ListBuffer.fill(4){from_dir.size}
//    }
//  }
  var num_input_per_fg = {
//    val ops = operations.map{str => OPC.withName(str)}
//    val aluOperandNum = ops.map(op => OpInfo.getOperandNum(op)).max
//    val aluOperandNum = operations.map(OpInfo.getOperandNum(_)).max
    val aluOperandNum = operations.map(OpInfo.getALUOperandNum(_)).max // only when op = SEL/ADC/SBC... has 3 alu inputs
    //@yuan: fg input should consider the CACC/CIACC operations
    val numACCOperandFG = operations.map(OpInfo.getAccFGOperand(_)).max
    val numOperandFG = numACCOperandFG + aluOperandNum - 2 + num_input_lut // ALU+LUT
    ListBuffer.fill(numOperandFG){from_dir.size}
  }
  var num_output_cg = 1
  var num_output_fg = {
//    val ops = operations.map{str => OPC.withName(str)}
//    val aluResNum = ops.map(op => OpInfo.getResNum(op)).max
    val aluResNum = operations.map(OpInfo.getResNum(_)).max
    val aluOutFG = aluResNum - num_output_cg
    aluOutFG + {if (num_input_lut > 0) 1 else 0} // ALU+LUT
  }

  def == (gpe : GpeParam): Boolean = {
    operations == gpe.operations &&
    num_input_lut  == gpe.num_input_lut &&
    num_input_per_cg == gpe.num_input_per_cg &&  
    num_input_per_fg == gpe.num_input_per_fg &&   
    gpe_mode == gpe.gpe_mode
  }
  def update() : Unit ={
    num_input_per_cg = ListBuffer.fill(2){from_dir.size}
    num_input_per_fg = {
//      val ops = operations.map(OPC.withName(_))
//      val aluOperandNum = ops.map(OpInfo.getOperandNum(_)).max
      val aluOperandNum = operations.map(OpInfo.getOperandNum(_)).max
      val numOperandFG = aluOperandNum - 2 + num_input_lut // ALU+LUT
      ListBuffer.fill(numOperandFG){from_dir.size}
    }
    num_output_fg = {
//      val ops = operations.map{str => OPC.withName(str)}
//      val aluResNum = ops.map(op => OpInfo.getResNum(op)).max
      val aluResNum = operations.map(OpInfo.getResNum(_)).max
      val aluOutFG = aluResNum - num_output_cg
      aluOutFG + {if (num_input_lut > 0) 1 else 0} // ALU+LUT
    }
  }
}

object GpeParam {
  def apply(num_reg_rf_for_alu_ : Int = 1, num_reg_rf_for_lut_ : Int = 1,
            max_delay_cg_ : Int = 4, max_delay_fg_ : Int = 8,
            num_input_lut_ : Int = 3,
            gpe_mode_ : Int = 0,
            operations_ : ListBuffer[String] = ListBuffer( "PASS", "ADD", "SUB"),
            from_dir_ : List[Int] =  List(NORTHWEST, NORTHEAST, SOUTHWEST, SOUTHEAST),
            to_dir_ : List[Int] = List(NORTHWEST, NORTHEAST, SOUTHWEST, SOUTHEAST)) : GpeParam = {
    new GpeParam(num_reg_rf_for_alu_, num_reg_rf_for_lut_, max_delay_cg_, max_delay_fg_,
      num_input_lut_, gpe_mode_, operations_, from_dir_, to_dir_)
  }
}

// IOB parameters
class IobParam(mode : Int = FIFO_MODE,
               hasIOFG : Boolean = true,
               maxDelayCG : Int = 4,
               maxDelayFG : Int = 4) {
  var iob_mode = mode
  var has_io_fg = hasIOFG  // IF has fine-grained IO ports
  var max_delay_cg = maxDelayCG     // max delay cycles of the fine-grained SharedDelayPipe
  var max_delay_fg = maxDelayFG     // max delay cycles of the fine-grained SharedDelayPipe
  var num_input_per_cg = {
    val numOperandCG = {
      if(mode == SRAM_MODE || mode == TASK_COND_EXIT_MODE) 2 // address and data
      else 1  // data
    }
    ListBuffer.fill(numOperandCG){2}
  }
  var num_input_per_fg = {
    val hasFGIn = has_io_fg
    val hasTaskExit  = mode == TASK_COND_EXIT_MODE
    val numOperandFG = {
      if(hasFGIn && hasTaskExit) 2
      else if (hasFGIn || hasTaskExit) 1
      else 0
    }
    ListBuffer.fill(numOperandFG){2}
  }
  var num_output_cg = 1 // data
  var num_output_fg = {
    val hasFGOut = has_io_fg   // ((iob_mode == IFIFO_MODE) || (iob_mode == IOFIFO_MODE))
    if(hasFGOut) 1
    else 0
  }

  def == (iob : IobParam): Boolean = {
    iob_mode == iob.iob_mode &&
      has_io_fg  == iob.has_io_fg
  }

  def update() : Unit = {
    num_input_per_cg = {
      val numOperandCG = {
        if (mode == SRAM_MODE || mode == TASK_COND_EXIT_MODE) 2 // address and data
        else 1 // data
      }
      ListBuffer.fill(numOperandCG) {
        2
      }
    }
    num_input_per_fg = {
      val hasFGIn = has_io_fg
      val hasTaskExit = mode == TASK_COND_EXIT_MODE
      val numOperandFG = {
        if (hasFGIn && hasTaskExit) 2
        else if (hasFGIn || hasTaskExit) 1
        else 0
      }
      ListBuffer.fill(numOperandFG) {
        2
      }
    }
    num_output_cg =  1 // data
    num_output_fg = {
      val hasFGOut = has_io_fg //((iob_mode == OFIFO_MODE) || (iob_mode == IOFIFO_MODE))
      if (hasFGOut) 1
      else 0
    }
  }
}

object IobParam {
  def apply(mode : Int = FIFO_MODE,
            hasIOFG : Boolean = true,
            maxDelayCG : Int = 4,
            maxDelayFG : Int = 4) : IobParam = {
    new IobParam(mode, hasIOFG, maxDelayCG, maxDelayFG)
  }
}

// GIB parameters
class GibParam(dataWidth: Int = 32, numTrack : Int = 1, diagIOPinConnect : Boolean = true, fclist_ : List[Int] = List(2, 4, 4)){
  var data_width = dataWidth
  var num_track = numTrack
  var diag_iopin_connect = diagIOPinConnect
  var fclist = fclist_
  require(fclist.size == 3)
  var connect_flexibility = mutable.Map(
    ("num_itrack_per_ipin" -> fclist(0)),
    ("num_otrack_per_opin" -> fclist(1)),
    ("num_ipin_per_opin" -> fclist(2))
  )
  var num_iopin_list = mutable.Map[String, Int]()
  var track_directions : ListBuffer[Int] = ListBuffer()
  var track_reged = false

  def == (gib : GibParam):Boolean = {
    data_width == gib.data_width &&
    num_track == gib.num_track &&
    track_directions == gib.track_directions &&
    diag_iopin_connect == gib.diag_iopin_connect &&
    fclist == gib.fclist &&
    num_iopin_list == gib.num_iopin_list
  }

  def update() : Unit ={
    connect_flexibility = mutable.Map(
      ("num_itrack_per_ipin" -> fclist(0)),
      ("num_otrack_per_opin" -> fclist(1)),
      ("num_ipin_per_opin" -> fclist(2))
    )
  }
}

object GibParam {
  def apply (dataWidth: Int = 32, numTrack : Int = 1, diagIOPinConnect : Boolean = true , fclist_ : List[Int] = List(2, 4, 4)) : GibParam = {
    new GibParam(dataWidth, numTrack, diagIOPinConnect, fclist_)
  }
}


// FGRA Parameters
class FgraParam(attrs: mutable.Map[String, Any]){
  // ====== Global attributes =======//
  val rows = attrs("fgra_num_row").asInstanceOf[Int]     // PE number in a row
  val cols = attrs("fgra_num_colum").asInstanceOf[Int]   // PE number in a colum
  val dataWidth = attrs("fgra_data_width").asInstanceOf[Int] // data width in bit
  // cfgParams
  val cfgDataWidth = attrs("fgra_cfg_data_width").asInstanceOf[Int]
  val cfgAddrWidth = attrs("fgra_cfg_addr_width").asInstanceOf[Int]
  val cfgBlkOffset = attrs("fgra_cfg_blk_offset").asInstanceOf[Int]   // configuration offset bit of blocks

  // ====== GPE-Specific attributes =======//
  // number of registers in Regfile for ALU
  val numRegRF4ALU = attrs("fgra_gpe_num_reg_rf_for_alu").asInstanceOf[Int]
  // number of registers in Regfile for LUT
  val numRegRF4LUT = attrs("fgra_gpe_num_reg_rf_for_lut").asInstanceOf[Int]
  // which directions are the GPE input from
  val gpeInFromDir = attrs("fgra_gpe_in_from_dir").asInstanceOf[List[Int]]
  // which directions are the GPE output to
  val gpeOutToDir= attrs("fgra_gpe_out_to_dir").asInstanceOf[List[Int]]
  // parameters of GPEs in 2D
  val gpesSpec = attrs("fgra_gpes").asInstanceOf[ListBuffer[ListBuffer[GpeSpec]]]  
  val gpesParam = gpesSpec.map{ buf =>
    buf.map{ spec => GpeParam(numRegRF4ALU, numRegRF4LUT, spec.max_delay_cg, spec.max_delay_fg,
      spec.num_input_lut, spec.gpe_mode, spec.operations, gpeInFromDir, gpeOutToDir) }
  }
  val gpe_operations = gpesSpec.flatten.map(_.operations).reduce(_++_).distinct
  val lut_operations = ListBuffer[String]()
  val num_lut_in_max = gpesSpec.flatten.map(_.num_input_lut).max
  if(num_lut_in_max > 0){
    lut_operations ++= ListBuffer("LUT")
  }
  // different types of GPEs (as submodules)
  // the type of each GPE (as instance)
  val gpe_typemap : mutable.Map[Int, GpeParam] =  mutable.Map() // [type-id, GpeParam]
  val gpe_posmap : mutable.Map[Tuple2[Int,Int],Int] = mutable.Map() // [(x, y), type-id]

  // FG positions: row index
  val fgRows = attrs("fgra_gpe_fg_rows").asInstanceOf[List[Int]]
  // FG positions: column index
  val fgCols = attrs("fgra_gpe_fg_columns").asInstanceOf[List[Int]]

  //@yuan_xcore: logwidth and maximum segments
  val logWidth = attrs("logWidth").asInstanceOf[Int]
  val maxSeg = attrs("maxSeg").asInstanceOf[Int]

  // ====== Coarse-grained GIB-Specific attributes =======//
  val numTrackCG = attrs("fgra_gib_num_track_cg").asInstanceOf[Int]
  // trackRegedMode, 0: no reg; 1: half of GIBs reged; 2: all GIBs reged
  val trackRegedModeCG = attrs("fgra_gib_track_reged_mode_cg").asInstanceOf[Int]
  // parameters of GIBs in 2D
  val cggibsSpec = attrs("fgra_cg_gibs").asInstanceOf[ListBuffer[ListBuffer[GibSpec]]]
  val cggibsParam = cggibsSpec.map{ buf =>
    buf.map{ spec => GibParam(dataWidth, numTrackCG, spec.diag_iopin_connect, spec.fclist) }
  }
  // different types of GIBs (as submodules)
  // the type of each GIB (as instance)
  val gib_typemap : mutable.Map[Int, GibParam] =  mutable.Map()
  val cggib_posmap : mutable.Map[Tuple2[Int,Int],Int] = mutable.Map()

  // ====== Fine-grained GIB-Specific attributes =======//
  val numTrackFG = attrs("fgra_gib_num_track_fg").asInstanceOf[Int]
  // trackRegedMode, 0: no reg; 1: half of GIBs reged; 2: all GIBs reged
  val trackRegedModeFG = attrs("fgra_gib_track_reged_mode_fg").asInstanceOf[Int]
  // parameters of GIBs in 2D
  val fggibsSpec = attrs("fgra_fg_gibs").asInstanceOf[ListBuffer[ListBuffer[GibSpec]]]
  val fggibsParam = fggibsSpec.map{ buf =>
    buf.map{ spec => GibParam(1, numTrackFG, spec.diag_iopin_connect, spec.fclist) }
  }
  // different types of GIBs (as submodules)
  // the type of each GIB (as instance)
//  val fggib_typemap : mutable.Map[Int, GibParam] =  mutable.Map()
  val fggib_posmap : mutable.Map[Tuple2[Int,Int],Int] = mutable.Map()

  // ====== IOB-Specific attributes =======//
  val addrWidthSram = attrs("fgra_iob_sram_addr_width").asInstanceOf[Int] - log2Ceil(dataWidth/8) // address width (+1 every data width)
  val hasMaskSram = attrs("fgra_iob_sram_has_mask").asInstanceOf[Boolean]  // if has write data byte mask
  val addRegSram = attrs("fgra_iob_sram_add_reg").asInstanceOf[Boolean]    // if add reg, write/read latency is 1/2; otherwise, 0/1
  val iobNumSides = attrs("fgra_iob_num_sides").asInstanceOf[Int]      // now only support top/bottom side
  val iobMode = attrs("fgra_iob_mode").asInstanceOf[Int]
  val iobHasIOFG = attrs("fgra_iob_has_io_fg").asInstanceOf[Boolean]
  val agNestLevels = attrs("fgra_iob_ag_nest_levels").asInstanceOf[Int] // nested levels of the address generation

  val lgMaxStride = attrs("fgra_iob_lg_max_stride").asInstanceOf[Int] // log2(max address stride, n represent n*dataWidth bits)
  val lgMaxLat = attrs("fgra_iob_lg_max_lat").asInstanceOf[Int] // log2(max in/out latency)
  val lgMaxCycles = attrs("fgra_iob_lg_max_cycles").asInstanceOf[Int] // log2(max in/out cycles)
  val lgMaxII = attrs("fgra_iob_lg_max_ii").asInstanceOf[Int] // log2(max in/out Initialization Interval)

  // parameters of IOBs in 2D
  val iobsSpec = attrs("fgra_iobs").asInstanceOf[ListBuffer[ListBuffer[IobSpec]]]
  val iobsParam = iobsSpec.map{ buf =>
    buf.map{ spec => IobParam(spec.iob_mode, spec.has_io_fg, spec.max_delay_cg, spec.max_delay_fg) }
  }
  val iob_modes = iobsSpec.flatten.map(_.iob_mode).distinct
  val iob_operations = ListBuffer[String]()
//  if (iob_modes.contains(COND_LS_MODE)) {
//    iob_operations ++= ListBuffer("INPUT", "OUTPUT", "LOAD", "STORE", "CLOAD", "CSTORE")
//  } else
  if(iob_modes.contains(TASK_COND_EXIT_MODE)){
    iob_operations ++= ListBuffer("INPUT", "OUTPUT", "LOAD", "STORE", "TLOAD", "TSTORE")
    if (iobHasIOFG) {
      iob_operations ++= ListBuffer("CINPUT", "COUTPUT", "CLOAD", "CSTORE", "TCLOAD", "TCSTORE")
    }
  }else if (iob_modes.contains(SRAM_MODE)) {
    iob_operations ++= ListBuffer("INPUT", "OUTPUT", "LOAD", "STORE")
    if(iobHasIOFG){
      iob_operations ++= ListBuffer("CINPUT", "COUTPUT", "CLOAD", "CSTORE")
    }
  }else{
    iob_operations ++= ListBuffer("INPUT", "OUTPUT")
    if (iobHasIOFG) {
      iob_operations ++= ListBuffer("CINPUT", "COUTPUT")
    }
  }
  // set operation set and data width
  OpInfo.apply(dataWidth).apply(gpe_operations ++ iob_operations ++ lut_operations)
  if(lut_operations.size > 0){
    OpInfo.setLUTWidth("LUT", num_lut_in_max)
  }
  // different types of IOBs (as submodules)
  // the type of each IOB (as instance)
  val iob_typemap : mutable.Map[Int, IobParam] =  mutable.Map() // [type-id, IobParam]
  val iob_posmap : mutable.Map[Tuple2[Int,Int],Int] = mutable.Map() // [(x, y), type-id]

  // SRAM interface number
  val numSrams = iobNumSides * cols
  // val load_latency = {if(addRegSram) 2 else 1}
  val load_latency = {if(addRegSram) 2 else 1} 
  val store_latency = {if(addRegSram) 1 else 0}

  val iob_to_spad_banks = mutable.Map[Int, List[Int]]() // the scratchpad banks connected to each IOB
  // divide all the banks into n groups where the internal banks are coalesced
  // eg. {0, 1}, {2, 3}, {4, 5},...
  val coalesceBanksIOB = attrs("fgra_iob_sram_banks_coalesce").asInstanceOf[Int]
  // println("coalesceBanksIOB: " + coalesceBanksIOB)
  for(i <- 0 until numSrams by coalesceBanksIOB){
    val coalBanks = coalesceBanksIOB min (numSrams-i) // last group may have banks no more than coalesceBanks
    val range = (i until (i+coalBanks)).toList
    range.foreach{ x =>
      iob_to_spad_banks += x -> range
    }
  }
  val spad_bank_lg_size = attrs("spad_bank_lg_size").asInstanceOf[Int]
  val cfg_spad_lg_size = attrs("spad_cfg_lg_size").asInstanceOf[Int]
  val cfg_spad_data_width = attrs("spad_data_width").asInstanceOf[Int]


  // find different types of GPEs (as submodules) according to the GPE Parameter
  // get the type of each GPE (as instance)
  for( x <- 0 until gpesParam.size){
    for( y <- 0 until gpesParam.head.size){
      val gpe = gpesParam(x)(y)
      val res = gpe_typemap.find(ins => ins._2 == gpe)
      val type_id = {
        if(res.isDefined){ // find a type
          res.get._1
        }else{ // create a new type
          val new_type_id = gpe_typemap.size
          gpe_typemap += (new_type_id -> gpe)
          new_type_id
        }
      }
      gpe_posmap += ((x,y) -> type_id)
    }
  }

  // get the type of each IOB (as instance)
  for( x <- 0 until iobsParam.size){
    for( y <- 0 until iobsParam.head.size){
      val iob = iobsParam(x)(y)
      val res = iob_typemap.find(ins => ins._2 == iob)
      val type_id = {
        if(res != None){ // find a type
          res.get._1
        }else{ // create a new type
          val new_type_id = iob_typemap.size
          iob_typemap += (new_type_id -> iob)
          new_type_id
        }
      }
      iob_posmap += ((x,y) -> type_id)
    }
  }

  // find different types of GIBs (as submodules) according to the GIB Parameter
  // get the type of each GIB (as instance)
  // coarse-grained
  for(i <- 0 until cggibsParam.size){
    for(j <- 0 until cggibsParam.head.size){
      val gib = cggibsParam(i)(j)
      val num_iopin_list = mutable.Map[String, Int]()
      num_iopin_list += "ipin_nw" -> {
        if(i == 0 && j > 0)
          iobsParam(0)(j - 1).num_input_per_cg.size // operand number
        else if(i > 0 && j > 0) {
          if(gpesParam(i - 1)(j - 1).from_dir.contains(SOUTHEAST)) {
            gpesParam(i - 1)(j - 1).num_input_per_cg.size // operand number
          } else {
            0
          }
        }
        else 0
      }
      num_iopin_list += "opin_nw" -> {
        if(i == 0 && j > 0)
          iobsParam(0)(j - 1).num_output_cg
        else if(i > 0 && j > 0){
          if(gpesParam(i - 1)(j - 1).to_dir.contains(SOUTHEAST)){
            gpesParam(i - 1)(j - 1).num_output_cg
          }else {
            0
          }
        }
        else 0
      }
      num_iopin_list += "ipin_ne" -> {
        if(i == 0 && j < cols) {
          iobsParam(0)(j).num_input_per_cg.size
        } // operand number
        else if(i > 0 && j < cols) {
          if(gpesParam(i-1)(j).from_dir.contains(SOUTHWEST)) {
            gpesParam(i-1)(j).num_input_per_cg.size
          }else{
            0
          }
        }
        else 0
      }
      num_iopin_list += "opin_ne" -> {
        if(i == 0 && j < cols)
          iobsParam(0)(j).num_output_cg
        else if(i > 0 && j < cols) {
          if(gpesParam(i-1)(j).to_dir.contains(SOUTHWEST)){
            gpesParam(i-1)(j).num_output_cg
          }else{
            0
          }
        }
        else 0
      }
      num_iopin_list += "ipin_se" -> {
        if(i == rows && j < cols){
          if(iobNumSides > 1)
            iobsParam(1)(j).num_input_per_cg.size // operand number
          else
            0
        } else if(i < rows && j < cols) {
          if(gpesParam(i)(j).from_dir.contains(NORTHWEST)) {
            gpesParam(i)(j).num_input_per_cg.size
          }else{
            0
          }
        }
        else 0
      }
      num_iopin_list += "opin_se" -> {
        if(i == rows && j < cols){
          if(iobNumSides > 1)
            iobsParam(1)(j).num_output_cg
          else
            0
        } else if(i < rows && j < cols) {
          if(gpesParam(i)(j).to_dir.contains(NORTHWEST)){
            gpesParam(i)(j).num_output_cg
          }else {
            0
          }
        }
        else 0
      }
      num_iopin_list += "ipin_sw" -> {
        if(i == rows && j > 0) {
          if(iobNumSides > 1)
            iobsParam(1)(j-1).num_input_per_cg.size // operand number
          else
            0
        } else if(i < rows && j > 0) {
          if(gpesParam(i)(j-1).from_dir.contains(NORTHEAST)) {
            gpesParam(i)(j-1).num_input_per_cg.size
          }else{
            0
          }
        }
        else 0
      }
      num_iopin_list += "opin_sw" -> {
        if(i == rows && j > 0) {
          if(iobNumSides > 1)
            iobsParam(1)(j-1).num_output_cg
          else
            0
        } else if(i < rows && j > 0) {
          if(gpesParam(i)(j-1).to_dir.contains(NORTHEAST)){
            gpesParam(i)(j-1).num_output_cg
          }else{
            0
          }
        }
        else 0
      }
      gib.num_iopin_list = num_iopin_list
      // if there are register behind the GIB
      val reged = {
        if(trackRegedModeCG == 0) false
        else if(trackRegedModeCG == 2) true
        else (i%2 + j%2) == 1
      }
      gib.track_reged= reged
      // which side has tracks
      val trackdirbuf : ListBuffer[Int] = ListBuffer()
      if(j > 0) trackdirbuf.append( WEST ) // WEST
      if(i > 0) trackdirbuf.append( NORTH ) // NORTH
      if(j+1 < cggibsParam.head.size)  trackdirbuf.append( EAST ) // EAST
      if(i+1 < cggibsParam.size)  trackdirbuf.append( SOUTH )  // SOUTH
      gib.track_directions = trackdirbuf
      // find the type of each GIB
      val res = gib_typemap.find(ins => ins._2 == gib)
      val type_id = {
        if(res != None){ // find a type
          res.get._1
        }else{ // create a new type
          val new_type_id = gib_typemap.size
          gib_typemap += (new_type_id -> gib)
          new_type_id
        }
      }
      cggib_posmap += ((i, j) -> type_id)
    }
  }

  // find different types of GIBs (as submodules) according to the GIB Parameter
  // get the type of each GIB (as instance)
  // fine-grained
  val fg_rows = fgRows.size
  val fg_columns = fgCols.size
 println(fggibsParam.size + " " + fg_rows+1)
  assert(fggibsParam.size == fg_rows+1)
  assert(fggibsParam.head.size == fg_columns+1)
  println("fg_columns" + " " + fg_columns + "fg_rows: " + fg_rows)
  for(i <- 0 to fg_rows) {
    val i_t = { if(i == 0) -1 else fgRows(i-1) } // top row of GPE
    val i_b = { if(i == fg_rows) -1 else fgRows(i) } // bottom row of GPE
    for (j <- 0 to fg_columns) {      
      val j_l = { if(j == 0) -1 else fgCols(j-1) } // left column of GPE/IOB
      val j_r = { if(j == fg_columns) -1 else fgCols(j) } // right column of GPE/IOB
      val gib = fggibsParam(i)(j)
      val num_iopin_list = mutable.Map[String, Int]()
      num_iopin_list += "ipin_nw" -> {
        if (i == 0 && j > 0)
          iobsParam(0)(j_l).num_input_per_fg.size // operand number
        else if (i > 0 && j > 0) {
          if (gpesParam(i_t)(j_l).from_dir.contains(SOUTHEAST)) {
            gpesParam(i_t)(j_l).num_input_per_fg.size // operand number
          } else {
            0
          }
        }
        else 0
      }
      num_iopin_list += "opin_nw" -> {
        if (i == 0 && j > 0)
          iobsParam(0)(j_l).num_output_fg
        else if (i > 0 && j > 0) {
          if (gpesParam(i_t)(j_l).to_dir.contains(SOUTHEAST)) {
            gpesParam(i_t)(j_l).num_output_fg
          } else {
            0
          }
        }
        else 0
      }
      num_iopin_list += "ipin_ne" -> {
        if (i == 0 && j < fg_columns)
          iobsParam(0)(j_r).num_input_per_fg.size // operand number
        else if (i > 0 && j < fg_columns) {
          if (gpesParam(i_t)(j_r).from_dir.contains(SOUTHWEST)) {
            gpesParam(i_t)(j_r).num_input_per_fg.size
          } else {
            0
          }
        }
        else 0
      }
      num_iopin_list += "opin_ne" -> {
        if (i == 0 && j < fg_columns)
          iobsParam(0)(j_r).num_output_fg
        else if (i > 0 && j < fg_columns) {
          if (gpesParam(i_t)(j_r).to_dir.contains(SOUTHWEST)) {
            gpesParam(i_t)(j_r).num_output_fg
          } else {
            0
          }
        }
        else 0
      }
      num_iopin_list += "ipin_se" -> {
        if (i == fg_rows && j < fg_columns) {
          if(iobNumSides > 1)
            iobsParam(1)(j_r).num_input_per_fg.size // operand number
          else
            0
        } else if (i < fg_rows && j < fg_columns) {
          if (gpesParam(i_b)(j_r).from_dir.contains(NORTHWEST)) {
            gpesParam(i_b)(j_r).num_input_per_fg.size
          } else {
            0
          }
        }
        else 0
      }
      num_iopin_list += "opin_se" -> {
        if (i == fg_rows && j < fg_columns) {
          if(iobNumSides > 1)
            iobsParam(1)(j_r).num_output_fg
          else
            0
        } else if (i < fg_rows && j < fg_columns) {
          if (gpesParam(i_b)(j_r).to_dir.contains(NORTHWEST)) {
            gpesParam(i_b)(j_r).num_output_fg
          } else {
            0
          }
        }
        else 0
      }
      num_iopin_list += "ipin_sw" -> {
        if (i == fg_rows && j > 0) {
          if(iobNumSides > 1)
            iobsParam(1)(j_l).num_input_per_fg.size // operand number
          else
            0
        } else if (i < fg_rows && j > 0) {
          if (gpesParam(i_b)(j_l).from_dir.contains(NORTHEAST)) {
            gpesParam(i_b)(j_l).num_input_per_fg.size
          } else {
            0
          }
        }
        else 0
      }
      num_iopin_list += "opin_sw" -> {
        if (i == fg_rows && j > 0) {
          if(iobNumSides > 1)
            iobsParam(1)(j_l).num_output_fg
          else
            0
        } else if (i < fg_rows && j > 0) {
          if (gpesParam(i_b)(j_l).to_dir.contains(NORTHEAST)) {
            gpesParam(i_b)(j_l).num_output_fg
          } else {
            0
          }
        }
        else 0
      }
      gib.num_iopin_list = num_iopin_list
      // if there are register behind the GIB
      val reged = {
        if (trackRegedModeFG == 0) false
        else if (trackRegedModeFG == 2) true
        else (i % 2 + j % 2) == 1
      }
      gib.track_reged = reged
      // which side has tracks
      val trackdirbuf: ListBuffer[Int] = ListBuffer()
      if (j > 0) trackdirbuf.append( WEST ) // WEST
      if(i > 0) trackdirbuf.append( NORTH ) // NORTH
      if(j < fg_columns)  trackdirbuf.append( EAST ) // EAST
      if(i < fg_rows)  trackdirbuf.append( SOUTH )  // SOUTH
      gib.track_directions = trackdirbuf
      // find the type of each GIB
      val res = gib_typemap.find(ins => ins._2 == gib)
      val type_id = {
        if (res != None) { // find a type
          res.get._1
        } else { // create a new type
          val new_type_id = gib_typemap.size
          gib_typemap += (new_type_id -> gib)
          new_type_id
        }
      }
      fggib_posmap += ((i, j) -> type_id)
    }
  }
}

object FgraParam{
  def apply(attrs: mutable.Map[String, Any]) : FgraParam = {
    new FgraParam(attrs)
  }
}