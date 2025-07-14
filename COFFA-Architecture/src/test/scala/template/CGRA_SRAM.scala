package fgramem.dsa
import chisel3._
import fgramem.ir._
import fgramem.common.MacroVar._

import scala.collection.mutable
import scala.collection.mutable.ListBuffer

/** CGRA module
 * @param attrs     module attributes
 */ 
class CGRA_SRAM(attrs: mutable.Map[String, Any]) extends Module with IR{
  // CGRA parameters
  val param = FgraParam(attrs)
  import param._

  val SramaddrWidth = spad_bank_lg_size
  val coalesce_Bank = coalesceBanksIOB

  val io = IO(new Bundle{
    // config signals
    val cfg_en   = Input(Bool())
    val cfg_addr = Input(UInt(cfgAddrWidth.W))
    val cfg_data = Input(UInt(cfgDataWidth.W))
//    val ii = Input(UInt(lgMaxII.W)) // Initialization Interval, shared among all IOB
//    val cycles = Input(UInt(lgMaxCycles.W)) // valid in/out cycles, shared among all IOB
    val iob_ens = Input(UInt(numSrams.W)) // enable signals for every IOB
    // computing signals
    val en  = Input(Bool()) // global enable
    val start = Input(Bool()) // pulse signal, should be valid before latency 0, namely -1
    val done = Output(Bool()) // transfer done, keep true until next start
    val hostInterface = Vec(numSrams, new SRAMIO(dataWidth, SramaddrWidth, hasMaskSram))
  })

  //val srams = Vec(numIOB, Flipped(new SRAMIO(dataWidth, addrWidthSram, hasMaskSram)))

    //iob.io.sram <> io.srams(i)
    val fgra = Module(new FGRA(attrs))
    val sram_coalesce_iob = Module(new SRAMCoalesce(dataWidth, SramaddrWidth, hasMaskSram, numSrams, coalesce_Bank))
    fgra.io.cfg_en := io.cfg_en
    fgra.io.cfg_addr := io.cfg_addr
    fgra.io.cfg_data := io.cfg_data
    fgra.io.iob_ens := io.iob_ens
    fgra.io.en := io.en
    fgra.io.start := io.start
    io.done := fgra.io.done
    // CGRA -> coalesce
    fgra.io.srams <> sram_coalesce_iob.io.coal
    for (i <- 0 until numSrams) {
      val SRAM = Module(new TrueDualPortSRAM(dataWidth, SramaddrWidth, hasMaskSram))
      sram_coalesce_iob.io.orig(i) <> SRAM.io.a
      SRAM.io.b <> io.hostInterface(i) 
    }
val cfgRegNum = fgra.cfgRegNum

}
