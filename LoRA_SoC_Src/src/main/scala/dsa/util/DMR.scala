package fgramemfp.dsa

import chisel3._
import chisel3.util._
import scala.collection.mutable

/** DMR Mode
 * 0: register mode, no acc
 * 1: accumulative mode, periodic initialization and accumulation,
 *    res = f(op), WI = II * Stride
 * 2: conditionally accumulative mode, periodic initialization and irregular conditional accumulation,
 *    res = f(op, en), WI = II
 * 3: conditionally initial and accumulative mode, irregular conditional initialization and accumulation,
 *    res = f(op, en, init), WI = II
 * 4: conditionally initial and accumulative mode, irregular conditional initialization and accumulation, with dual initial values
 *    res = f(op, op1, en, init), WI = II
 * 5: initialization and selection
 *    res = f(op, op1), WI = II
 */

/** Controlled Register with Affine Access (Write) Pattern
 *
 * @param width          register data width
 * @param numIn          number of inputs
 * @param lgMaxWI        log2(max writing Interval)
 * @param lgMaxLat       log2(max starting latency)
 * @param lgMaxCycles    log2(max writing cycles)
 * @param lgMaxRepeats   log2(max repeat number)
 */
class CtrlReg(width: Int, lgMaxWI: Int, numIn: Int, lgMaxLat: Int, lgMaxCycles: Int, lgMaxRepeats: Int) extends Module {
  val cfgWidth = width + lgMaxWI + lgMaxLat + lgMaxCycles + lgMaxRepeats + 2 // width : Initialized value; 1 : if accept io.in at the first cycle; @yuan: if the enable signal should be delayed 1 cycle
  val io = IO(new Bundle {
    val start = Input(Bool())   // pulse signal, should be valid before latency 0, namely -1
    val mode = Input(UInt(3.W)) // 0: no acc, in -> reg -> out; 1: acc; 2: conditional acc; 3: conditional init and acc, 4: conditional dual init and acc； 5： init select; 6: conditional init select
    val config = Input(UInt(cfgWidth.W))
    val en = Input(Bool())  // acc enable
    val init = Input(Bool()) // initialize reg value
    val in = Input(Vec(numIn, UInt(width.W)))
    val out = Output(Vec(2, UInt(width.W)))
  })

  val valueReg = RegInit(0.U(width.W))
  val valueRegAcc = RegInit(0.U(width.W))
  val secondIn = {
    if(numIn > 1) io.in(1)
    else 0.U
  }

  // Config elements
  // [name, (id, high-bit, low-bit)]
  val cfg_idx: mutable.Map[String, (Int, Int, Int)] = mutable.Map()
  // io.config should keep constant during io.en is true
  var offset = 0
  var id = 0
  val initVal = io.config(width+offset-1, offset)//累加的初始值
  cfg_idx += "InitVal" -> (id, width+offset-1, offset)
  offset += width
  id += 1
  val WI = io.config(lgMaxWI+offset-1, offset)
  cfg_idx += "WI" -> (id, lgMaxWI+offset-1, offset)
  offset += lgMaxWI
  id += 1
  val latency = io.config(lgMaxLat+offset-1, offset) // the latency of starting input or output
  cfg_idx += "Latency" -> (id, lgMaxLat+offset-1, offset)
  offset += lgMaxLat
  id += 1
  val cycles = io.config(lgMaxCycles+offset-1, offset)
  cfg_idx += "Cycles"-> (id, lgMaxCycles+offset-1, offset)
  offset += lgMaxCycles
  id += 1
  val repeats = io.config(lgMaxRepeats+offset-1, offset)
  cfg_idx += "Repeats"-> (id, lgMaxRepeats+offset-1, offset)
  offset += lgMaxRepeats
  id += 1
  val skipFirst = io.config(offset, offset).asBool // should be true if mode = 3 and mode = 5 and mode = 6
  cfg_idx += "SkipFirst"-> (id, offset, offset)
  offset += 1
  id += 1
  val delayEn = io.config(offset, offset).asBool // should be true if mode = 3 and mode = 5 and mode = 6
  cfg_idx += "delayEn" -> (id, offset, offset)
  offset += 1
  id += 1

  val s_idle :: s_pre_lat :: s_data :: Nil = Enum(3)
  val state = RegInit(s_idle)
  val wiCnt = RegInit(0.U(lgMaxWI.W))
  val latCnt = RegInit(0.U(lgMaxLat.W))
  val cycleCnt = RegInit(0.U(lgMaxCycles.W))
  val repeatCnt = RegInit(0.U(lgMaxRepeats.W))
  val wiEnd = (wiCnt+1.U >= WI)
  val cycleCntEnd = (cycleCnt+1.U >= cycles) //
  val repeatCntEnd = (repeatCnt+1.U >= repeats)

