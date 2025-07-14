package fgramem

import chisel3._
import chisel3.util._
import freechips.rocketchip.tile._
import org.chipsalliance.cde.config._
import freechips.rocketchip.rocket._
import fgramem.dsa._
import FgramemISA._
import scala.collection.mutable


/** FGRA controller
  * @param attrs     module attributes
  */
class FGRAController(attrs: mutable.Map[String, Any])(implicit p: Parameters) extends CoreModule {
  val idWidth = attrs("id_width").asInstanceOf[Int]
  val dataWidthFgra = attrs("fgra_data_width").asInstanceOf[Int]
  val nBanksIOB = attrs("fgra_iob_num_sides").asInstanceOf[Int] * attrs("fgra_num_colum").asInstanceOf[Int]
//  val nBanksCfg = attrs("fgra_cfg_sram_banks_cascade").asInstanceOf[Int]
//  val nBanks = nBanksIOB + nBanksCfg
  val addrWidthSram = attrs("spad_bank_lg_size").asInstanceOf[Int] - log2Ceil(dataWidthFgra/8) // add 1 per data
  val dataWidthSramCfg = attrs("spad_data_width").asInstanceOf[Int] // data width in bit
  val addrWidthSramCfg = attrs("spad_cfg_lg_size").asInstanceOf[Int] - log2Ceil(dataWidthSramCfg/8) // add 1 per data
  val hasMaskSram = false // attrs("fgra_iob_sram_has_mask").asInstanceOf[Boolean]
  val hasMaskSramCfg = false  // if has write data byte mask
  val readLatencySramCfg = 1  // read latency
  val coalesceBanksIOB = attrs("fgra_iob_sram_banks_coalesce").asInstanceOf[Int]
//  val lgMaxLoopCycles = attrs("fgra_exe_lg_max_loop_cycles").asInstanceOf[Int]  // log2(max in/out cycles)
//  val lgMaxExeCycles = attrs("fgra_exe_lg_max_execute_cycles").asInstanceOf[Int]  // log2(max execute cycles)
//  val lgMaxII = attrs("fgra_exe_lg_max_ii").asInstanceOf[Int]             // log2(max in/out Initialization Interval)
  val cfgDataWidth = attrs("fgra_cfg_data_width").asInstanceOf[Int]
  val cfgAddrWidth = attrs("fgra_cfg_addr_width").asInstanceOf[Int]
  val cfgAddrWidthAlign = attrs("fgra_cfg_addr_width_align").asInstanceOf[Int]

  val io = IO(new Bundle {
    val core = new CoreIF(idWidth)
    val srams_iob = Vec(nBanksIOB, Flipped(new SRAMIO(dataWidthFgra, addrWidthSram, hasMaskSram)))
    val sram_cfg = Flipped(new SRAMIO(dataWidthSramCfg, addrWidthSramCfg, hasMaskSram))
  })

  val fgra = Module(new FGRA(attrs))
  val cfgRegNum = fgra.cfgRegNum // config chain register number
  val cfgCtrl = Module(new ConfigController(dataWidthSramCfg, addrWidthSramCfg, hasMaskSramCfg, readLatencySramCfg,
    cfgDataWidth, cfgAddrWidth, cfgAddrWidthAlign, cfgRegNum))
  val sram_coalesce_iob = Module(new SRAMCoalesce(dataWidthFgra, addrWidthSram, hasMaskSram, nBanksIOB, coalesceBanksIOB))
//  val sram_cascade_cfg = Module(new SRAMBanksCascade(dataWidthFgra, addrWidthSram, hasMaskSram, nBanksCfg))
  fgra.io.srams <> sram_coalesce_iob.io.coal
  io.srams_iob <> sram_coalesce_iob.io.orig
  io.sram_cfg <> cfgCtrl.io.sram
//  cfgCtrl.io.sram <> sram_cascade_cfg.io.casc
//  io.srams.zipWithIndex.foreach{ case (sram, i) =>
//    if(i < nBanksIOB){
//      sram_coalesce_iob.io.orig(i) <> sram
//    }else{
//      sram_cascade_cfg.io.orig(i-nBanksIOB) <> sram
//    }
//  }
  fgra.io.cfg_en := cfgCtrl.io.cfg_en
  fgra.io.cfg_addr := cfgCtrl.io.cfg_addr
  fgra.io.cfg_data := cfgCtrl.io.cfg_data

//  val cfg_cmd = Reg(new CoreReq(idWidth))
//  val cfg_rs1 = cfg_cmd.cmd.rs1.asTypeOf(new FgraCfgRs1)
//  val cfg_rs2 = cfg_cmd.cmd.rs2.asTypeOf(new FgraCfgRs2)
  val id = RegInit(0.U(idWidth.W))
  val cfg_base_addr = RegInit(0.U(addrWidthSramCfg.W))
  val cfg_num = RegInit(0.U(cfgAddrWidth.W))
//  val ii = RegInit(0.U(lgMaxII.W)) // Initialization Interval
//  val loop_cycles = RegInit(0.U(lgMaxLoopCycles.W)) // loop cycles
//  val exe_cycles = RegInit(0.U(lgMaxExeCycles.W)) // execute cycles
  val iob_ens = RegInit(0.U((nBanksIOB).W)) // enable signals for every IOB
  val cfg_start = RegInit(false.B)
  val exe_start = RegInit(false.B)
  val ex_enable = RegInit(false.B)

