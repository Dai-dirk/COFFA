//package fgramemfp.ppa
//
//import chisel3.util.log2Ceil
//import fgramemfp.op.OPC
//
//import scala.collection.mutable.ListBuffer
//
//object ppa_gpe {
//
//  def getgpearea(operations :ListBuffer[String] , num_input_per_operand:ListBuffer[Int] ,max_delay :Int) : Double = {
//    val muxarea = num_input_per_operand.map{muxindex => area_par.area_mux32_map(muxindex + 2)}.reduce(_ +_)
//    val delayarea = num_input_per_operand.size * area_par.area_delay32_map(max_delay)
//    val aluarea = operations.foldLeft(0){(a,b)=> a+ area_par.area_alu32_map(b)}
//    val constarea = area_par.area_const32
//    val rfarea = area_par.area_rf32
//
//
//    val constCfgWidth = 32 // constant
//    val aluCfgWidth = log2Ceil(OPC.numOPC) // ALU Config width
//    val rfCfgWidth = 0  // RF
//    val delayCfgWidthEach = log2Ceil(max_delay+1) // DelayPipe Config width
//    val delayCfgWidth = num_input_per_operand.size * delayCfgWidthEach
//    val imuxCfgWidthList = num_input_per_operand.map{ mux => log2Ceil(mux + 2) } // input Muxes
//    val imuxCfgWidth = imuxCfgWidthList.sum
//    val sumCfgWidth = constCfgWidth + aluCfgWidth + rfCfgWidth + delayCfgWidth + imuxCfgWidth
//    val cfgarea = sumCfgWidth* area_par.area_cfgpre32
//
//    val area = muxarea + delayarea +  aluarea + constarea + rfarea + cfgarea
//    println("gpe area :" + area)
//    area
//
//  }
//
//}