  switch(state){
    is(s_idle){
      latCnt := 0.U
      when(io.start && latency === 0.U){
        state := s_data
      }.elsewhen(io.start && latency =/= 0.U){
        state := s_pre_lat
      }
    }
    is(s_pre_lat){ // the latency before starting to write
      when(latCnt + 1.U >= latency){
        state := s_data
      }
      latCnt := latCnt + 1.U
    }
    is(s_data){
      when(repeatCntEnd && cycleCntEnd && wiEnd){
        state := s_idle
      }
    }
  }

  val launch = (state === s_data)
  when(state === s_idle){
    wiCnt := 0.U
  }.elsewhen(launch){
    wiCnt := Mux(wiEnd, 0.U, wiCnt+1.U)
  }

  when(state === s_idle){
    cycleCnt := 0.U
  }.elsewhen(launch && wiEnd){
    cycleCnt := Mux(cycleCntEnd, 0.U, cycleCnt + 1.U)
  }

  when(state === s_idle){
    repeatCnt := 0.U
  }.elsewhen(launch && wiEnd && cycleCntEnd){
    repeatCnt := repeatCnt + 1.U
  }

  //  switch(io.mode){
  //    is(0.U){ // bypass
  //      valueReg := io.in
  //    }
  //    is(1.U){ // acc
  //      when(launch && wiCnt === 0.U){
  //        when(cycleCnt === 0.U && skipFirst){
  //          valueReg := initVal
  //        }.otherwise{
  //          valueReg := io.in
  //        }
  //      }
  //      when(state === s_idle){
  //        valueRegAcc := initVal
  //      }.elsewhen(launch && wiCnt === 0.U){
  //        when((cycleCnt === 0.U && skipFirst) || (cycleCntEnd && !skipFirst)){
  //          valueRegAcc := initVal
  //        }.otherwise{
  //          valueRegAcc := io.in
  //        }
  //      }
  //    }
  //    is(2.U) { // conditional acc
  //      when(launch && wiCnt === 0.U){
  //        when(cycleCnt === 0.U && skipFirst){
  //          valueReg := initVal
  //        }.elsewhen(io.en){
  //          valueReg := io.in
  //        }
  //      }
  //      when(state === s_idle){
  //        valueRegAcc := initVal
  //      }.elsewhen(launch && wiCnt === 0.U){
  //        when((cycleCnt === 0.U && skipFirst) || (cycleCntEnd && !skipFirst)){
  //          valueRegAcc := initVal
  //        }.elsewhen(io.en){
  //          valueRegAcc := io.in
  //        }
  //      }
  //    }
  //    is(3.U) { // conditional init acc
  //      when(launch && wiCnt === 0.U){
  //        when(io.init){
  //          valueReg := initVal
  //        }.elsewhen(io.en){
  //          valueReg := io.in
  //        }
  //      }
  //      when(state === s_idle){
  //        valueRegAcc := initVal
  //      }.elsewhen(launch && wiCnt === 0.U){
  //        when(io.init){
  //          valueRegAcc := initVal
  //        }.elsewhen(io.en){
  //          valueRegAcc := io.in
  //        }
  //      }
  //    }
  //  }

//  val init = ((io.mode === 3.U || io.mode === 4.U) && io.init) || (io.mode =/= 3.U && ((cycleCnt === 0.U && skipFirst) || (cycleCntEnd && !skipFirst)))
  val dualInit = (io.mode === 4.U && io.init) || (io.mode === 6.U)
  val initSel = io.mode === 5.U
  val en = (io.mode === 1.U) || (io.mode > 1.U && io.en) || initSel
  val realen = Wire(UInt(1.W))
  when(delayEn){
    realen := RegNext(en)
  }.otherwise{
    realen := en
  }
  val init = ((io.mode === 3.U || io.mode === 4.U || io.mode === 6.U) && io.init && skipFirst ) || ((io.mode =/= 3.U ) && ((cycleCnt === 0.U && (skipFirst || io.mode >= 5.U)) || (cycleCnt === 0.U && !skipFirst && !en )))
//  val FirstFlag = RegInit(0.U(1.W))
//  when(!init && (realen === true.B) && !FirstFlag) {
//    FirstFlag := 1.U
//  }.elsewhen(cycleCntEnd) {
//    FirstFlag := 0.U
//  }
//  val DMRen = Wire(UInt(1.W))
//  when(FirstFlag === true.B) {
//    DMRen := en
//  }.otherwise {
//    DMRen := realen
//  }
//  val realInit = init || !realFirst || (!init && (realFirst === true.B) && skipFirst)
  when(io.mode === 0.U){
    valueReg := io.in(0)
  }.elsewhen(launch && wiCnt === 0.U){ // the first is acc value
    when(init){
      when(dualInit){
        valueReg := secondIn
      }.elsewhen(initSel){
        valueReg := secondIn
      }.otherwise{
        valueReg := initVal
      }

    }.elsewhen(realen === 1.U){
      valueReg := io.in(0)
    }
  }

