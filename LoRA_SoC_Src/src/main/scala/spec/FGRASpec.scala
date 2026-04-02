//  package fgramemfp.spec
//  // Architecture Specification

//  import chisel3._
//  import chisel3.util._
//  import scala.collection.mutable
//  import scala.collection.mutable.ListBuffer
//  import fgramemfp.common.MacroVar._
//  import fgramemfp.ir._

//  // GPE Spec to support heterogeneous GPEs
//  class GpeSpec(// num_reg_rf_for_alu_ : Int = 1, num_reg_rf_for_lut_ : Int = 1,
//                max_delay_cg_ : Int = 4,
//                max_delay_fg_ : Int = 8,
//                num_input_lut_ : Int = 3,
//                operations_ : ListBuffer[String] = ListBuffer( "PASS", "ADD", "SUB")
// //               fromdir_ : List[Int] =  List(NORTHWEST, NORTHEAST, SOUTHWEST, SOUTHEAST),
// //               todir_ : List[Int] = List(NORTHWEST, NORTHEAST, SOUTHWEST, SOUTHEAST)
//               ){
// //   var num_reg_rf_for_alu =  num_reg_rf_for_alu_  // number of registers in Regfile for ALU
// //   var num_reg_rf_for_lut =  num_reg_rf_for_lut_  // number of registers in Regfile for LUT
//    var max_delay_cg = max_delay_cg_     // max delay cycles of the fine-grained SharedDelayPipe
//    var max_delay_fg = max_delay_fg_     // max delay cycles of the fine-grained SharedDelayPipe
//    var num_input_lut = num_input_lut_   // LUT input number
//    var operations = operations_   // supported operations
// //   var from_dir = fromdir_        // which directions the GPE inputs are from
// //   var to_dir = todir_            // which directions the GPE outputs are to
//  }

//  object GpeSpec {
//    def apply(max_delay_cg_ : Int = 4,
//              max_delay_fg_ : Int = 8,
//              num_input_lut_ : Int = 3,
//              operations_ : ListBuffer[String] = ListBuffer( "PASS", "ADD", "SUB")) = {
//      new GpeSpec(max_delay_cg_, max_delay_fg_, num_input_lut_, operations_)
// //   def apply(num_reg_rf_for_alu_ : Int = 1, num_reg_rf_for_lut_ : Int = 1,
// //             max_delay_cg_ : Int = 4, max_delay_fg_ : Int = 8,
// //             num_input_lut_ : Int = 3,
// //             operations_ : ListBuffer[String] = ListBuffer( "PASS", "ADD", "SUB"),
// //             fromdir_ : List[Int] =  List(NORTHWEST, NORTHEAST, SOUTHWEST, SOUTHEAST),
// //             todir_ : List[Int] = List(NORTHWEST, NORTHEAST, SOUTHWEST, SOUTHEAST)) = {
// //     new GpeSpec(num_reg_rf_for_alu_, num_reg_rf_for_lut_, max_delay_cg_, max_delay_fg_,
// //       num_input_lut_, operations_, fromdir_, todir_)
//    }
//  }


//  // GIB Spec to support heterogeneous GIBs
//  class GibSpec(diagIOPinConnect : Boolean = true , fclist_ : List[Int] = List(2, 4, 4)){
//    var diag_iopin_connect = diagIOPinConnect // if support diagonal connections between OPins and IPins
//    var fclist = fclist_ // num_itrack_per_ipin, num_otrack_per_opin, num_ipin_per_opin
//    // "num_itrack_per_ipin" : ipin-itrack connection flexibility, connected track number
//    // "num_otrack_per_opin" : opin-otrack connection flexibility, connected track number
//    // "num_ipin_per_opin"   : opin-ipin  connection flexibility, connected ipin number
//  }

//  object GibSpec {
//    def apply(diagIOPinConnect : Boolean = true , fclist_ : List[Int] = List(2, 4, 4)) = {
//      new GibSpec(diagIOPinConnect, fclist_)
//    }
//  }

