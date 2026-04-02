package fgramemfp.dsa

import chisel3._
import chisel3.util._
import scala.collection.mutable
import scala.collection.mutable.{ArrayBuffer, ListBuffer}
import fgramemfp.op._
import fgramemfp.ir._
import fgramemfp.common.MacroVar._
import fgramemfp.spec.{GpeSpec, GibSpec}

/** FGRA Top module
 * 
 * @param attrs      module attributes
 * @param dumpIR     if dump IR file
 */
class FGRA(attrs: mutable.Map[String, Any]) extends Module with IR{
  // CGRA parameters
  val param = FgraParam(attrs)
  import param._

//  apply("top_module", "FGRA")
  apply("num_row", rows)
  apply("num_colum", cols)
  apply("data_width", dataWidth)
  apply("cfg_data_width", cfgDataWidth)
  apply("cfg_addr_width", cfgAddrWidth)
  apply("cfg_blk_offset", cfgBlkOffset)
  apply("num_srams", numSrams)
//  apply("load_latency", load_latency)
//  apply("store_latency", store_latency)
  apply("cfg_spad_size", (1 << cfg_spad_lg_size))
  apply("iob_spad_bank_size", (1 << spad_bank_lg_size))
  apply("iob_to_spad_banks", iob_to_spad_banks)
  apply("iob_ag_nest_levels", agNestLevels)
  apply("cfg_spad_data_width", cfg_spad_data_width)

  val io = IO(new Bundle{
    // config signals
    val cfg_en   = Input(Bool())
    val cfg_addr = Input(UInt(cfgAddrWidth.W))
    val cfg_data = Input(UInt(cfgDataWidth.W))
    val start = Input(Bool()) // pulse signal, should be valid before latency 0, namely -1
    val done = Output(Bool()) // transfer done, keep true until next start
    val iob_ens = Input(UInt(numSrams.W)) // enable signals for every IOB
    val en = Input(Bool()) // Input(Vec(cols, Bool()))
    val srams = Vec(numSrams, Flipped(new SRAMIO(dataWidth, addrWidthSram, hasMaskSram)))
  })

  val gpe_attrs: mutable.Map[String, Any] = mutable.Map(
    "data_width" -> dataWidth,
    "cfg_data_width" -> cfgDataWidth,
    "cfg_addr_width" -> cfgAddrWidth,
    "cfg_blk_index" -> 0,
    "cfg_blk_offset" -> cfgBlkOffset,
    "x" -> 0,
    "y" -> 0,
    "gpe_mode" -> 0,
    "numOperandCG" -> 2,
    "num_reg_rf_for_alu" -> 1,
    "num_reg_rf_for_lut" -> 1,
    "operations" -> ListBuffer(),
    "num_input_lut" -> 3,
    "num_input_per_cg" -> ListBuffer(),
    "num_input_per_fg" -> ListBuffer(),
    "max_delay_cg" -> 4,
    "max_delay_fg" -> 8,
    "logWidth" -> logWidth, //@yuan_hw: the polynomial coefficient
    "maxSeg" -> maxSeg, //@yuan_hw: the maximum number of segments
    "lg_max_ii" -> lgMaxII,
    "lg_max_lat" -> lgMaxLat,
    "lg_max_wi" -> lgMaxCycles,
    "lg_max_cycles" -> lgMaxCycles,
    "lg_max_repeats" -> lgMaxCycles
  )

  val gib_attrs: mutable.Map[String, Any] = mutable.Map(
    "data_width" -> dataWidth,
    "cfg_data_width" -> cfgDataWidth,
    "cfg_addr_width" -> cfgAddrWidth,
    "cfg_blk_index" -> 0,
    "cfg_blk_offset" -> cfgBlkOffset,
    "x" -> 0,
    "y" -> 0,
    "num_track" -> numTrackCG,
    "diag_iopin_connect" -> true,
    "num_iopin_list" -> mutable.Map[String, Int](),
    "connect_flexibility" -> Map(),
	  "track_reged" -> false,
    "track_directions" -> ListBuffer()
  )

  val iob_attrs: mutable.Map[String, Any] = mutable.Map(
    "data_width" -> dataWidth,
    "cfg_data_width" -> cfgDataWidth,
    "cfg_addr_width" -> cfgAddrWidth,
    "cfg_blk_index" -> 0,
    "cfg_blk_offset" -> cfgBlkOffset,
    "addr_width_sram" -> addrWidthSram,
    "has_mask_sram" -> hasMaskSram,
    "add_reg_sram" -> addRegSram,
    "iob_mode" -> iobMode,
    "has_io_fg" -> iobHasIOFG,
    "lg_max_stride" -> lgMaxStride,
    "lg_max_lat" -> lgMaxLat,
    "lg_max_cycles" -> lgMaxCycles,
    "lg_max_ii" -> lgMaxII,
    "ag_nest_levels" -> agNestLevels,
    "lg_max_partition" -> log2Ceil(coalesceBanksIOB), 
    "x" -> 0,
    "y" -> 0,
    "num_input_per_cg" -> ListBuffer(),
    "num_input_per_fg" -> ListBuffer(),
    "max_delay_cg" -> 4,
    "max_delay_fg" -> 4
  )

  // ======= sub_modules attribute ========//
  // 1-n : sub-modules 
  val sm_id: mutable.Map[String, ListBuffer[Int]] = mutable.Map(
    "IOB" -> ListBuffer[Int](),
    "GPE" -> ListBuffer[Int](), 
    "GIB" -> ListBuffer[Int]()  
  )

  // ======= sub_module instances attribute ========//
  // 0 : this module
  // 1-n : sub-module instances 
  val smi_id: mutable.Map[String, ListBuffer[Int]] = mutable.Map(
    "This" -> ListBuffer(0),
    "IOB" -> ListBuffer[Int](),  // id = cfg_blk_idx
    "GPE" -> ListBuffer[Int](), // id = cfg_blk_idx
    "CGGIB" -> ListBuffer[Int](), // id = cfg_blk_idx
    "FGGIB" -> ListBuffer[Int]()  // id = cfg_blk_idx
  )

  // sub-module id to attribute
  val sm_id_attrs = mutable.Map[Int, mutable.Map[String, Any]]()
  // sub-module instance id to attribute
  val smi_id_attrs = mutable.Map[Int, mutable.Map[String, Any]]()

  val iobs = new ArrayBuffer[IOB]()
  val pes = new ArrayBuffer[GPE]()  
  val unifiedpes = new ArrayBuffer[GPE_Unified]()
  val xcores = new ArrayBuffer[XCore]()
  val cggibs = new ArrayBuffer[GIB]()
  val fggibs = new ArrayBuffer[GIB]()

  val row_idxs_iob = ListBuffer[Int]()
  val row_idxs_pe = ListBuffer[Int]()
  val row_idxs_cggib = ListBuffer[Int]()
  val row_idxs_fggib = ListBuffer[Int]()

  row_idxs_iob += 0
  var idx = 1
  val fggibRows = (0 to fg_rows).map{ i =>
    if(i == 0)
      (1 + fgRows(i))/2
    else if(i == fg_rows)
      (fgRows(i-1) + rows + 1) / 2
    else
      (fgRows(i-1) + fgRows(i) + 1) / 2
  }
  for(i <- 0 to rows){
    row_idxs_cggib += idx
    idx += 1
    if(fggibRows.contains(i)){
      row_idxs_fggib += idx
      idx += 1
    }
    if(i < rows){
      row_idxs_pe += idx
      idx += 1
    }
  }
  if(iobNumSides > 1){
    row_idxs_iob += idx
    idx += 1
  }
  val totalRows = idx