  // multiple small request FSM to enable cache multiple different commands
  // global FSM controls the config and execute flow
//  val s_req_idle :: s_req_cache :: Nil = Enum(2)
//  val cfgReqState = RegInit(s_req_idle)
//  val exeReqState = RegInit(s_req_idle)
  val s_idle :: s_cfg_run :: s_cfg_resp :: s_exe_wait :: s_exe_run :: s_exe_resp :: Nil = Enum(6)
  val state = RegInit(s_idle)

//  // Config request FSM
//  val isCfgCmd = io.core.req.bits.cmd.inst.funct === CGRA_CFG_CMD
////  cfg_cmd := DontCare
//  switch(cfgReqState){
//    is(s_req_idle){
//      when(io.core.req.valid && isCfgCmd){
//        cfg_cmd := io.core.req.bits
//        cfgReqState := s_req_cache
//      }
//    }
//    is(s_req_cache){
//      when(state === s_cfg_run){ // cached cfg_cmd has been used, ready for next cmd
//        cfgReqState := s_req_idle
//      }
//    }
//  }

//  // Execute request FSM
//  val isExeCmd = io.core.req.bits.cmd.inst.funct === EXE_CMD
////  exe_cmd := DontCare
//  switch(exeReqState){
//    is(s_req_idle){
//      when(io.core.req.valid && isExeCmd){
//        exe_cmd := io.core.req.bits
//        exeReqState := s_req_cache
//      }
//    }
//    is(s_req_cache){
//      when(state === s_exe_run){ // cached exe_cmd has been used, ready for next cmd
//        exeReqState := s_req_idle
//      }
//    }
//  }

//  io.core.req.ready := (cfgReqState === s_req_idle && isCfgCmd) ||  (exeReqState === s_req_idle && isExeCmd)

  // global FSM to control the config and execute flow
//  val isCfgCmdValid = (cfgReqState === s_req_cache)
//  val isExeCmdValid = (exeReqState === s_req_cache)
  val cmd = io.core.req.bits.cmd
//  val cfg_rs1 = cmd.rs1.asTypeOf(new CfgRs1)
  val cfg_rs2 = cmd.rs2.asTypeOf(new CfgRs2)
  val exe_rs1 = cmd.rs1.asTypeOf(new ExeRs1)
  val exe_rs2 = cmd.rs2.asTypeOf(new ExeRs2)
  val isCfgCmd = cmd.inst.funct === CFG_CMD
  val isExeCmd = cmd.inst.funct === EXE_CMD
  io.core.req.ready := ((state === s_idle) && isCfgCmd) || ((state === s_exe_wait) && isExeCmd)
  switch(state){
    is(s_idle){
      when(io.core.req.valid && isCfgCmd){
        state := s_cfg_run
        cfg_base_addr := cfg_rs2.cfg_base_addr
        cfg_num := cfg_rs2.cfg_num
//        loop_cycles := cfg_rs2.loop_cycles
//        ii := cfg_rs2.ii
        id := io.core.req.bits.id
        cfg_start := true.B
      }
    }
    is(s_cfg_run){
      cfg_start := false.B
      when(!cfgCtrl.io.done){ // indicate config is running
        state := s_cfg_resp
      }
    }
    is(s_cfg_resp){
      when(io.core.resp.fire){
        state := s_exe_wait
      }
    }
    is(s_exe_wait){
      when(io.core.req.valid && isExeCmd){
        state := s_exe_run
        id := io.core.req.bits.id
//        exe_cycles := exe_rs1.exe_cycles
        iob_ens := exe_rs1.iob_ens
        exe_start := true.B
      }
    }
    is(s_exe_run){
      exe_start := false.B
      ex_enable := true.B
      when(!fgra.io.done){ // indicate execution is running
        state := s_exe_resp
      }
    }
    is(s_exe_resp){
      when(io.core.resp.fire){
        state := s_idle
        ex_enable := false.B
      }
    }
  }

  io.core.resp.valid := (state === s_cfg_resp && cfgCtrl.io.done) || (state === s_exe_resp && fgra.io.done)
  io.core.resp.bits.success := true.B
  io.core.resp.bits.id := id

  cfgCtrl.io.start := cfg_start
  fgra.io.start := exe_start
  fgra.io.en := ex_enable
  cfgCtrl.io.base_addr := cfg_base_addr
  cfgCtrl.io.cfg_num := cfg_num
//  fgra.io.ii := ii
//  fgra.io.cycles := loop_cycles
//  fgra.io.exe_cycles := exe_cycles
  fgra.io.iob_ens := iob_ens

}