//  // IOB Spec to support heterogeneous IOBs
//  class IobSpec(mode : Int = FIFO_MODE,
//                hasIOFG : Boolean = true,
//                maxDelayCG : Int = 4,
//                maxDelayFG : Int = 4){
//    var iob_mode = mode
//    // 0: IFIFO mode (with dout), 1: OFIFO mode(with din),
//    // 2: IOFIFO mode (with din, dout), 3: SRAM mode (with addr, din, dout)
//    var has_io_fg = hasIOFG  // IF has fine-grained IO ports
//    var max_delay_cg = maxDelayCG     // max delay cycles of the fine-grained SharedDelayPipe
//    var max_delay_fg = maxDelayFG     // max delay cycles of the fine-grained SharedDelayPipe
//  }

//  object IobSpec {
//    def apply(mode : Int = FIFO_MODE,
//              hasIOFG : Boolean = true,
//              maxDelayCG : Int = 4,
//              maxDelayFG : Int = 4) = {
//      new IobSpec(mode, hasIOFG, maxDelayCG, maxDelayFG)
//    }
//  }

//  // FGRA Specification
//  object FGRASpec{
//    val system_bus_beat_bits = 64 // data width of the system bus
//    val spad_bank_lg_size = 10    // log2(single scratchpad bank size in byts)
//    val fgra_iob_sram_banks_coalesce = 1 // coalescing sram banks that FGRA IOB can access
// //   val fgra_cfg_sram_banks_cascade = 2 // cascading sram banks that FGRA config controller can access
//    val connect_flexibility = mutable.Map(
//      "num_itrack_per_ipin" -> 2, // ipin number = 2
//      "num_otrack_per_opin" -> 4, // opin number = 1
//      "num_ipin_per_opin"   -> 4
//    )