  var sm_id_offset = 0
  val iob_type_modid : mutable.Map[Int , Int] = mutable.Map()
  // IOB : top and bottom row
  for(i <- 0 until iobNumSides) {
    val x = row_idxs_iob(i)
    val iob_index_base = x*(cols+1)
    for(j <- 0 until cols){
      val y = 2*j+1
      val index = iob_index_base+j+1
      iob_attrs("cfg_blk_index") = index
      iob_attrs("iob_index") = i*cols+j
      iob_attrs("x") = x
      iob_attrs("y") = y
      val iob_type = iob_posmap((i, j))
      val iob_param = iob_typemap(iob_type)
      iob_attrs("iob_mode") = iob_param.iob_mode
      iob_attrs("has_io_fg") = iob_param.has_io_fg
      iob_attrs("num_input_per_cg") = iob_param.num_input_per_cg
      iob_attrs("num_input_per_fg") = iob_param.num_input_per_fg
      iob_attrs("max_delay_cg") = iobsParam(i)(j).max_delay_cg  // do not affect type decision
      iob_attrs("max_delay_fg") = iobsParam(i)(j).max_delay_fg  // do not affect type decision
      iobs += Module(new IOB(iob_attrs))
//      area = area + ppa.ppa_iob.getiobarea(1,numOutIB)
      if(!iob_type_modid.contains(iob_type)){ // new IOB type
        sm_id_offset += 1
        iob_type_modid += (iob_type -> sm_id_offset)
        sm_id("IOB") += sm_id_offset
        sm_id_attrs += sm_id_offset -> iobs.last.getAttrs
      }
      val smi_id_attr: mutable.Map[String, Any] = mutable.Map(
        "module_id" -> iob_type_modid(iob_type),
        "cfg_blk_index" -> index,
        "iob_index" -> (i*cols+j),
        "max_delay_cg" -> iobsParam(i)(j).max_delay_cg,
        "max_delay_fg" -> iobsParam(i)(j).max_delay_fg,
        "x" -> x,
        "y" -> y
      )
      smi_id("IOB") += index
      smi_id_attrs += index -> smi_id_attr
    }
  }

  // GPE  
  val gpe_type_modid : mutable.Map[Int , Int] = mutable.Map()
  val gpe_index_map : mutable.Map[Int , Int] = mutable.Map()
  val unifiedgpe_index_map: mutable.Map[Int, Int] = mutable.Map()
  val xcore_index_map: mutable.Map[Int, Int] = mutable.Map()
  val gpe_smi_index_map: mutable.Map[Int, Int] = mutable.Map()
  for(i <- 0 until rows){
    val x = row_idxs_pe(i)
    for(j <- 0 until cols){
      val idx = i * cols + j // center
      val y = 2*j+1
      val index = x*(cols+1) + j + 1
      gpe_attrs("cfg_blk_index") = index
      gpe_attrs("x") = x
      gpe_attrs("y") = y
      val gpe_type = gpe_posmap((i, j))
      val gpe_param = gpe_typemap(gpe_type)
      gpe_attrs("num_reg_rf_for_alu") = gpe_param.num_reg_rf_for_alu // constant
      gpe_attrs("num_reg_rf_for_lut") = gpe_param.num_reg_rf_for_lut // constant
      gpe_attrs("operations") = gpe_param.operations
      gpe_attrs("num_input_lut") = gpe_param.num_input_lut
      gpe_attrs("num_input_per_cg") = gpe_param.num_input_per_cg
      gpe_attrs("num_input_per_fg") = gpe_param.num_input_per_fg
      gpe_attrs("numOperandCG") = gpe_param.numOperandCG
      val gpe_mode = gpe_param.gpe_mode
      gpe_attrs("gpe_mode") = gpe_mode // do not affect type decision
      gpe_attrs("max_delay_cg") = gpesParam(i)(j).max_delay_cg  // do not affect type decision
      gpe_attrs("max_delay_fg") = gpesParam(i)(j).max_delay_fg  // do not affect type decision
      if(gpe_mode == 0){//@hw_yuan: mode = 0, it is original GPE that supports accumulation
        pes += Module(new GPE(gpe_attrs))
        gpe_index_map += idx -> (pes.size - 1)
      }else if(gpe_mode == 1){
        unifiedpes += Module(new GPE_Unified(gpe_attrs))
        unifiedgpe_index_map += idx -> (unifiedpes.size - 1)
      }else {//@yuan_xcore:
        xcores += Module(new XCore(gpe_attrs))
        xcore_index_map  += idx ->  (xcores.size - 1)
      }
      gpe_smi_index_map += idx -> (pes.size + unifiedpes.size + xcores.size - 1)
//      println(gpe_attrs)
//      area = area +  ppa.ppa_gpe.getgpearea(GpeParam.operations,GpeParam.num_input_per_operand,GpeParam.max_delay)
      if(!gpe_type_modid.contains(gpe_type)){ // new GPE type
        sm_id_offset += 1
        gpe_type_modid += (gpe_type -> sm_id_offset)
        sm_id("GPE") += sm_id_offset
        if (gpe_mode == 0) { //@hw_yuan: mode = 0, it is original GPE that supports accumulation
          sm_id_attrs += sm_id_offset -> pes.last.getAttrs
        } else if (gpe_mode == 1) {
          sm_id_attrs += sm_id_offset -> unifiedpes.last.getAttrs
        } else{
          sm_id_attrs += sm_id_offset -> xcores.last.getAttrs
        }
      }
      val smi_id_attr: mutable.Map[String, Any] = mutable.Map(
        "module_id" -> gpe_type_modid(gpe_type),
        "cfg_blk_index" -> index,
        "max_delay_cg" -> gpesParam(i)(j).max_delay_cg,
        "max_delay_fg" -> gpesParam(i)(j).max_delay_fg,
        "x" -> x,
        "y" -> y
      )
      println("gpe_mode: " + gpe_mode + " index: " + index)
      smi_id("GPE") += index
      smi_id_attrs += index -> smi_id_attr
    }
  }  
  apply("num_MIMD_GPE", unifiedpes.size)

  // CGGIB
  val gib_type_modid : mutable.Map[Int , Int] = mutable.Map()
  for(i <- 0 to rows){
    for(j <- 0 to cols){
      val gib_type = cggib_posmap(i,j)
      val gib_param = gib_typemap(gib_type)
      val x = row_idxs_cggib(i)
      val y = 2*j
      val index = x*(cols+1) + j + 1
      gib_attrs("data_width") = dataWidth
      gib_attrs("num_track") = numTrackCG
      gib_attrs("cfg_blk_index") = index
      gib_attrs("x") = x
      gib_attrs("y") = y
      // if there are register behind the GIB
      val reged = cggibsParam(i)(j).track_reged // gib_param.track_reged //
      gib_attrs("track_reged") = reged
      gib_attrs("num_iopin_list") = gib_param.num_iopin_list
      gib_attrs("diag_iopin_connect") = gib_param.diag_iopin_connect
      gib_attrs("connect_flexibility") = gib_param.connect_flexibility
      gib_attrs("track_directions") = gib_param.track_directions
      cggibs += Module(new GIB(gib_attrs))
//      area = area +  ppa.ppa_gib.getgibarea(numTrack,GibParam.diag_iopin_connect,GibParam.num_iopin_list,GibParam.connect_flexibility,GibParam.track_reged,GibParam.trackDirections)
      if(!gib_type_modid.contains(gib_type)){ // new GIB type
        sm_id_offset += 1
        gib_type_modid += (gib_type -> sm_id_offset)
        sm_id("GIB") += sm_id_offset
        sm_id_attrs += sm_id_offset -> cggibs.last.getAttrs
      }
      val smi_id_attr: mutable.Map[String, Any] = mutable.Map(
        "module_id" -> gib_type_modid(gib_type),
        "cfg_blk_index" -> index,
        "x" -> x,
        "y" -> y,
		    "track_reged" -> reged // gib_param.track_reged //
      )
      smi_id("CGGIB") += index
      smi_id_attrs += index -> smi_id_attr
    }
  }

  // FGGIB: 2*(1,2,...,rows+1) row
  val fggibCols = (0 to fg_columns).map{ i =>
    if(i == 0)
      (1 + fgCols(i)) / 2
    else if(i == fg_columns)
      (fgCols(i-1) + cols + 1) / 2
    else
      (fgCols(i-1) + fgCols(i) + 1) / 2
  }
  for(ii <- 0 until fggibsParam.size) {
    for (jj <- 0 until fggibsParam.head.size) {
//      val i = fggibRows(ii)
//      val j = fggibColums(jj)
      val gib_type = fggib_posmap(ii,jj)
      val gib_param = gib_typemap(gib_type)
      val x = row_idxs_fggib(ii)
      val y = 2*fggibCols(jj)
      val index = x*(cols+1) + jj + 1
      gib_attrs("data_width") = 1
      gib_attrs("num_track") = numTrackFG
      gib_attrs("cfg_blk_index") = index
      gib_attrs("x") = x
      gib_attrs("y") = y
      // if there are register behind the GIB
      val reged = fggibsParam(ii)(jj).track_reged // gib_param.track_reged //
      gib_attrs("track_reged") = reged
      gib_attrs("num_iopin_list") = gib_param.num_iopin_list
      gib_attrs("diag_iopin_connect") = gib_param.diag_iopin_connect
      gib_attrs("connect_flexibility") = gib_param.connect_flexibility
      gib_attrs("track_directions") = gib_param.track_directions
      fggibs += Module(new GIB(gib_attrs))
      //      area = area +  ppa.ppa_gib.getgibarea(numTrack,GibParam.diag_iopin_connect,GibParam.num_iopin_list,GibParam.connect_flexibility,GibParam.track_reged,GibParam.trackDirections)
      if(!gib_type_modid.contains(gib_type)){ // new GIB type
        sm_id_offset += 1
        gib_type_modid += (gib_type -> sm_id_offset)
        sm_id("GIB") += sm_id_offset
        sm_id_attrs += sm_id_offset -> fggibs.last.getAttrs
      }
      val smi_id_attr: mutable.Map[String, Any] = mutable.Map(
        "module_id" -> gib_type_modid(gib_type),
        "cfg_blk_index" -> index,
        "x" -> x,
        "y" -> y,
        "track_reged" -> reged // gib_param.track_reged //
      )
      smi_id("FGGIB") += index
      smi_id_attrs += index -> smi_id_attr
    }
  }


