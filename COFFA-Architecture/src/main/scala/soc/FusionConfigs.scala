package fgramem

import chisel3._
import freechips.rocketchip.tile._
import org.chipsalliance.cde.config._
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.subsystem.SystemBusKey


class WithFusionMem extends Config((site, here, up) => {
  case BuildRoCC => up(BuildRoCC) ++ Seq(
    (p: Parameters) => {
      val fusion = LazyModule(new FusionMem(OpcodeSet.custom0)(p))
      fusion
    }
  )
  case SystemBusKey => up(SystemBusKey).copy(beatBytes = 16)
})