//    val attrs: mutable.Map[String, Any] = mutable.Map(
//      // 1. FGRA Controller parameters
//      // 1.1. FGRA Global parameters
//      "fgra_num_row" -> 4,
//      "fgra_num_colum" -> 4,
//      "fgra_data_width" -> 32,
//      "fgra_cfg_data_width" -> 32,
//      "fgra_cfg_addr_width" -> 12,
//      "fgra_cfg_blk_offset" -> 2,
//      "fgra_max_delay_cg" -> 4, // for both GPE and IOB
//      "fgra_max_delay_fg" -> 8, // for both GPE and IOB
//      // 1.2. GPE attributes (default for all)
//      "fgra_gpe_num_reg_rf_for_alu" -> 1,
//      "fgra_gpe_num_reg_rf_for_lut" -> 1,
//      "fgra_gpe_operations" -> ListBuffer("PASS", "ADD", "SUB", "MUL", "AND", "OR", "XOR", "SHL", "LSHR", "ASHR", "EQ", "ULT", "ULE", "SEL"),
//      "fgra_gpe_num_input_lut" -> 3,
//      "fgra_gpe_in_from_dir" -> List(NORTHWEST, NORTHEAST, SOUTHWEST, SOUTHEAST),
//      "fgra_gpe_out_to_dir" -> List(NORTHWEST, NORTHEAST, SOUTHWEST, SOUTHEAST),
//      // 1.3. GIB attributes (default for all)
//      "fgra_gib_num_track_cg" -> 1,
//      "fgra_gib_num_track_fg" -> 2,
//      "fgra_gib_track_reged_mode_cg" -> 1,
//      "fgra_gib_track_reged_mode_fg" -> 1,
//      "fgra_gib_connect_flexibility_cg" -> connect_flexibility,
//      "fgra_gib_connect_flexibility_fg" -> connect_flexibility,
//      "fgra_gib_diag_iopin_connect_cg" -> true,
//      "fgra_gib_diag_iopin_connect_fg" -> true,
//      // 1.4. IOB attributes (default for all)
//      "fgra_iob_sram_addr_width" -> (spad_bank_lg_size + log2Ceil(fgra_iob_sram_banks_coalesce)), // address in byte
//      "fgra_iob_sram_has_mask" -> false,
//      "fgra_iob_sram_add_reg" -> true,
//      "fgra_iob_num_sides" -> 2,   // now only support top/bottom side
//      "fgra_iob_mode" -> FIFO_MODE,
//      "fgra_iob_has_io_fg" -> true,
//      "fgra_iob_lg_max_stride" -> 8,
//      "fgra_iob_lg_max_lat" -> 10,
//      "fgra_iob_lg_max_cycles" -> 12,
//      "fgra_iob_lg_max_ii" -> 4,
//      "fgra_iob_sram_banks_coalesce" -> fgra_iob_sram_banks_coalesce,
//      "fgra_iob_ag_nest_levels" -> 3,
//      // 1.5. FGRA Config controller parameters
//      "fgra_cfg_addr_width_align" -> 16, // cfg_data and cfg_addr are stored as an array in scratchpad, cfg_addr_width should be aligned
// //     "fgra_cfg_sram_banks_cascade" -> fgra_cfg_sram_banks_cascade,
//      //    "fgra_cfg_sram_data_width" -> 32,
// //     "fgra_cfg_sram_addr_width" -> (spad_bank_lg_size + log2Ceil(fgra_cfg_sram_banks_cascade)), // address in byte
//      "fgra_cfg_sram_add_reg" -> false, // add pipeline register into the SRAM IF to improve timing
//      //    "fgra_cfg_sram_read_latency" -> 1,
//      // 1.6. FGRA Execute controller parameters
//      "fgra_exe_lg_max_ii" -> 4,
//      "fgra_exe_lg_max_loop_cycles" -> 10,
//      "fgra_exe_lg_max_execute_cycles" -> 16,
//      // 2. Scratchpad parameters
//      "spad_data_width" -> system_bus_beat_bits,
//      "spad_bank_lg_size" -> spad_bank_lg_size,
//      "spad_cfg_lg_size" -> 10,
//      //    "spad_num_banks" -> (fgra_iob_num_sides * fgra_num_colum + fgra_cfg_sram_banks_cascade)
//      // 3. Load/Store controller parameters
//      "ls_stream_queue_depth" -> 2,  // cache stream data to reduce combinational path length
//      // 4. Reservation station parameters
//      "id_width" -> 8, // command ID for debug
//      "rs_cmd_queue_depth" -> 16,
//      "rs_load_queue_depth" -> 8,
//      "rs_store_queue_depth" -> 8,
//      "rs_exe_queue_depth" -> 4,   // FGRA controller queue
//      // 5. DMA parameters
//      "dma_num_req_in_flight" -> 8,
//      "dma_lg_max_burst_size" -> 6, // max data size in bytes of one burst transferring, <=6 (limited by TileLink edge attribute)
//      // 6. TLB parameters
//      "tlb_num_ways" -> 32,  // way number in the set-associate tlb
//      "tlb_is_shared" -> true, // TLB is shared by DMA reader and writer
//      // 7. system bus parameters
//      "system_bus_beat_bits" -> system_bus_beat_bits,
//      // 8. misc parameters
//      "dumpOperationSet" -> true,
//      "dumpADG" -> true,
//      "operation_set_filename" -> "operations.json",
//      "fgra_adg_filename" -> "fgra_adg.json"
//    )
//    attrs += ("spad_num_banks" -> (attrs("fgra_iob_num_sides").asInstanceOf[Int] * attrs("fgra_num_colum").asInstanceOf[Int]))

//    //     attrs += ("gpes" -> gpes_spec)
//    //     attrs += ("gibs" -> gibs_spec)
//    // set default values from attr
//    // the attributes in attrs are used as default values
//    def setDefaultGpesSpec(): Unit = {
//      val gpes_spec = ListBuffer[ListBuffer[GpeSpec]]()
//      for(i <- 0 until attrs("fgra_num_row").asInstanceOf[Int]){
//        gpes_spec.append(new ListBuffer[GpeSpec])
//        for( j <- 0 until attrs("fgra_num_colum").asInstanceOf[Int]){
//          //         val num_reg_rf_for_alu = attrs("fgra_num_reg_rf_for_alu").asInstanceOf[Int]
//          //         val num_reg_rf_for_lut = attrs("fgra_num_reg_rf_for_lut").asInstanceOf[Int]
//          val max_delay_cg = attrs("fgra_max_delay_cg").asInstanceOf[Int]
//          val max_delay_fg = attrs("fgra_max_delay_fg").asInstanceOf[Int]
//          val num_input_lut = attrs("fgra_gpe_num_input_lut").asInstanceOf[Int]
//          val operations = attrs("fgra_gpe_operations").asInstanceOf[ListBuffer[String]]
//          //         val fromdir = attrs("fgra_gpe_in_from_dir").asInstanceOf[List[Int]]
//          //         val todir= attrs("fgra_gpe_out_to_dir").asInstanceOf[List[Int]]
//          //         gpes_spec(i).append(GpeSpec(num_reg_rf_for_alu, num_reg_rf_for_lut, max_delay_cg, max_delay_fg,
//          //           num_input_lut, operations, fromdir, todir))
//          gpes_spec(i).append(GpeSpec(max_delay_cg, max_delay_fg, num_input_lut, operations))
//        }
//      }
//      attrs("fgra_gpes") = gpes_spec
//    }