  val sub_modules = sm_id.map{case (name, ids) =>
    ids.map{id => mutable.Map(
      "id" -> id, 
      "type" -> name,
      "attributes" -> sm_id_attrs(id)
    )}
  }.flatten
  apply("sub_modules", sub_modules)

  val instances = smi_id.map{case (name, ids) =>
    ids.map{id => mutable.Map(
      "id" -> id, 
      "type" -> name) ++ 
      {if(name != "This") smi_id_attrs(id) else mutable.Map[String, Any]()}
    }
  }.flatten
  apply("instances", instances)


  // ======= connections attribute ========//
  apply("connection_format", ("src_id", "src_type", "src_out_idx", "dst_id", "dst_type", "dst_in_idx", "bit_width"))
  // This:src_out_idx is the input index
  // This:dst_in_idx is the output index
  val connections = ListBuffer[(Int, String, Int, Int, String, Int, Int)]()
  val portNameMap = cggibs(0).portNameMap
  // IOB to CG-GIB connections
  iobs.zipWithIndex.foreach{ case (iob, i) =>
    iob.io.start := io.start && io.iob_ens(i)
    iob.io.en := io.en && io.iob_ens(i) // (i%cols)
    iob.io.sram <> io.srams(i)
    if (i < cols) { // top row
      val gibIdx = i
      iob.io.in_cg.zipWithIndex.foreach { case (in, j) =>
        if (j % 2 == 0) {
          in := cggibs(gibIdx).io.ipinNE(j / 2)
          val index = cggibs(gibIdx).oPortMap("ipinNE" + (j / 2).toString)
          connections.append((smi_id("CGGIB")(gibIdx), "CGGIB", index, smi_id("IOB")(i), "IOB", j, dataWidth))
        } else {
          in := cggibs(gibIdx + 1).io.ipinNW(j / 2)
          val index = cggibs(gibIdx + 1).oPortMap("ipinNW" + (j / 2).toString)
          connections.append((smi_id("CGGIB")(gibIdx + 1), "CGGIB", index, smi_id("IOB")(i), "IOB", j, dataWidth))
        }
      }
      iob.io.out_cg.zipWithIndex.foreach { case (out, j) =>
        cggibs(gibIdx).io.opinNE(j) := out
        cggibs(gibIdx + 1).io.opinNW(j) := out
        val index1 = cggibs(gibIdx).iPortMap("opinNE" + j.toString)
        val index2 = cggibs(gibIdx + 1).iPortMap("opinNW" + j.toString)
        connections.append((smi_id("IOB")(i), "IOB", j, smi_id("CGGIB")(gibIdx), "CGGIB", index1, dataWidth))
        connections.append((smi_id("IOB")(i), "IOB", j, smi_id("CGGIB")(gibIdx + 1), "CGGIB", index2, dataWidth))
      }
    } else { // bottom row
      val gibIdx = rows * (cols + 1) - cols + i
      iob.io.in_cg.zipWithIndex.foreach { case (in, j) =>
        if (j % 2 == 0) {
          in := cggibs(gibIdx).io.ipinSE(j / 2)
          val index = cggibs(gibIdx).oPortMap("ipinSE" + (j / 2).toString)
          connections.append((smi_id("CGGIB")(gibIdx), "CGGIB", index, smi_id("IOB")(i), "IOB", j, dataWidth))
        } else {
          in := cggibs(gibIdx + 1).io.ipinSW(j / 2)
          val index = cggibs(gibIdx + 1).oPortMap("ipinSW" + (j / 2).toString)
          connections.append((smi_id("CGGIB")(gibIdx + 1), "CGGIB", index, smi_id("IOB")(i), "IOB", j, dataWidth))
        }
      }
      iob.io.out_cg.zipWithIndex.foreach{ case (out, j) =>
        cggibs(gibIdx).io.opinSE(j) := out
        cggibs(gibIdx+1).io.opinSW(j) := out
        val index1 = cggibs(gibIdx).iPortMap("opinSE" + j.toString)
        val index2 = cggibs(gibIdx+1).iPortMap("opinSW" + j.toString)
        connections.append((smi_id("IOB")(i), "IOB", j, smi_id("CGGIB")(gibIdx), "CGGIB", index1, dataWidth))
        connections.append((smi_id("IOB")(i), "IOB", j, smi_id("CGGIB")(gibIdx+1), "CGGIB", index2, dataWidth))
      }
    }
  }

  val done = iobs.zipWithIndex.map{ case (iob, i) => iob.io.done || (!io.iob_ens(i).asBool) }.reduce(_ & _)
  io.done := RegNext(done)

  // IOB to FGGIB connections
  for(n <- 0 until iobNumSides) {
    if(n == 0) {
      for (i <- 0 until fg_columns) {
        val iobIdx = n * cols + fgCols(i)
        val gibIdx = i
        iobs(iobIdx).io.in_fg.zipWithIndex.foreach { case (in, j) =>
          if (j % 2 == 0) {
            in := fggibs(gibIdx).io.ipinNE(j / 2)
            val index = fggibs(gibIdx).oPortMap("ipinNE" + (j / 2).toString)
            connections.append((smi_id("FGGIB")(gibIdx), "FGGIB", index, smi_id("IOB")(iobIdx), "IOB", j, 1))
          } else {
            in := fggibs(gibIdx + 1).io.ipinNW(j / 2)
            val index = fggibs(gibIdx + 1).oPortMap("ipinNW" + (j / 2).toString)
            connections.append((smi_id("FGGIB")(gibIdx + 1), "FGGIB", index, smi_id("IOB")(iobIdx), "IOB", j, 1))
          }
        }
        iobs(iobIdx).io.out_fg.zipWithIndex.foreach { case (out, j) =>
          fggibs(gibIdx).io.opinNE(j) := out
          fggibs(gibIdx + 1).io.opinNW(j) := out
          val index1 = fggibs(gibIdx).iPortMap("opinNE" + j.toString)
          val index2 = fggibs(gibIdx + 1).iPortMap("opinNW" + j.toString)
          connections.append((smi_id("IOB")(iobIdx), "IOB", j, smi_id("FGGIB")(gibIdx), "FGGIB", index1, 1))
          connections.append((smi_id("IOB")(iobIdx), "IOB", j, smi_id("FGGIB")(gibIdx + 1), "FGGIB", index2, 1))
        }
      }
    }else{
      for (i <- 0 until fg_columns) {
        val iobIdx = n * cols + fgCols(i)
        val gibIdx = fg_rows * (fg_columns + 1) + i
        iobs(iobIdx).io.in_fg.zipWithIndex.foreach { case (in, j) =>
          if (j % 2 == 0) {
            in := fggibs(gibIdx).io.ipinSE(j / 2)
            val index = fggibs(gibIdx).oPortMap("ipinSE" + (j / 2).toString)
            connections.append((smi_id("FGGIB")(gibIdx), "FGGIB", index, smi_id("IOB")(iobIdx), "IOB", j, 1))
          } else {
            in := fggibs(gibIdx + 1).io.ipinSW(j / 2)
            val index = fggibs(gibIdx + 1).oPortMap("ipinSW" + (j / 2).toString)
            connections.append((smi_id("FGGIB")(gibIdx + 1), "FGGIB", index, smi_id("IOB")(iobIdx), "IOB", j, 1))
          }
        }
        iobs(iobIdx).io.out_fg.zipWithIndex.foreach { case (out, j) =>
          fggibs(gibIdx).io.opinSE(j) := out
          fggibs(gibIdx+1).io.opinSW(j) := out
          val index1 = fggibs(gibIdx).iPortMap("opinSE" + j.toString)
          val index2 = fggibs(gibIdx+1).iPortMap("opinSW" + j.toString)
          connections.append((smi_id("IOB")(iobIdx), "IOB", j, smi_id("FGGIB")(gibIdx), "FGGIB", index1, 1))
          connections.append((smi_id("IOB")(iobIdx), "IOB", j, smi_id("FGGIB")(gibIdx+1), "FGGIB", index2, 1))
        }
      }
    }
  }
  

