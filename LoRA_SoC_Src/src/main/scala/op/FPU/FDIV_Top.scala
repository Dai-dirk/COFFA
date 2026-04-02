// package fgramemfp.op

// import chisel3._
// import chisel3.util._
// import fgramemfp.common.CompileMacroVar._
// import fgramemfp.op.FPU._

// //@yuan: adding counters to control the float-point divider to execute correctly
// class FDIVTop(val expWidth: Int, val precision: Int, lgMaxII: Int, latency: Int) extends Module {
//   val io = IO(new Bundle() {
//     val a, b = Input(UInt((expWidth + precision).W))
//     val rm = Input(UInt(3.W))
//     val mode = Input(UInt(1.W))
//     val en = Input(Bool())
//     val II = Input(UInt(lgMaxII.W))
//     val result = Output(UInt((expWidth + precision).W))
//   })
//   if (TARGET_TYPE == 5) {
//     class DW_lp_piped_fp_div_inst  extends BlackBox(Map(
//       "sig_width" -> (precision - 1) ,
//       "exp_width " -> expWidth,
//       "ieee_compliance " -> 0,
//       "faithful_round  " -> 0,
//       "op_iso_mode   " -> 0,
//       "id_width    " -> 8,
//       "in_reg     " -> 0,
//       "stages " -> (latency + 1),
//       "out_reg" -> 0,
//       "no_pm " -> 1,
//       "rst_mode " -> 0,
//     )) {
//       val io = IO(new Bundle {
//         val clk = Input(Clock())
//         val rst_n = Input(Bool())
//         val rnd = Input(UInt(3.W))
//         val a = Input(UInt((expWidth + precision).W))
//         val b = Input(UInt((expWidth + precision).W))
//         val z = Output(UInt((expWidth + precision).W))
//         val launch = Input(Bool())
//         val status = Output(UInt(8.W))
//         val launch_id = Input(UInt(8.W))
//         val pipe_full = Output(Bool())
//         val pipe_ovf = Output(Bool())
//         val accept_n = Input(Bool())
//         val arrive = Output(Bool())
//         val arrive_id = Output(UInt(8.W))
//         val push_out_n = Output(Bool())
//         val pipe_census = Output(UInt(2.W))
//       })
//     }

//     val div = Module(new DW_lp_piped_fp_div_inst )
//     div.io.clk := clock
//     div.io.rst_n := !reset.asBool
//     div.io.rnd := 0.U
//     div.io.a := io.a
//     div.io.b := io.b
//     io.result := div.io.z
//     div.io.launch := io.en
//     div.io.accept_n := !io.en
//     div.io.launch_id := 0.U
//   }else {
//     //instance the float-point divider
//     val fdiv = Module(new FDIV(expWidth, precision))
//     fdiv.io.a := io.a
//     fdiv.io.b := io.b
//     fdiv.io.specialIO.isSqrt := io.mode
//     fdiv.io.specialIO.in_valid := io.en
//     fdiv.io.specialIO.out_ready := io.en
//     fdiv.io.rm := io.rm

//     val IICnt = RegInit(1.U(lgMaxII.W))
//     val IICntEnd = (IICnt + 2.U) >= io.II
//     val s_idle :: s_count :: s_kill :: Nil = Enum(3)
//     val state = RegInit(s_idle)
//     val isKill = WireInit(false.B)

//     //it's noted that the minimum value of II is 11
//     switch(state) {
//       is(s_idle) {
//         IICnt := 1.U
//         when(io.en) {
//           state := s_count
//         }
//         isKill := false.B
//       }
//       is(s_count) {
//         isKill := false.B
//         when(io.en && IICntEnd) {
//           state := s_kill
//         }
//         IICnt := IICnt + 1.U
//       }
//       is(s_kill) {
//         isKill := true.B
//         state := s_idle
//       }
//     }
//     fdiv.io.specialIO.kill := isKill
//     io.result := fdiv.io.result
//   }
// }

// // object VerilogFDIVTopGen extends App {
// //   (new chisel3.stage.ChiselStage).emitVerilog(new FDIVTop(8,24, 5), args)
// // }