//    def setDefaultIobsSpec(): Unit = {
//      val iobs_spec = ListBuffer[ListBuffer[IobSpec]]()
//      for(i <- 0 until attrs("fgra_iob_num_sides").asInstanceOf[Int]){
//        iobs_spec.append(new ListBuffer[IobSpec])
//        for( j <- 0 until attrs("fgra_num_colum").asInstanceOf[Int]){
//          val mode = attrs("fgra_iob_mode").asInstanceOf[Int]
//          val hasIOFG = attrs("fgra_iob_has_io_fg").asInstanceOf[Boolean]
//          val maxDelayCG = attrs("fgra_max_delay_cg").asInstanceOf[Int]
//          val maxDelayFG = attrs("fgra_max_delay_fg").asInstanceOf[Int]
//          iobs_spec(i).append(IobSpec(mode, hasIOFG, maxDelayCG, maxDelayFG))
//        }
//      }
//      attrs("fgra_iobs") = iobs_spec
//    }

//    // Coarse-grained GIBs
//    def setDefaultCgGibsSpec(): Unit = {
//      val gibs_spec = ListBuffer[ListBuffer[GibSpec]]()
//      for(i <- 0 to attrs("fgra_num_row").asInstanceOf[Int]){
//        gibs_spec.append(new ListBuffer[GibSpec])
//        for( j <- 0 to attrs("fgra_num_colum").asInstanceOf[Int]){
//          val diag_iopin_connect = attrs("fgra_gib_diag_iopin_connect_cg").asInstanceOf[Boolean]
//          val conf = attrs("fgra_gib_connect_flexibility_cg").asInstanceOf[mutable.Map[String, Int]]
//          val fclist = List(conf("num_itrack_per_ipin"), conf("num_otrack_per_opin"), conf("num_ipin_per_opin"))
//          gibs_spec(i).append(GibSpec(diag_iopin_connect, fclist))
//        }
//      }
//      attrs("fgra_cg_gibs") = gibs_spec
//    }

//    // Fine-grained GIBs
//    def setDefaultFgGibsSpec(): Unit = {
//      val num_row = attrs("fgra_num_row").asInstanceOf[Int]
//      val num_colum = attrs("fgra_num_colum").asInstanceOf[Int]
//      attrs("fgra_gpe_fg_rows") = (0 until num_row).toList     // FG positions: row index
//      attrs("fgra_gpe_fg_columns") = (0 until num_colum).toList // FG positions: column index
//      val gibs_spec = ListBuffer[ListBuffer[GibSpec]]()
//      for(i <- 0 to num_row){
//        gibs_spec.append(new ListBuffer[GibSpec])
//        for( j <- 0 to num_colum){
//          val diag_iopin_connect = attrs("fgra_gib_diag_iopin_connect_fg").asInstanceOf[Boolean]
//          val conf = attrs("fgra_gib_connect_flexibility_fg").asInstanceOf[mutable.Map[String, Int]]
//          val fclist = List(conf("num_itrack_per_ipin"), conf("num_otrack_per_opin"), conf("num_ipin_per_opin"))
//          gibs_spec(i).append(GibSpec(diag_iopin_connect, fclist))
//        }
//      }
//      attrs("fgra_fg_gibs") = gibs_spec
//    }

//    setDefaultGpesSpec()
//    setDefaultIobsSpec()
//    setDefaultCgGibsSpec()
//    setDefaultFgGibsSpec()