  // PE to CGGIB connections
  for(i <- 0 until rows){
    for(j <- 0 until cols) {
      val idx = i * cols + j // center
      val idx_se = i * (cols + 1) + j // in GIB's perspective
      val idx_sw = i * (cols + 1) + j + 1
      val idx_ne = (i + 1) * (cols + 1) + j
      val idx_nw = (i + 1) * (cols + 1) + j + 1
      val gpe_param = gpe_typemap(gpe_posmap(i, j))
      val gpe_mode = gpe_param.gpe_mode
      if (gpe_mode == 0) {
        val idx_c = gpe_index_map(idx)
        val smi_id_index = gpe_smi_index_map(idx)
        pes(idx_c).io.en := io.en // (j)
        pes(idx_c).io.start := io.start
        val numinput = gpe_param.num_input_per_cg.size // operand number
        println("GPE Mode: " + gpe_mode + " idx_c: " + idx_c + " numinput: " + numinput + " io size: " + pes(idx_c).io.in_cg.size + " smi_id: " + smi_id("GPE")(smi_id_index))
//        println("GPE numinput_CG: " + numinput)
        // which directions of GIBs are connected to GPE input ports
        // number of inputs from each direction: numinput
        val from_dir = gpesParam(i)(j).from_dir
        if (from_dir.contains(NORTHWEST)) {
          val baseindex = from_dir.indexOf(NORTHWEST)
          for (k <- 0 until numinput) {
            val indexgpe = baseindex + k * from_dir.size // input order: inputs for 1st operand, inputs for 2nd operand...
            pes(idx_c).io.in_cg(indexgpe) := cggibs(idx_se).io.ipinSE(k)
            val indexgib = cggibs(idx_se).oPortMap("ipinSE" + (k).toString)
            connections.append((smi_id("CGGIB")(idx_se), "CGGIB", indexgib, smi_id("GPE")(smi_id_index), "GPE", indexgpe, dataWidth))
          }
        }
        if (from_dir.contains(NORTHEAST)) {
          val baseindex = from_dir.indexOf(NORTHEAST)
          for (k <- 0 until numinput) {
            val indexgpe = baseindex + k * from_dir.size
            pes(idx_c).io.in_cg(indexgpe) := cggibs(idx_sw).io.ipinSW(k)
            val indexgib = cggibs(idx_sw).oPortMap("ipinSW" + (k).toString)
            connections.append((smi_id("CGGIB")(idx_sw), "CGGIB", indexgib, smi_id("GPE")(smi_id_index), "GPE", indexgpe, dataWidth))
          }
        }

        if (from_dir.contains(SOUTHWEST)) {
          val baseindex = from_dir.indexOf(SOUTHWEST)
          for (k <- 0 until numinput) {
            val indexgpe = baseindex + k * from_dir.size
            pes(idx_c).io.in_cg(baseindex + k * from_dir.size) := cggibs(idx_ne).io.ipinNE(k)
            val indexgib = cggibs(idx_ne).oPortMap("ipinNE" + (k).toString)
            connections.append((smi_id("CGGIB")(idx_ne), "CGGIB", indexgib, smi_id("GPE")(smi_id_index), "GPE", indexgpe, dataWidth))
          }
        }
        if (from_dir.contains(SOUTHEAST)) {
          val baseindex = from_dir.indexOf(SOUTHEAST)
          for (k <- 0 until numinput) {
            val indexgpe = baseindex + k * from_dir.size
            pes(idx_c).io.in_cg(indexgpe) := cggibs(idx_nw).io.ipinNW(k)
            val indexgib = cggibs(idx_nw).oPortMap("ipinNW" + (k).toString)
            connections.append((smi_id("CGGIB")(idx_nw), "CGGIB", indexgib, smi_id("GPE")(smi_id_index), "GPE", indexgpe, dataWidth))
          }
        }

        // which directions of GIBs are connected to GPE output port
        val to_dir = gpesParam(i)(j).to_dir
        pes(idx_c).io.out_cg.zipWithIndex.foreach { case (out, k) =>
          if (to_dir.contains(NORTHWEST)) {
            cggibs(idx_se).io.opinSE(k) := out
            val index = cggibs(idx_se).iPortMap("opinSE" + k.toString)
            connections.append((smi_id("GPE")(smi_id_index), "GPE", k, smi_id("CGGIB")(idx_se), "CGGIB", index, dataWidth))
          }
          if (to_dir.contains(NORTHEAST)) {
            cggibs(idx_sw).io.opinSW(k) := out
            val index = cggibs(idx_sw).iPortMap("opinSW" + k.toString)
            connections.append((smi_id("GPE")(smi_id_index), "GPE", k, smi_id("CGGIB")(idx_sw), "CGGIB", index, dataWidth))
          }
          if (to_dir.contains(SOUTHWEST)) {
            cggibs(idx_ne).io.opinNE(k) := out
            val index = cggibs(idx_ne).iPortMap("opinNE" + k.toString)
            connections.append((smi_id("GPE")(smi_id_index), "GPE", k, smi_id("CGGIB")(idx_ne), "CGGIB", index, dataWidth))
          }
          if (to_dir.contains(SOUTHEAST)) {
            cggibs(idx_nw).io.opinNW(k) := out
            val index = cggibs(idx_nw).iPortMap("opinNW" + k.toString)
            connections.append((smi_id("GPE")(smi_id_index), "GPE", k, smi_id("CGGIB")(idx_nw), "CGGIB", index, dataWidth))
          }
        }
      } else if (gpe_mode == 1){
        val idx_c = unifiedgpe_index_map(idx)
        unifiedpes(idx_c).io.en := io.en // (j)
        val smi_id_index = gpe_smi_index_map(idx)
//        unifiedpes(idx_c).io.start := io.start
        val numinput = gpe_param.num_input_per_cg.size // operand number
        println("GPE Mode: " + gpe_mode + " idx_c: " + idx_c + " numinput: " + numinput + " io size: " + unifiedpes(idx_c).io.in_cg.size+ " smi_id: " + smi_id("GPE")(smi_id_index))
//        println("GPE numinput_CG: " + numinput)
        // which directions of GIBs are connected to GPE input ports
        // number of inputs from each direction: numinput
        val from_dir = gpesParam(i)(j).from_dir
        if (from_dir.contains(NORTHWEST)) {
          val baseindex = from_dir.indexOf(NORTHWEST)
          for (k <- 0 until numinput) {
            val indexgpe = baseindex + k * from_dir.size // input order: inputs for 1st operand, inputs for 2nd operand...
            unifiedpes(idx_c).io.in_cg(indexgpe) := cggibs(idx_se).io.ipinSE(k)
            val indexgib = cggibs(idx_se).oPortMap("ipinSE" + (k).toString)
            connections.append((smi_id("CGGIB")(idx_se), "CGGIB", indexgib, smi_id("GPE")(smi_id_index), "GPE", indexgpe, dataWidth))
          }
        }
        if (from_dir.contains(NORTHEAST)) {
          val baseindex = from_dir.indexOf(NORTHEAST)
          for (k <- 0 until numinput) {
            val indexgpe = baseindex + k * from_dir.size
            unifiedpes(idx_c).io.in_cg(indexgpe) := cggibs(idx_sw).io.ipinSW(k)
            val indexgib = cggibs(idx_sw).oPortMap("ipinSW" + (k).toString)
            connections.append((smi_id("CGGIB")(idx_sw), "CGGIB", indexgib, smi_id("GPE")(smi_id_index), "GPE", indexgpe, dataWidth))
          }
        }

        if (from_dir.contains(SOUTHWEST)) {
          val baseindex = from_dir.indexOf(SOUTHWEST)
          for (k <- 0 until numinput) {
            val indexgpe = baseindex + k * from_dir.size
            unifiedpes(idx_c).io.in_cg(baseindex + k * from_dir.size) := cggibs(idx_ne).io.ipinNE(k)
            val indexgib = cggibs(idx_ne).oPortMap("ipinNE" + (k).toString)
            connections.append((smi_id("CGGIB")(idx_ne), "CGGIB", indexgib, smi_id("GPE")(smi_id_index), "GPE", indexgpe, dataWidth))
          }
        }
        if (from_dir.contains(SOUTHEAST)) {
          val baseindex = from_dir.indexOf(SOUTHEAST)
          for (k <- 0 until numinput) {
            val indexgpe = baseindex + k * from_dir.size
            unifiedpes(idx_c).io.in_cg(indexgpe) := cggibs(idx_nw).io.ipinNW(k)
            val indexgib = cggibs(idx_nw).oPortMap("ipinNW" + (k).toString)
            connections.append((smi_id("CGGIB")(idx_nw), "CGGIB", indexgib, smi_id("GPE")(smi_id_index), "GPE", indexgpe, dataWidth))
          }
        }

        // which directions of GIBs are connected to GPE output port
        val to_dir = gpesParam(i)(j).to_dir
        unifiedpes(idx_c).io.out_cg.zipWithIndex.foreach { case (out, k) =>
          if (to_dir.contains(NORTHWEST)) {
            cggibs(idx_se).io.opinSE(k) := out
            val index = cggibs(idx_se).iPortMap("opinSE" + k.toString)
            connections.append((smi_id("GPE")(smi_id_index), "GPE", k, smi_id("CGGIB")(idx_se), "CGGIB", index, dataWidth))
          }
          if (to_dir.contains(NORTHEAST)) {
            cggibs(idx_sw).io.opinSW(k) := out
            val index = cggibs(idx_sw).iPortMap("opinSW" + k.toString)
            connections.append((smi_id("GPE")(smi_id_index), "GPE", k, smi_id("CGGIB")(idx_sw), "CGGIB", index, dataWidth))
          }
          if (to_dir.contains(SOUTHWEST)) {
            cggibs(idx_ne).io.opinNE(k) := out
            val index = cggibs(idx_ne).iPortMap("opinNE" + k.toString)
            connections.append((smi_id("GPE")(smi_id_index), "GPE", k, smi_id("CGGIB")(idx_ne), "CGGIB", index, dataWidth))
          }
          if (to_dir.contains(SOUTHEAST)) {
            cggibs(idx_nw).io.opinNW(k) := out
            val index = cggibs(idx_nw).iPortMap("opinNW" + k.toString)
            connections.append((smi_id("GPE")(smi_id_index), "GPE", k, smi_id("CGGIB")(idx_nw), "CGGIB", index, dataWidth))
          }
        }
      }else{ //@yuan_xcore: connect xcore to CGIBs
        val idx_c = xcore_index_map(idx)
        val smi_id_index = gpe_smi_index_map(idx)
        xcores(idx_c).io.en := io.en // (j)
        val numinput = gpe_param.num_input_per_cg.size // operand number
        // println("GPE Mode: " + gpe_mode + " idx_c: " + idx_c + " numinput: " + numinput + " io size: " + pes(idx_c).io.in_cg.size + " smi_id: " + smi_id("GPE")(smi_id_index))
        //        println("GPE numinput_CG: " + numinput)
        // which directions of GIBs are connected to GPE input ports
        // number of inputs from each direction: numinput
        val from_dir = gpesParam(i)(j).from_dir
        if (from_dir.contains(NORTHWEST)) {
          val baseindex = from_dir.indexOf(NORTHWEST)
          for (k <- 0 until numinput) {
            val indexgpe = baseindex + k * from_dir.size // input order: inputs for 1st operand, inputs for 2nd operand...
            xcores(idx_c).io.in_cg(indexgpe) := cggibs(idx_se).io.ipinSE(k)
            val indexgib = cggibs(idx_se).oPortMap("ipinSE" + (k).toString)
            connections.append((smi_id("CGGIB")(idx_se), "CGGIB", indexgib, smi_id("GPE")(smi_id_index), "GPE", indexgpe, dataWidth))
          }
        }
        if (from_dir.contains(NORTHEAST)) {
          val baseindex = from_dir.indexOf(NORTHEAST)
          for (k <- 0 until numinput) {
            val indexgpe = baseindex + k * from_dir.size
            xcores(idx_c).io.in_cg(indexgpe) := cggibs(idx_sw).io.ipinSW(k)
            val indexgib = cggibs(idx_sw).oPortMap("ipinSW" + (k).toString)
            connections.append((smi_id("CGGIB")(idx_sw), "CGGIB", indexgib, smi_id("GPE")(smi_id_index), "GPE", indexgpe, dataWidth))
          }
        }

        if (from_dir.contains(SOUTHWEST)) {
          val baseindex = from_dir.indexOf(SOUTHWEST)
          for (k <- 0 until numinput) {
            val indexgpe = baseindex + k * from_dir.size
            xcores(idx_c).io.in_cg(baseindex + k * from_dir.size) := cggibs(idx_ne).io.ipinNE(k)
            val indexgib = cggibs(idx_ne).oPortMap("ipinNE" + (k).toString)
            connections.append((smi_id("CGGIB")(idx_ne), "CGGIB", indexgib, smi_id("GPE")(smi_id_index), "GPE", indexgpe, dataWidth))
          }
        }
        if (from_dir.contains(SOUTHEAST)) {
          val baseindex = from_dir.indexOf(SOUTHEAST)
          for (k <- 0 until numinput) {
            val indexgpe = baseindex + k * from_dir.size
            xcores(idx_c).io.in_cg(indexgpe) := cggibs(idx_nw).io.ipinNW(k)
            val indexgib = cggibs(idx_nw).oPortMap("ipinNW" + (k).toString)
            connections.append((smi_id("CGGIB")(idx_nw), "CGGIB", indexgib, smi_id("GPE")(smi_id_index), "GPE", indexgpe, dataWidth))
          }
        }
        // which directions of GIBs are connected to GPE output port
        val to_dir = gpesParam(i)(j).to_dir
        xcores(idx_c).io.out_cg.zipWithIndex.foreach { case (out, k) =>
          if (to_dir.contains(NORTHWEST)) {
            cggibs(idx_se).io.opinSE(k) := out
            val index = cggibs(idx_se).iPortMap("opinSE" + k.toString)
            connections.append((smi_id("GPE")(smi_id_index), "GPE", k, smi_id("CGGIB")(idx_se), "CGGIB", index, dataWidth))
          }
          if (to_dir.contains(NORTHEAST)) {
            cggibs(idx_sw).io.opinSW(k) := out
            val index = cggibs(idx_sw).iPortMap("opinSW" + k.toString)
            connections.append((smi_id("GPE")(smi_id_index), "GPE", k, smi_id("CGGIB")(idx_sw), "CGGIB", index, dataWidth))
          }
          if (to_dir.contains(SOUTHWEST)) {
            cggibs(idx_ne).io.opinNE(k) := out
            val index = cggibs(idx_ne).iPortMap("opinNE" + k.toString)
            connections.append((smi_id("GPE")(smi_id_index), "GPE", k, smi_id("CGGIB")(idx_ne), "CGGIB", index, dataWidth))
          }
          if (to_dir.contains(SOUTHEAST)) {
            cggibs(idx_nw).io.opinNW(k) := out
            val index = cggibs(idx_nw).iPortMap("opinNW" + k.toString)
            connections.append((smi_id("GPE")(smi_id_index), "GPE", k, smi_id("CGGIB")(idx_nw), "CGGIB", index, dataWidth))
          }
        }
      }
    }
  }
// sys.exit(0)
  // PE to FGGIB connections
  //@yuan_xcore: for the xcore, there is no fine-grained connections
  for(i <- 0 until fg_rows){
    val x = fgRows(i)
    for(j <- 0 until fg_columns) {
      val y = fgCols(j)
      val idx = x * cols + y // center
      val idx_se = i * (fg_columns + 1) + j
      val idx_sw = i * (fg_columns + 1) + j + 1
      val idx_ne = (i + 1) * (fg_columns + 1) + j
      val idx_nw = (i + 1) * (fg_columns + 1) + j + 1
      val gpe_param = gpe_typemap(gpe_posmap(x, y))
      val gpe_mode = gpe_param.gpe_mode
      if (gpe_mode == 0) {
        val idx_c = gpe_index_map(idx)
        pes(idx_c).io.en := io.en //(j)
        val smi_id_index = gpe_smi_index_map(idx)
        val numinput = gpe_param.num_input_per_fg.size // operand number
        //      println("GPE numinput_FG: " + numinput)
        // which directions of GIBs are connected to GPE input ports
        // number of inputs from each direction: numinput
        val from_dir = gpesParam(x)(y).from_dir
        if (from_dir.contains(NORTHWEST)) {
          val baseindex = from_dir.indexOf(NORTHWEST)
          for (k <- 0 until numinput) {
            val indexgpe = baseindex + k * from_dir.size // input order: inputs for 1st operand, inputs for 2nd operand...
            pes(idx_c).io.in_fg(indexgpe) := fggibs(idx_se).io.ipinSE(k)
            val indexgib = fggibs(idx_se).oPortMap("ipinSE" + (k).toString)
            connections.append((smi_id("FGGIB")(idx_se), "FGGIB", indexgib, smi_id("GPE")(smi_id_index), "GPE", indexgpe, 1))
          }
        }
        if (from_dir.contains(NORTHEAST)) {
          val baseindex = from_dir.indexOf(NORTHEAST)
          for (k <- 0 until numinput) {
            val indexgpe = baseindex + k * from_dir.size
            pes(idx_c).io.in_fg(indexgpe) := fggibs(idx_sw).io.ipinSW(k)
            val indexgib = fggibs(idx_sw).oPortMap("ipinSW" + (k).toString)
            connections.append((smi_id("FGGIB")(idx_sw), "FGGIB", indexgib, smi_id("GPE")(smi_id_index), "GPE", indexgpe, 1))
          }
        }

        if (from_dir.contains(SOUTHWEST)) {
          val baseindex = from_dir.indexOf(SOUTHWEST)
          for (k <- 0 until numinput) {
            val indexgpe = baseindex + k * from_dir.size
            pes(idx_c).io.in_fg(baseindex + k * from_dir.size) := fggibs(idx_ne).io.ipinNE(k)
            val indexgib = fggibs(idx_ne).oPortMap("ipinNE" + (k).toString)
            connections.append((smi_id("FGGIB")(idx_ne), "FGGIB", indexgib, smi_id("GPE")(smi_id_index), "GPE", indexgpe, 1))
          }
        }
        if (from_dir.contains(SOUTHEAST)) {
          val baseindex = from_dir.indexOf(SOUTHEAST)
          for (k <- 0 until numinput) {
            val indexgpe = baseindex + k * from_dir.size
            pes(idx_c).io.in_fg(indexgpe) := fggibs(idx_nw).io.ipinNW(k)
            val indexgib = fggibs(idx_nw).oPortMap("ipinNW" + (k).toString)
            connections.append((smi_id("FGGIB")(idx_nw), "FGGIB", indexgib, smi_id("GPE")(smi_id_index), "GPE", indexgpe, 1))
          }
        }

        // which directions of GIBs are connected to GPE output port
        val to_dir = gpesParam(x)(y).to_dir
        pes(idx_c).io.out_fg.zipWithIndex.foreach { case (out, k) =>
          if (to_dir.contains(NORTHWEST)) {
            fggibs(idx_se).io.opinSE(k) := out
            val index = fggibs(idx_se).iPortMap("opinSE" + k.toString)
            connections.append((smi_id("GPE")(smi_id_index), "GPE", k, smi_id("FGGIB")(idx_se), "FGGIB", index, 1))
          }
          if (to_dir.contains(NORTHEAST)) {
            fggibs(idx_sw).io.opinSW(k) := out
            val index = fggibs(idx_sw).iPortMap("opinSW" + k.toString)
            connections.append((smi_id("GPE")(smi_id_index), "GPE", k, smi_id("FGGIB")(idx_sw), "FGGIB", index, 1))
          }
          if (to_dir.contains(SOUTHWEST)) {
            fggibs(idx_ne).io.opinNE(k) := out
            val index = fggibs(idx_ne).iPortMap("opinNE" + k.toString)
            connections.append((smi_id("GPE")(smi_id_index), "GPE", k, smi_id("FGGIB")(idx_ne), "FGGIB", index, 1))
          }
          if (to_dir.contains(SOUTHEAST)) {
            fggibs(idx_nw).io.opinNW(k) := out
            val index = fggibs(idx_nw).iPortMap("opinNW" + k.toString)
            connections.append((smi_id("GPE")(smi_id_index), "GPE", k, smi_id("FGGIB")(idx_nw), "FGGIB", index, 1))
          }
        }
      }else if(gpe_mode == 1) {
        val idx_c = unifiedgpe_index_map(idx)
        unifiedpes(idx_c).io.en := io.en //(j)
        val smi_id_index = gpe_smi_index_map(idx)
        val numinput = gpe_param.num_input_per_fg.size // operand number
        //      println("GPE numinput_FG: " + numinput)
        // which directions of GIBs are connected to GPE input ports
        // number of inputs from each direction: numinput
        val from_dir = gpesParam(x)(y).from_dir
        if (from_dir.contains(NORTHWEST)) {
          val baseindex = from_dir.indexOf(NORTHWEST)
          for (k <- 0 until numinput) {
            val indexgpe = baseindex + k * from_dir.size // input order: inputs for 1st operand, inputs for 2nd operand...
            unifiedpes(idx_c).io.in_fg(indexgpe) := fggibs(idx_se).io.ipinSE(k)
            val indexgib = fggibs(idx_se).oPortMap("ipinSE" + (k).toString)
            connections.append((smi_id("FGGIB")(idx_se), "FGGIB", indexgib, smi_id("GPE")(smi_id_index), "GPE", indexgpe, 1))
          }
        }
        if (from_dir.contains(NORTHEAST)) {
          val baseindex = from_dir.indexOf(NORTHEAST)
          for (k <- 0 until numinput) {
            val indexgpe = baseindex + k * from_dir.size
            unifiedpes(idx_c).io.in_fg(indexgpe) := fggibs(idx_sw).io.ipinSW(k)
            val indexgib = fggibs(idx_sw).oPortMap("ipinSW" + (k).toString)
            connections.append((smi_id("FGGIB")(idx_sw), "FGGIB", indexgib, smi_id("GPE")(smi_id_index), "GPE", indexgpe, 1))
          }
        }

        if (from_dir.contains(SOUTHWEST)) {
          val baseindex = from_dir.indexOf(SOUTHWEST)
          for (k <- 0 until numinput) {
            val indexgpe = baseindex + k * from_dir.size
            unifiedpes(idx_c).io.in_fg(baseindex + k * from_dir.size) := fggibs(idx_ne).io.ipinNE(k)
            val indexgib = fggibs(idx_ne).oPortMap("ipinNE" + (k).toString)
            connections.append((smi_id("FGGIB")(idx_ne), "FGGIB", indexgib, smi_id("GPE")(smi_id_index), "GPE", indexgpe, 1))
          }
        }
        if (from_dir.contains(SOUTHEAST)) {
          val baseindex = from_dir.indexOf(SOUTHEAST)
          for (k <- 0 until numinput) {
            val indexgpe = baseindex + k * from_dir.size
            unifiedpes(idx_c).io.in_fg(indexgpe) := fggibs(idx_nw).io.ipinNW(k)
            val indexgib = fggibs(idx_nw).oPortMap("ipinNW" + (k).toString)
            connections.append((smi_id("FGGIB")(idx_nw), "FGGIB", indexgib, smi_id("GPE")(smi_id_index), "GPE", indexgpe, 1))
          }
        }

        // which directions of GIBs are connected to GPE output port
        val to_dir = gpesParam(x)(y).to_dir
        unifiedpes(idx_c).io.out_fg.zipWithIndex.foreach { case (out, k) =>
          if (to_dir.contains(NORTHWEST)) {
            fggibs(idx_se).io.opinSE(k) := out
            val index = fggibs(idx_se).iPortMap("opinSE" + k.toString)
            connections.append((smi_id("GPE")(smi_id_index), "GPE", k, smi_id("FGGIB")(idx_se), "FGGIB", index, 1))
          }
          if (to_dir.contains(NORTHEAST)) {
            fggibs(idx_sw).io.opinSW(k) := out
            val index = fggibs(idx_sw).iPortMap("opinSW" + k.toString)
            connections.append((smi_id("GPE")(smi_id_index), "GPE", k, smi_id("FGGIB")(idx_sw), "FGGIB", index, 1))
          }
          if (to_dir.contains(SOUTHWEST)) {
            fggibs(idx_ne).io.opinNE(k) := out
            val index = fggibs(idx_ne).iPortMap("opinNE" + k.toString)
            connections.append((smi_id("GPE")(smi_id_index), "GPE", k, smi_id("FGGIB")(idx_ne), "FGGIB", index, 1))
          }
          if (to_dir.contains(SOUTHEAST)) {
            fggibs(idx_nw).io.opinNW(k) := out
            val index = fggibs(idx_nw).iPortMap("opinNW" + k.toString)
            connections.append((smi_id("GPE")(smi_id_index), "GPE", k, smi_id("FGGIB")(idx_nw), "FGGIB", index, 1))
          }
        }
      }
    }
  }

