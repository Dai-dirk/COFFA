package fgramemfp.dsa

import chisel3._
import chisel3.util._
import scala.math.pow


/** Delay-configurable Pipe
 * 
 * @param width     data width
 * @param maxDelay  max delay cycles
 */
class DelayPipe(width: Int, maxDelay: Int) extends Module {
  val cfgWidth = log2Ceil(maxDelay+1)
  val io = IO(new Bundle {
    val en = Input(Bool())
    val config = Input(UInt(cfgWidth.W)) // delay cycles
    val in = Input(UInt(width.W))
    val out = Output(UInt(width.W))
  })

  val regs = RegInit(VecInit(Seq.fill(maxDelay+1){0.U(width.W)}))
  val wptr = RegInit(0.U(cfgWidth.W))   // write pointer
  val rptr = RegInit(0.U(cfgWidth.W))   // read pointer
  
  when(io.en && (wptr < maxDelay.U)){
    wptr := wptr+1.U
  }.otherwise{
    wptr := 0.U
  }

  when(wptr+1.U >= io.config){
    rptr := wptr + 1.U - io.config
  }.otherwise{
    rptr := (2 + maxDelay).U + wptr - io.config
  }


  when(io.en && (io.config > 0.U)){
    regs(wptr) := io.in
  }

  val cnt = RegInit(0.U(cfgWidth.W)) // counter
  when(!io.en){
    cnt := 0.U
  }.elsewhen(cnt < io.config){
    cnt := cnt + 1.U
  }

  when(io.en && (0.U === io.config)){
    io.out := io.in  // delay = 0
  }.elsewhen(io.en && (cnt === io.config)){
    io.out := regs(rptr)
  }.otherwise{ // out 0 before the data is written into the position
    io.out := 0.U
  }

}


/** Reconfigurable Delay Pipe shared by N pairs of IO ports
  *
  * @param width     data width
  * @param maxDelay  total max delay cycles
  * @param num       pairs of IO ports
  */
class SharedDelayPipe(width: Int, maxDelay: Int, num: Int) extends Module {
  val cfgWidth = log2Ceil(maxDelay + 1)

  val io = IO(new Bundle {
    val en     = Input(Bool())
    val config = Input(UInt((num * cfgWidth).W)) // packed per-port delays
    val in     = Input(Vec(num, UInt(width.W)))
    val out    = Output(Vec(num, UInt(width.W)))
  })

  // Params/state
  val regNum   = maxDelay + num
  val ptrWidth = log2Ceil(regNum)
  val regs     = RegInit(VecInit(Seq.fill(regNum)(0.U(width.W))))
  val wptr     = RegInit(VecInit(Seq.fill(num)(0.U(ptrWidth.W))))
  val rptr     = RegInit(VecInit(Seq.fill(num)(0.U(ptrWidth.W)))) // <-- rptr is a Reg

  // Unpack config
  val cfg = Wire(Vec(num, UInt(cfgWidth.W)))
  for (i <- 0 until num) {
    cfg(i) := io.config((i + 1) * cfgWidth - 1, i * cfgWidth)
  }

  // Your existing offset for wptr load when !io.en
  val offset = Wire(Vec(math.max(0, num - 1), UInt(cfgWidth.W)))
  if (num > 1) {
    offset(0) := cfg(1) + 1.U
    for (i <- 1 until num - 1) {
      offset(i) := offset(i - 1) + cfg(i + 1) + 1.U
    }
  }

  // Helper: modulo add/sub (implemented as add with wrap)
  def incMod(x: UInt, mod: Int): UInt 
    = Mux(x === (mod - 1).U, 0.U, x + 1.U)
  def subMod(x: UInt, y: UInt, mod: Int): UInt = {
    val xz = x.zext(); val yz = y.zext(); val mz = mod.U.zext()
    val diff = xz - yz
    Mux(diff >= 0.S, diff.asUInt, (mz + diff).asUInt)(ptrWidth - 1, 0)
  }

  val alignEvent = !io.en

  // Update pointers
  for (i <- 0 until num) {
    when (io.en) {
      // streaming: advance both, preserving phase
      wptr(i) := incMod(wptr(i), regNum)
      rptr(i) := incMod(rptr(i), regNum)
    } .otherwise {
      // align/load phase
      wptr(i) := (if (i == 0) 0.U else offset(i - 1))
      // set rptr to wptr - cfg (one-time subtract, off critical path)
      rptr(i) := subMod(wptr(i), cfg(i), regNum)
    }
  }

  // Writes 
  when (io.en) {
    for (pi <- 0 until num) {
      when (wptr(pi) < regNum.U) { regs(wptr(pi)) := io.in(pi) }
    }
  } .otherwise {
    for (j <- 0 until regNum) { regs(j) := 0.U }
  }

