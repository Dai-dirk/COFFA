//package fgramem.ppa
//
//import chisel3.util.log2Ceil
//
//object ppa_iob {
//  def getiobarea(innum : Int , outnum: Int): Double ={
//    val muxarea = outnum*area_par.area_mux32_map(innum)
//    val cfgarea = outnum*log2Ceil(innum)*area_par.area_cfgpre32
//    val area = muxarea + cfgarea
//    println("iob area :" + area)
//    area
//  }
//
//}
//