  // CGGIB to CGGIB connections
  if(numTrackCG > 0) {
    for (i <- 0 to rows) {
      for (j <- 0 to cols) {
        val idx_c = i * (cols + 1) + j // center
        val idx_n = (i - 1) * (cols + 1) + j // in center GIB's perspective
        val idx_w = i * (cols + 1) + j - 1
        val idx_e = i * (cols + 1) + j + 1
        val idx_s = (i + 1) * (cols + 1) + j
        if (i == 0) {
          cggibs(idx_c).io.itrackN.foreach { in => in := 0.U }
          cggibs(idx_c).io.itrackS.zipWithIndex.foreach { case (in, k) =>
            in := cggibs(idx_s).io.otrackN(k)
            val index1 = cggibs(idx_c).iPortMap("itrackS" + k.toString)
            val index2 = cggibs(idx_s).oPortMap("otrackN" + k.toString)
            connections.append((smi_id("CGGIB")(idx_s), "CGGIB", index2, smi_id("CGGIB")(idx_c), "CGGIB", index1, dataWidth))
          }
        } else if (i == rows) {
          cggibs(idx_c).io.itrackS.foreach { in => in := 0.U }
          cggibs(idx_c).io.itrackN.zipWithIndex.foreach { case (in, k) =>
            in := cggibs(idx_n).io.otrackS(k)
            val index1 = cggibs(idx_c).iPortMap("itrackN" + k.toString)
            val index2 = cggibs(idx_n).oPortMap("otrackS" + k.toString)
            connections.append((smi_id("CGGIB")(idx_n), "CGGIB", index2, smi_id("CGGIB")(idx_c), "CGGIB", index1, dataWidth))
          }
        } else {
          cggibs(idx_c).io.itrackN.zipWithIndex.foreach { case (in, k) =>
            in := cggibs(idx_n).io.otrackS(k)
            val index1 = cggibs(idx_c).iPortMap("itrackN" + k.toString)
            val index2 = cggibs(idx_n).oPortMap("otrackS" + k.toString)
            connections.append((smi_id("CGGIB")(idx_n), "CGGIB", index2, smi_id("CGGIB")(idx_c), "CGGIB", index1, dataWidth))
          }
          cggibs(idx_c).io.itrackS.zipWithIndex.foreach { case (in, k) =>
            in := cggibs(idx_s).io.otrackN(k)
            val index1 = cggibs(idx_c).iPortMap("itrackS" + k.toString)
            val index2 = cggibs(idx_s).oPortMap("otrackN" + k.toString)
            connections.append((smi_id("CGGIB")(idx_s), "CGGIB", index2, smi_id("CGGIB")(idx_c), "CGGIB", index1, dataWidth))
          }
        }
        if (j == 0) {
          cggibs(idx_c).io.itrackW.foreach { in => in := 0.U }
          cggibs(idx_c).io.itrackE.zipWithIndex.foreach { case (in, k) =>
            in := cggibs(idx_e).io.otrackW(k)
            val index1 = cggibs(idx_c).iPortMap("itrackE" + k.toString)
            val index2 = cggibs(idx_e).oPortMap("otrackW" + k.toString)
            connections.append((smi_id("CGGIB")(idx_e), "CGGIB", index2, smi_id("CGGIB")(idx_c), "CGGIB", index1, dataWidth))
          }
        } else if (j == cols) {
          cggibs(idx_c).io.itrackE.foreach { in => in := 0.U }
          cggibs(idx_c).io.itrackW.zipWithIndex.foreach { case (in, k) =>
            in := cggibs(idx_w).io.otrackE(k)
            val index1 = cggibs(idx_c).iPortMap("itrackW" + k.toString)
            val index2 = cggibs(idx_w).oPortMap("otrackE" + k.toString)
            connections.append((smi_id("CGGIB")(idx_w), "CGGIB", index2, smi_id("CGGIB")(idx_c), "CGGIB", index1, dataWidth))
          }
        } else {
          cggibs(idx_c).io.itrackW.zipWithIndex.foreach { case (in, k) =>
            in := cggibs(idx_w).io.otrackE(k)
            val index1 = cggibs(idx_c).iPortMap("itrackW" + k.toString)
            val index2 = cggibs(idx_w).oPortMap("otrackE" + k.toString)
            connections.append((smi_id("CGGIB")(idx_w), "CGGIB", index2, smi_id("CGGIB")(idx_c), "CGGIB", index1, dataWidth))
          }
          cggibs(idx_c).io.itrackE.zipWithIndex.foreach { case (in, k) =>
            in := cggibs(idx_e).io.otrackW(k)
            val index1 = cggibs(idx_c).iPortMap("itrackE" + k.toString)
            val index2 = cggibs(idx_e).oPortMap("otrackW" + k.toString)
            connections.append((smi_id("CGGIB")(idx_e), "CGGIB", index2, smi_id("CGGIB")(idx_c), "CGGIB", index1, dataWidth))
          }
        }
      }
    }
  }