//    // NOTICE: TODO adding check
//    // fg_rows,fg_columns represent the rows and columns that GPEs with fine-grained IOs are located
//    // should be consistent with the FG-IOs of GPEs, IOBs and IOBs, ensuring that
//    // all the FG-IOs of GPEs and IOBs are connected to FG-GIBs without floating ports


//    def loadSpec(jsonFile : String): Unit ={
//      val jsonMap = IRHandler.loadIR(jsonFile)
//      var gpes_spec_update = false
//      var iobs_spec_update = false
//      var cg_gibs_spec_update = false
//      var fg_gibs_spec_update = false
//      for(kv <- jsonMap){
//        if(attrs.contains(kv._1)) {
//          if (kv._1 == "fgra_gpe_operations") {
//            attrs(kv._1) = ListBuffer[String]() ++ kv._2.asInstanceOf[List[String]]
//          } else if (kv._1 == "fgra_gib_connect_flexibility_cg") {
//            attrs(kv._1) = mutable.Map() ++ kv._2.asInstanceOf[Map[String, Int]]
//          } else if (kv._1 == "fgra_gib_connect_flexibility_fg") {
//            attrs(kv._1) = mutable.Map() ++ kv._2.asInstanceOf[Map[String, Int]]
//          } else if (kv._1 == "fgra_gpe_in_from_dir") {
//            attrs(kv._1) = kv._2.asInstanceOf[List[Int]]
//          } else if (kv._1 == "fgra_gpe_out_to_dir") {
//            attrs(kv._1) = kv._2.asInstanceOf[List[Int]]
//          } else if (kv._1 == "fgra_gpe_fg_rows") {
//            attrs(kv._1) = kv._2.asInstanceOf[List[Int]]
//          } else if (kv._1 == "fgra_gpe_fg_columns") {
//            attrs(kv._1) = kv._2.asInstanceOf[List[Int]]
//          } else if (kv._1 == "fgra_gpes") {
//            gpes_spec_update = true
//            val gpe_2d = kv._2.asInstanceOf[List[List[Any]]]
//            val gpes_spec = ListBuffer[ListBuffer[GpeSpec]]()
//            for (i <- gpe_2d.indices) {
//              gpes_spec.append(new ListBuffer[GpeSpec])
//              val gpe_1d = gpe_2d(i)
//              for (j <- gpe_1d.indices) {
//                val gpemap = gpe_1d(j).asInstanceOf[Map[String, Any]]
//                //         val num_reg_rf_for_alu = gpemap("num_reg_rf_for_alu").asInstanceOf[Int]
//                //         val num_reg_rf_for_lut = gpemap("num_reg_rf_for_lut").asInstanceOf[Int]
//                val max_delay_cg = gpemap("max_delay_cg").asInstanceOf[Int]
//                val max_delay_fg = gpemap("max_delay_fg").asInstanceOf[Int]
//                val num_input_lut = gpemap("num_input_lut").asInstanceOf[Int]
//                val operations = ListBuffer[String]() ++ gpemap("operations").asInstanceOf[List[String]]
//                //         val fromdir = gpemap("in_from_dir").asInstanceOf[List[Int]]
//                //         val todir= gpemap("out_to_dir").asInstanceOf[List[Int]]
//                //         gpes_spec(i).append(GpeSpec(num_reg_rf_for_alu, num_reg_rf_for_lut, max_delay_cg, max_delay_fg,
//                //           num_input_lut, operations, fromdir, todir))
//                gpes_spec(i).append(GpeSpec(max_delay_cg, max_delay_fg, num_input_lut, operations))
//              }
//            }
//            attrs("fgra_gpes") = gpes_spec
//          } else if (kv._1 == "fgra_iobs") {
//            iobs_spec_update = true
//            val iob_2d = kv._2.asInstanceOf[List[List[Any]]]
//            val iobs_spec = ListBuffer[ListBuffer[IobSpec]]()
//            for (i <- iob_2d.indices) {
//              iobs_spec.append(new ListBuffer[IobSpec])
//              val iob_1d = iob_2d(i)
//              for (j <- iob_1d.indices) {
//                val iobmap = iob_1d(j).asInstanceOf[Map[String, Any]]
//                val mode = iobmap("iob_mode").asInstanceOf[Int]
//                val hasIOFG = iobmap("has_io_fg").asInstanceOf[Boolean]
//                val maxDelayCG = iobmap("max_delay_cg").asInstanceOf[Int]
//                val maxDelayFG = iobmap("max_delay_fg").asInstanceOf[Int]
//                iobs_spec(i).append(IobSpec(mode, hasIOFG, maxDelayCG, maxDelayFG))
//              }
//            }
//            attrs("fgra_iobs") = iobs_spec
//          } else if (kv._1 == "fgra_cg_gibs") {
//            cg_gibs_spec_update = true
//            val gib_2d = kv._2.asInstanceOf[List[List[Any]]]
//            val gibs_spec = ListBuffer[ListBuffer[GibSpec]]()
//            for (i <- gib_2d.indices) {
//              gibs_spec.append(new ListBuffer[GibSpec])
//              val gib_1d = gib_2d(i)
//              for (j <- gib_1d.indices) {
//                val gibmap = gib_1d(j).asInstanceOf[Map[String, Any]]
//                val diag_iopin_connect = gibmap("diag_iopin_connect").asInstanceOf[Boolean]
//                val fclist = gibmap("fclist").asInstanceOf[List[Int]]
//                gibs_spec(i).append(GibSpec(diag_iopin_connect, fclist))
//              }
//            }
//            attrs("fgra_cg_gibs") = gibs_spec
//          } else if (kv._1 == "fgra_fg_gibs") {
//            fg_gibs_spec_update = true
//            val gib_2d = kv._2.asInstanceOf[List[List[Any]]]
//            val gibs_spec = ListBuffer[ListBuffer[GibSpec]]()
//            for (i <- gib_2d.indices) {
//              gibs_spec.append(new ListBuffer[GibSpec])
//              val gib_1d = gib_2d(i)
//              for (j <- gib_1d.indices) {
//                val gibmap = gib_1d(j).asInstanceOf[Map[String, Any]]
//                val diag_iopin_connect = gibmap("diag_iopin_connect").asInstanceOf[Boolean]
//                val fclist = gibmap("fclist").asInstanceOf[List[Int]]
//                gibs_spec(i).append(GibSpec(diag_iopin_connect, fclist))
//              }
//            }
//            attrs("fgra_fg_gibs") = gibs_spec
//          } else {
//            attrs(kv._1) = kv._2
//          }
//        }
//      }
//      if(gpes_spec_update == false){ // set default values
//        setDefaultGpesSpec()
//      }
//      if(iobs_spec_update == false){ // set default values
//        setDefaultIobsSpec()
//      }
//      if(cg_gibs_spec_update == false){ // set default values
//        setDefaultCgGibsSpec()
//      }
//      if(fg_gibs_spec_update == false){ // set default values
//        setDefaultFgGibsSpec()
//      }
//      // verification
//      assert(attrs("fgra_iob_sram_addr_width").asInstanceOf[Int] == attrs("spad_bank_lg_size").asInstanceOf[Int] +
//        log2Ceil(attrs("fgra_iob_sram_banks_coalesce").asInstanceOf[Int]))
//        println("spad_bank_lg_size: " + attrs("spad_bank_lg_size").asInstanceOf[Int])
// //     assert(attrs("fgra_cfg_sram_addr_width").asInstanceOf[Int] == attrs("spad_bank_lg_size").asInstanceOf[Int] +
// //       log2Ceil(attrs("fgra_cfg_sram_banks_cascade").asInstanceOf[Int]))
//      //    assert(attrs("fgra_cfg_sram_data_width").asInstanceOf[Int] == attrs("system_bus_beat_bits").asInstanceOf[Int])
//      assert(attrs("spad_data_width").asInstanceOf[Int] == attrs("system_bus_beat_bits").asInstanceOf[Int])
//      //    if(attrs("fgra_iob_mode").asInstanceOf[Int] == SRAM_MODE){
//      //      assert(attrs("fgra_iob_sram_add_reg").asInstanceOf[Boolean] == true)
//      //    }
//    }

//    def dumpSpec(jsonFile : String): Unit={
//      IRHandler.dumpIR(attrs, jsonFile)
//    }
//  }


// // object testjson extends  App{
// //   IRHandler.dumpIR( attrs , "test.json")
// // }