  // Reads: use registered rptr (no wptr to rptr comb path)
  for (i <- 0 until num) {
    val readComb = (0 until regNum).map { j =>
      Mux(rptr(i) === j.U, regs(j), 0.U(width.W))
    }.reduce(_ | _)
    io.out(i) := Mux(io.en, Mux(cfg(i) === 0.U, io.in(i), readComb), 0.U)
  }
}
// class SharedDelayPipe(width: Int, maxDelay: Int, num: Int) extends Module {
//   val cfgWidth = log2Ceil(maxDelay+1)
//   val io = IO(new Bundle {
//     val en = Input(Bool())
//     val config = Input(UInt((num*cfgWidth).W)) // delay cycles
//     val in = Input(Vec(num, UInt(width.W)))
//     val out = Output(Vec(num, UInt(width.W)))
//   })


//   val regNum = maxDelay+num
//   //println("regNum: " + regNum)
//   val ptrWidth = log2Ceil(regNum)
//   val regs = RegInit(VecInit(Seq.fill(regNum){0.U(width.W)}))
//   val wptr = RegInit(VecInit(Seq.fill(num){0.U(ptrWidth.W)}))   // write pointer
//   val rptr = RegInit(VecInit(Seq.fill(num){0.U(ptrWidth.W)}))   // read pointer
// //  val rptr = Wire(Vec(num, UInt(ptrWidth.W)))   // read pointer
//   val config = Wire(Vec(num, UInt(cfgWidth.W)))
//   for(i <- 0 until num){
//     config(i) := io.config((i+1)*cfgWidth-1, i*cfgWidth)
//   }
//   val offset = Wire(Vec(num-1, UInt(cfgWidth.W))) // offset of the write ptr
//   for(i <- 0 until num-1){
//     if(i == 0){
//       offset(i) := config(i+1) + 1.U
//     }else{
//       offset(i) := offset(i-1) + config(i+1) + 1.U
//     }
//   }

//   for(i <- 0 until num){
//     if(i == 0){ //  first pair of IO: wptr increase from 0
//       when(io.en && (wptr(i) < (regNum-1).U)){
//         wptr(i) := wptr(i) + 1.U
//       }.otherwise{
//         wptr(i) := 0.U
//       }
//     }else{
//       when(io.en) {
//         when(wptr(i) < (regNum - 1).U) {
//           wptr(i) := wptr(i) + 1.U
//         }.otherwise {
//           wptr(i) := 0.U
//         }
//       }.otherwise{
//         wptr(i) := offset(i - 1)
//       }
//     }

//     if (i == 0) { //  first pair of IO: wptr increase from 0
//       when(io.en && (rptr(i) < (regNum - 1).U)) {
//         rptr(i) := rptr(i) + 1.U
//       }.elsewhen(io.en){
//         rptr(i) := 0.U
//       }.otherwise {// initialization
//         rptr(i) := regNum.U - config(i)
//       }
//     } else {
//       when(io.en) {
//         when(rptr(i) < (regNum - 1).U) {
//           rptr(i) := rptr(i) + 1.U
//         }.otherwise {
//           rptr(i) := 0.U
//         }
//       }.otherwise {//initialization
//         when(offset(i-1) >= config(i)){
//           rptr(i) := offset(i-1) - config(i)
//         }.otherwise {
//           rptr(i) := regNum.U + offset(i - 1) - config(i)
//         }
//       }
//     }

// //    when(wptr(i) >= config(i)){
// //      rptr(i) := wptr(i) - config(i)
// //    }.otherwise{
// //      rptr(i) := regNum.U + wptr(i) - config(i)
// //    }
//     //    when(wptr(i) + 1.U >= config(i)){
//     //      rptr(i) := wptr(i) + 1.U - config(i)
//     //    }.otherwise{
//     //      rptr(i) := (regNum+1).U + wptr(i) - config(i)
//     //    }

//     when(io.en && (0.U === config(i))){
//       io.out(i) := io.in(i)  // delay = 0
//     }.elsewhen(io.en){
//       io.out(i) := regs(rptr(i))
//     }.otherwise{
//       io.out(i) := 0.U
//     }
//   }

//   //    when(io.en && (io.config(i) > 0.U)){
//   when(io.en){
//     for(i <- 0 until num){
//       regs(wptr(i)) := io.in(i)
//     }
//   }
// }

 object VerilogGen extends App {
   (new chisel3.stage.ChiselStage).emitVerilog(new SharedDelayPipe(32, 4, 3),args)
 }