  // FGGIB to FGGIB connections
  if(numTrackFG > 0) {
    for (i <- 0 to fg_rows) {
      for (j <- 0 to fg_columns) {
        val idx_c = i * (fg_columns + 1) + j // center
        val idx_n = (i - 1) * (fg_columns + 1) + j // in center GIB's perspective
        val idx_w = i * (fg_columns + 1) + j - 1
        val idx_e = i * (fg_columns + 1) + j + 1
        val idx_s = (i + 1) * (fg_columns + 1) + j
        if (i == 0) {
          fggibs(idx_c).io.itrackN.foreach { in => in := 0.U }
          fggibs(idx_c).io.itrackS.zipWithIndex.foreach { case (in, k) =>
            in := fggibs(idx_s).io.otrackN(k)
            val index1 = fggibs(idx_c).iPortMap("itrackS" + k.toString)
            val index2 = fggibs(idx_s).oPortMap("otrackN" + k.toString)
            connections.append((smi_id("FGGIB")(idx_s), "FGGIB", index2, smi_id("FGGIB")(idx_c), "FGGIB", index1, 1))
          }
        } else if (i == fg_rows) {
          fggibs(idx_c).io.itrackS.foreach { in => in := 0.U }
          fggibs(idx_c).io.itrackN.zipWithIndex.foreach { case (in, k) =>
            in := fggibs(idx_n).io.otrackS(k)
            val index1 = fggibs(idx_c).iPortMap("itrackN" + k.toString)
            val index2 = fggibs(idx_n).oPortMap("otrackS" + k.toString)
            connections.append((smi_id("FGGIB")(idx_n), "FGGIB", index2, smi_id("FGGIB")(idx_c), "FGGIB", index1, 1))
          }
        } else {
          fggibs(idx_c).io.itrackN.zipWithIndex.foreach { case (in, k) =>
            in := fggibs(idx_n).io.otrackS(k)
            val index1 = fggibs(idx_c).iPortMap("itrackN" + k.toString)
            val index2 = fggibs(idx_n).oPortMap("otrackS" + k.toString)
            connections.append((smi_id("FGGIB")(idx_n), "FGGIB", index2, smi_id("FGGIB")(idx_c), "FGGIB", index1, 1))
          }
          fggibs(idx_c).io.itrackS.zipWithIndex.foreach { case (in, k) =>
            in := fggibs(idx_s).io.otrackN(k)
            val index1 = fggibs(idx_c).iPortMap("itrackS" + k.toString)
            val index2 = fggibs(idx_s).oPortMap("otrackN" + k.toString)
            connections.append((smi_id("FGGIB")(idx_s), "FGGIB", index2, smi_id("FGGIB")(idx_c), "FGGIB", index1, 1))
          }
        }
        if (j == 0) {
          fggibs(idx_c).io.itrackW.foreach { in => in := 0.U }
          fggibs(idx_c).io.itrackE.zipWithIndex.foreach { case (in, k) =>
            in := fggibs(idx_e).io.otrackW(k)
            val index1 = fggibs(idx_c).iPortMap("itrackE" + k.toString)
            val index2 = fggibs(idx_e).oPortMap("otrackW" + k.toString)
            connections.append((smi_id("FGGIB")(idx_e), "FGGIB", index2, smi_id("FGGIB")(idx_c), "FGGIB", index1, 1))
          }
        } else if (j == fg_columns) {
          fggibs(idx_c).io.itrackE.foreach { in => in := 0.U }
          fggibs(idx_c).io.itrackW.zipWithIndex.foreach { case (in, k) =>
            in := fggibs(idx_w).io.otrackE(k)
            val index1 = fggibs(idx_c).iPortMap("itrackW" + k.toString)
            val index2 = fggibs(idx_w).oPortMap("otrackE" + k.toString)
            connections.append((smi_id("FGGIB")(idx_w), "FGGIB", index2, smi_id("FGGIB")(idx_c), "FGGIB", index1, 1))
          }
        } else {
          fggibs(idx_c).io.itrackW.zipWithIndex.foreach { case (in, k) =>
            in := fggibs(idx_w).io.otrackE(k)
            val index1 = fggibs(idx_c).iPortMap("itrackW" + k.toString)
            val index2 = fggibs(idx_w).oPortMap("otrackE" + k.toString)
            connections.append((smi_id("FGGIB")(idx_w), "FGGIB", index2, smi_id("FGGIB")(idx_c), "FGGIB", index1, 1))
          }
          fggibs(idx_c).io.itrackE.zipWithIndex.foreach { case (in, k) =>
            in := fggibs(idx_e).io.otrackW(k)
            val index1 = fggibs(idx_c).iPortMap("itrackE" + k.toString)
            val index2 = fggibs(idx_e).oPortMap("otrackW" + k.toString)
            connections.append((smi_id("FGGIB")(idx_e), "FGGIB", index2, smi_id("FGGIB")(idx_c), "FGGIB", index1, 1))
          }
        }
      }
    }
  }

