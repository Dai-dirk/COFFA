package fgramem

import chisel3._
import chisel3.util._
import firrtl.Utils.True
import freechips.rocketchip.tile._
import org.chipsalliance.cde.config._
import freechips.rocketchip.diplomacy._
import fgramem.spec._

import java.io.File

object FusionParam {
  val dumpSpec : Boolean = false
  val loadSpec : Boolean = true
  val dumpOperationSet : Boolean = true
  val dumpADG : Boolean = true
  val rootDirPath = (new File("")).getAbsolutePath()
  val fusion_spec_filename = rootDirPath + "/generators/fusion_mem/fgra-mg/src/main/resources/fusion_spec.json"
  val operation_set_filename = rootDirPath + "/generators/fusion_mem/fgra-mg/src/main/resources/operations.json"
  val fgra_adg_filename = rootDirPath + "/generators/fusion_mem/fgra-mg/src/main/resources/fgra_adg.json"
}

class FusionMem(opcodes: OpcodeSet)(implicit p: Parameters) extends LazyRoCC(opcodes, nPTWPorts = 1) {
  import FusionParam._
  if(dumpSpec){ FusionSpec.dumpSpec(fusion_spec_filename) }
  if(loadSpec){ FusionSpec.loadSpec(fusion_spec_filename) }
  FusionSpec.attrs("dumpOperationSet") = dumpOperationSet
  if(dumpOperationSet){ FusionSpec.attrs("operation_set_filename") = operation_set_filename }
  FusionSpec.attrs("dumpADG") = dumpADG
  if(dumpADG){ FusionSpec.attrs("fgra_adg_filename") = fgra_adg_filename }
  // partition size
  // val lgMaxPartition = log2Ceil(FusionSpec.attrs("fgra_iob_sram_banks_coalesce").asInstanceOf[Int])
  val lgMaxPartition = math.min(3, log2Ceil(FusionSpec.attrs("fgra_iob_sram_banks_coalesce").asInstanceOf[Int]))//@yuan: for test hw
  assert(lgMaxPartition <= 3, "[FGRA] The maximum Nb should less than 8." )
  // println("===================lgMaxPartition:===========: " + lgMaxPartition)
  // scratchpad banks used for IOB
  val lgSizeSpadBank = FusionSpec.attrs("spad_bank_lg_size").asInstanceOf[Int]
  val nSpadBanks = FusionSpec.attrs("spad_num_banks").asInstanceOf[Int]
  // scratchpad block used for Config
  val lgSizeSpadCfg = FusionSpec.attrs("spad_cfg_lg_size").asInstanceOf[Int]
  val cfgSpadBanks = {
    if(lgSizeSpadCfg <= lgSizeSpadBank) 1
    else 1 << (lgSizeSpadCfg - lgSizeSpadBank)
  }
  val spadAddrWidth = lgSizeSpadBank + log2Ceil(nSpadBanks+cfgSpadBanks) // in bytes
  val lgMaxDataLen = spadAddrWidth
  val spadAddrNum = FusionSpec.attrs("spad_addr_num").asInstanceOf[Int]
  val spadDataWidth = FusionSpec.attrs("spad_data_width").asInstanceOf[Int]
  val fgraDataWidth = FusionSpec.attrs("fgra_data_width").asInstanceOf[Int]
  val hasMask = false // spadDataWidth != fgraDataWidth
  val idWidth = FusionSpec.attrs("id_width").asInstanceOf[Int]
  val nReqInflight = FusionSpec.attrs("dma_num_req_in_flight").asInstanceOf[Int]
  val maxLgSizeTL = FusionSpec.attrs("dma_lg_max_burst_size").asInstanceOf[Int]
  val nWaysOfTLB = FusionSpec.attrs("tlb_num_ways").asInstanceOf[Int]
  val useSharedTLB = FusionSpec.attrs("tlb_is_shared").asInstanceOf[Boolean]
//  val cmdQueDepth = FusionSpec.attrs("rs_cmd_queue_depth").asInstanceOf[Int]
  val loadQueDepth = FusionSpec.attrs("rs_load_queue_depth").asInstanceOf[Int]
  val storeQueDepth = FusionSpec.attrs("rs_store_queue_depth").asInstanceOf[Int]
  val exeQueDepth = FusionSpec.attrs("rs_exe_queue_depth").asInstanceOf[Int]
  val streamQueDepth = FusionSpec.attrs("ls_stream_queue_depth").asInstanceOf[Int]
  
  override lazy val module = new FusionModuleImp(this)
  val dma_node = LazyModule(new DMAController(lgMaxDataLen, spadDataWidth, hasMask, idWidth, nReqInflight, maxLgSizeTL, nWaysOfTLB, useSharedTLB))
  override val tlNode = dma_node.id_node
//  tlNode :=* dma_node.id_node
}



class FusionModuleImp(outer: FusionMem)(implicit p: Parameters) extends LazyRoCCModuleImp(outer)
  with HasCoreParameters {
  import outer._

  val reservation = Module(new ReservationStation(loadQueDepth, storeQueDepth, exeQueDepth, idWidth))
//  val reservation = Module(new ReservationStationOoO(4, loadQueDepth, storeQueDepth, exeQueDepth, nSpadBanks, lgSizeSpadBank, idWidth))
  val loader = Module(new LoadController(spadAddrWidth, spadAddrNum, lgMaxPartition, lgMaxDataLen, spadDataWidth, fgraDataWidth, hasMask, idWidth, streamQueDepth, loadQueDepth/2))
  // val loader = Module(new LoadController(spadAddrWidth, spadAddrNum, lgMaxPartition, lgMaxDataLen, spadDataWidth, hasMask, idWidth, streamQueDepth, loadQueDepth/2))
  val storer = Module(new StoreController(spadAddrWidth, lgMaxPartition, lgMaxDataLen, spadDataWidth, fgraDataWidth, hasMask, idWidth, streamQueDepth))
  // val storer = Module(new StoreController(spadAddrWidth, lgMaxPartition, lgMaxDataLen, spadDataWidth, hasMask, idWidth, streamQueDepth))
  val spad = Module(new Scratchpad(spadAddrWidth, spadAddrNum, lgMaxDataLen, spadDataWidth, hasMask, idWidth, nSpadBanks, lgSizeSpadBank, lgSizeSpadCfg, fgraDataWidth, lgMaxPartition))
  val fgra = Module(new FGRAController(FusionSpec.attrs))
  val dmaCtrl = outer.dma_node.module

  io.busy := reservation.io.busy
  io.resp <> reservation.io.resp
  reservation.io.cmd <> io.cmd
  reservation.io.ld <> loader.io.core
  reservation.io.st <> storer.io.core
  reservation.io.ex <> fgra.io.core
  loader.io.dma <> dmaCtrl.io.read
  loader.io.spad <> spad.io.write
  storer.io.dma <> dmaCtrl.io.write
  storer.io.spad <> spad.io.read
  fgra.io.srams_iob <> spad.io.srams
  fgra.io.sram_cfg <> spad.io.sram_last
  io.ptw <> dmaCtrl.io.ptw
  dmaCtrl.io.exp.foreach(_.flush := reservation.io.flush)
  io.interrupt := dmaCtrl.io.exp.map(_.interrupt).reduce(_ || _)
  io.mem.req.valid := false.B

}