  //  when(io.mode === 0.U || (launch && wiCnt === 0.U && cycleCnt > 0.U)){
  //    valueReg := io.in
  //  }.elsewhen(launch && wiCnt === 0.U && cycleCnt === 0.U && !skipFirst){ // the first is acc value
  //    valueReg := io.in
  //  }.elsewhen(launch && wiCnt === 0.U && cycleCnt === 0.U && skipFirst){
  //    valueReg := initVal
  //  }
  //这里对应的是执行cycle次累加途中的情况
  val initAcc = ((io.mode === 3.U || io.mode === 4.U) && io.init && skipFirst ) || ((io.mode =/= 3.U && io.mode =/= 6.U) && ((cycleCnt === 0.U && skipFirst) || (cycleCntEnd && !skipFirst)))
  val dualInitAcc = (io.mode === 4.U && io.init && !cycleCntEnd)
  when(state === s_idle){
    valueRegAcc := initVal
  }.elsewhen(launch && wiCnt === 0.U){
    when(initAcc){
      when(dualInitAcc) {
        valueRegAcc := secondIn
      }.otherwise {
        valueRegAcc := initVal
      }
    }.elsewhen(realen === 1.U){
      valueRegAcc := io.in(0)
    }
  }
  //  when(state === s_idle){
  //    valueRegAcc := initVal
  //  }.elsewhen(launch && wiCnt === 0.U && cycleCnt === 0.U && skipFirst){
  //    valueRegAcc := initVal
  //  }.elsewhen(launch && wiCnt === 0.U && cycleCntEnd && !skipFirst){
  //    valueRegAcc := initVal
  //  }.elsewhen(launch && wiCnt === 0.U){
  //    valueRegAcc := io.in
  //  }
    val realRegAcc = Wire(UInt(width.W))
    when(io.mode === 3.U && io.init && !skipFirst){
      realRegAcc := initVal
    }.otherwise{
      realRegAcc := valueRegAcc
    }

  io.out(0) := valueReg
  io.out(1) := realRegAcc
}


/** Dual-mode Register supporting Affine Access (Write) Pattern and simple reg
 *
 * @param width          register data width
 * @param aluOutFG       the number of ALU fine-grain outputs
 * @param isAffine       is Affine Access (Write) Pattern
 * @param isDualIn       has 2 DMR Inputs
 * @param lgMaxWI        log2(max writing Interval)
 * @param lgMaxLat       log2(max starting latency)
 * @param lgMaxCycles    log2(max writing cycles)
 * @param lgMaxRepeats   log2(max repeat number)
 */
class DualModeReg(width: Int, isAffine: Boolean, isDualIn: Boolean,  lgMaxWI: Int, lgMaxLat: Int, lgMaxCycles: Int, lgMaxRepeats: Int) extends Module {
  val cfgWidth = {
    if(isAffine){
      width + lgMaxWI + lgMaxLat + lgMaxCycles + lgMaxRepeats + 2 // width : Initialized value; 1 : if accept io.in at the first cycle; @yuan: if the enable signal should be delayed 1 cycle
    }else{
      0
    }
  }
//  println("lgMaxCycles: " + lgMaxCycles + "lgMaxRepeats: " + lgMaxRepeats )
  val numIn = {if(isDualIn) 2 else 1}
//  println("numIn: " + numIn)
  val numOut = { if(isAffine) 2 else 1 }
  val io = IO(new Bundle {
    val start = Input({if(isAffine) Bool() else UInt(0.W)}) // pulse signal, should be valid before latency 0, namely -1
    val mode = Input(UInt({if(isAffine) 3 else 0}.W))
    val config = Input(UInt(cfgWidth.W))
    val en = Input({if(isAffine) Bool() else UInt(0.W)})  // acc enable
    val init = Input({if(isAffine) Bool() else UInt(0.W)}) // initialize reg value
    val in = Input(Vec(numIn, UInt(width.W)))
    val out = Output(Vec(numOut, UInt(width.W)))
  })

  // Config elements
  // [name, (id, high-bit, low-bit)]
  val cfg_idx: mutable.Map[String, (Int, Int, Int)] = mutable.Map()

  if(isAffine){
    val acr = Module(new CtrlReg(width, lgMaxWI, numIn, lgMaxLat, lgMaxCycles, lgMaxRepeats))
    acr.io.start := io.start
    acr.io.mode := io.mode
    acr.io.config := io.config
    acr.io.en := io.en
    acr.io.init := io.init
    acr.io.in(0) := io.in(0)
    if(isDualIn){
      acr.io.in(1) := io.in(1)
    }
    io.out := acr.io.out
    cfg_idx ++= acr.cfg_idx
  }else{
    io.out(0) := RegNext(io.in(0))
  }

}


// object VerilogGen extends App {
////   (new chisel3.stage.ChiselStage).emitVerilog(new CtrlReg(32, 16, 8, 16, 16),args)
//   (new chisel3.stage.ChiselStage).emitVerilog(new DualModeReg(32, true, 16, 8, 16, 16),args)
// }