  // apply("connections", connections) 
  apply("connections", connections.zipWithIndex.map{case (c, i) => i -> c}.toMap)
  
  // Configurations, each row share one config bus
  val cfgRegNum = totalRows - 1
  val cfgRegs = RegInit(VecInit(Seq.fill(cfgRegNum)(0.U((1+cfgAddrWidth+cfgDataWidth).W))))
  cfgRegs(0) := Cat(io.cfg_en, io.cfg_addr, io.cfg_data)
  (1 until cfgRegNum).map{ i => cfgRegs(i) := cfgRegs(i-1) }
  iobs.zipWithIndex.foreach{ case (iob, i) =>
    if(i < cols){ // top row
      iob.io.cfg_en   := io.cfg_en
      iob.io.cfg_addr := io.cfg_addr
      iob.io.cfg_data := io.cfg_data
    }else{ // bottom row
      iob.io.cfg_en   := cfgRegs(cfgRegNum-1)(cfgAddrWidth+cfgDataWidth)
      iob.io.cfg_addr := cfgRegs(cfgRegNum-1)(cfgAddrWidth+cfgDataWidth-1, cfgDataWidth)
      iob.io.cfg_data := cfgRegs(cfgRegNum-1)(cfgDataWidth-1, 0)
    }
  }

  for(i <- 0 to rows){
    val gibCfgIdx = row_idxs_cggib(i) - 1
    val peCfgIdx = {if(i < rows) row_idxs_pe(i) - 1 else 0}
    for(j <- 0 to cols){
      cggibs(i*(cols+1)+j).io.cfg_en   := cfgRegs(gibCfgIdx)(cfgAddrWidth+cfgDataWidth)
      cggibs(i*(cols+1)+j).io.cfg_addr := cfgRegs(gibCfgIdx)(cfgAddrWidth+cfgDataWidth-1, cfgDataWidth)
      cggibs(i*(cols+1)+j).io.cfg_data := cfgRegs(gibCfgIdx)(cfgDataWidth-1, 0)
      if((i < rows) && (j < cols)) {
        val gpe_type = gpe_posmap((i, j))
        val gpe_param = gpe_typemap(gpe_type)
        val gpe_mode = gpe_param.gpe_mode
        if (gpe_mode == 0) {
          val idx = gpe_index_map(i * cols + j)
          pes(idx).io.cfg_en := cfgRegs(peCfgIdx)(cfgAddrWidth + cfgDataWidth)
          pes(idx).io.cfg_addr := cfgRegs(peCfgIdx)(cfgAddrWidth + cfgDataWidth - 1, cfgDataWidth)
          pes(idx).io.cfg_data := cfgRegs(peCfgIdx)(cfgDataWidth - 1, 0)
        }else if(gpe_mode == 1){
          val idx = unifiedgpe_index_map(i * cols + j)
          unifiedpes(idx).io.cfg_en := cfgRegs(peCfgIdx)(cfgAddrWidth + cfgDataWidth)
          unifiedpes(idx).io.cfg_addr := cfgRegs(peCfgIdx)(cfgAddrWidth + cfgDataWidth - 1, cfgDataWidth)
          unifiedpes(idx).io.cfg_data := cfgRegs(peCfgIdx)(cfgDataWidth - 1, 0)
        }else {
          val idx = xcore_index_map(i * cols + j)
          xcores(idx).io.cfg_en := cfgRegs(peCfgIdx)(cfgAddrWidth + cfgDataWidth)
          xcores(idx).io.cfg_addr := cfgRegs(peCfgIdx)(cfgAddrWidth + cfgDataWidth - 1, cfgDataWidth)
          xcores(idx).io.cfg_data := cfgRegs(peCfgIdx)(cfgDataWidth - 1, 0)
        }
      }
    }
  }

