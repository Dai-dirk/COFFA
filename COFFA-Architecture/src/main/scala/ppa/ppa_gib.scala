//package fgramem.ppa
//
//import fgramem.dsa.GIB.{getOPin2IPinConnect, getOPin2TrackConnect, getTrack2IPinConnect, getTrack2TrackConnect}
//
//import scala.collection.mutable
//import scala.collection.mutable.{ListBuffer, Map}
//
//object ppa_gib {
//
//
//  def getMux(numTrack: Int, diagPinConect: Boolean, numIOPinList: List[Int], fcList: List[Int], trackDirections: ListBuffer[Int]): List[Int] = {
//    val totalconnect = (getTrack2TrackConnect(numTrack,trackDirections) ++ getOPin2TrackConnect(numTrack, fcList(1), List(numIOPinList(1), numIOPinList(3), numIOPinList(5), numIOPinList(7)) ,trackDirections)++
//      getTrack2IPinConnect(numTrack, fcList(0), List(numIOPinList(0), numIOPinList(2), numIOPinList(4), numIOPinList(6)) ,trackDirections)++
//      getOPin2IPinConnect(fcList(2), diagPinConect, numIOPinList))
//    val totalsink = ListBuffer[Seq[Int]]()
//    totalconnect.map{ins => totalsink.append(ins.tail.tail)}
//    val totalsinkdst = totalsink.distinct
//    val MuxList = totalsinkdst.map{dst =>totalsink.count( _== dst) }.toList
//    MuxList
//  }
//
//  def getgibarea(numTrack: Int, diagPinConect: Boolean, num_iopin_list : Map[String,Int], connect_flexibility:mutable.Map[String, Int] ,track_reged :Boolean, trackDirections: ListBuffer[Int]): Double = {
//
//    val numIOPinMap = num_iopin_list
//    val nNWi = numIOPinMap("ipin_nw")  // number of the PE input pins on the NORTHWEST side of the GIB
//    val nNWo = numIOPinMap("opin_nw")  // number of the PE output pins on the NORTHWEST side of the GIB
//    val nNEi = numIOPinMap("ipin_ne")  // number of the PE input pins on the NORTHEAST side of the GIB
//    val nNEo = numIOPinMap("opin_ne")  // number of the PE output pins on the NORTHEAST side of the GIB
//    val nSEi = numIOPinMap("ipin_se")  // number of the PE input pins on the SOUTHEAST side of the GIB
//    val nSEo = numIOPinMap("opin_se")  // number of the PE output pins on the SOUTHEAST side of the GIB
//    val nSWi = numIOPinMap("ipin_sw")  // number of the PE input pins on the SOUTHWEST side of the GIB
//    val nSWo = numIOPinMap("opin_sw")  // number of the PE output pins on the SOUTHWEST side of the GIB
//    val numIOPinList = List(nNWi, nNWo, nNEi, nNEo, nSEi, nSEo, nSWi, nSWo)
//
//    val fcMap = connect_flexibility
//    val fci  = fcMap("num_itrack_per_ipin")     // ipin-itrack connection flexibility, connected track number, 2n
//    val fco  = fcMap("num_otrack_per_opin")     // opin-otrack connection flexibility, connected track number, 2n
//    val fcio = fcMap("num_ipin_per_opin")       // opin-ipin  connection flexibility, connected ipin number, 2n
//    val fcList = List(fci, fco, fcio)
//
//
//    val muxlist = getMux(numTrack, diagPinConect, numIOPinList, fcList,trackDirections).sortBy(ins => ins)
//    var area : Double = 0
//    var index =0
//    var rate : Double= 1
//
//    muxlist.map{mux => {
//      if(index == area_par.cycle){
//        index = 0
//        rate = rate*(1 - area_par.reduce_rate)
//      }else {
//        index += 1
//      }
//      area = area + area_par.area_mux32_map(mux)*rate
//    }}
//    if(track_reged) {
//      area = area + muxlist.size * area_par.area_regnxt32
//    }
//    val cfgsBit = trackDirections.count( ins => ins == 1)*numTrack
//    area = area + cfgsBit*area_par.area_cfgpre32
//    println("gib area :" + area)
//    area
//  }
//}
//