  for (i <- 0 to fg_rows) {
    val gibCfgIdx = row_idxs_fggib(i) - 1
    for (j <- 0 to fg_columns) {
      fggibs(i*(fg_columns+1)+j).io.cfg_en   := cfgRegs(gibCfgIdx)(cfgAddrWidth+cfgDataWidth)
      fggibs(i*(fg_columns+1)+j).io.cfg_addr := cfgRegs(gibCfgIdx)(cfgAddrWidth+cfgDataWidth-1, cfgDataWidth)
      fggibs(i*(fg_columns+1)+j).io.cfg_data := cfgRegs(gibCfgIdx)(cfgDataWidth-1, 0)
    }
  }

  // config bits of the blocks
  val blkCfgBits = ListBuffer[Int]()
  blkCfgBits ++= iobs.map(_.sumCfgWidth).toList
  blkCfgBits ++= pes.map(_.sumCfgWidth).toList
  blkCfgBits ++= unifiedpes.map(_.sumCfgWidth).toList
  blkCfgBits ++= xcores.map(_.sumCfgWidth).toList
  blkCfgBits ++= cggibs.map(_.cfgsBit).toList
  blkCfgBits ++= fggibs.map(_.cfgsBit).toList
  val maxCfgDataNum = blkCfgBits.map{ x => (x+cfgDataWidth-1)/cfgDataWidth }.sum
    println("Max cfg bits: " + blkCfgBits.max, ", Min cfg bits: " + blkCfgBits.min, ", Total cfg bits: " + blkCfgBits.sum)
  apply("max_blk_cfg_bits", blkCfgBits.max)
  apply("min_blk_cfg_bits", blkCfgBits.min)
  apply("sum_blk_cfg_bits", blkCfgBits.sum)
  apply("max_cfg_data_num", maxCfgDataNum)

  val addrWidthAlign = attrs("fgra_cfg_addr_width_align").asInstanceOf[Int]
  val maxCfgMemSize = maxCfgDataNum * (cfgDataWidth + addrWidthAlign) / 8;
  println("cfg_spad_lg_size: " + cfg_spad_lg_size + " maxCfgDataNum: " + maxCfgDataNum + " log2Ceil(maxCfgMemSize)" + log2Ceil(maxCfgMemSize))
  assert(cfg_spad_lg_size >= log2Ceil(maxCfgMemSize))

  if(attrs("dumpADG").asInstanceOf[Boolean]){
    printIR(attrs("fgra_adg_filename").asInstanceOf[String])
  }
  if(attrs("dumpOperationSet").asInstanceOf[Boolean]){
    OpInfo.setLatency("INPUT", load_latency)
    OpInfo.setLatency("OUTPUT", store_latency)
    OpInfo.setLatency("LOAD", load_latency)
    OpInfo.setLatency("STORE", store_latency)
    OpInfo.setLatency("CLOAD", load_latency)
    OpInfo.setLatency("CSTORE", store_latency)
    OpInfo.dumpOpInfo(attrs("operation_set_filename").asInstanceOf[String])
  }

//  println("area is :" + area)

}





//object VerilogGen extends App {
//  val connect_flexibility = mutable.Map(
//    "num_itrack_per_ipin" -> 2, // ipin number = 3
//    "num_otrack_per_opin" -> 6, // opin number = 1
//    "num_ipin_per_opin"   -> 9
//  )
//  val attrs: mutable.Map[String, Any] = mutable.Map(
//    "num_row" -> 4,
//    "num_colum" -> 4,
//    "data_width" -> 32,
//    "cfg_data_width" -> 64,
//    "cfg_addr_width" -> 8,
//    "cfg_blk_offset" -> 2,
//    "num_rf_reg" -> 1,
//    "operations" -> ListBuffer("PASS", "ADD", "SUB", "MUL", "AND", "OR", "XOR", "SEL"),
//    "max_delay" -> 4,
//    "num_track" -> 3,
//    "connect_flexibility" -> connect_flexibility,
//    "num_output_ib" -> 3,
//    "num_input_ob" -> 6
//  )
//
//  (new chisel3.stage.ChiselStage).emitVerilog(new CGRA(attrs, true), args)
//}