package fgra.dsa

import scala.io.Source
import java.io._
import java.math.BigInteger
import scala.math.BigInt
import chisel3._
import chisel3.util._
import chisel3.assert

import scala.collection.mutable
import scala.collection.mutable.{ArrayBuffer, ListBuffer}
import chisel3.iotesters.{ChiselFlatSpec, Driver, PeekPokeTester}
import chiseltest._
import org.scalatest._
import org.scalatest.flatspec.AnyFlatSpec
import fgra.spec.FGRASpec
import scala.util.control.Breaks._
import fgra.dsa.TrueDualPortSRAM


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//The main object of Architecture simulation
class TestSim extends AnyFlatSpec with ChiselScalatestTester{
  

  //select which benchmark to be simulated
  val benchmark = "test_case/S2_case1"
  val IOFilename = s"../benchmarks/$benchmark/mapped_dfgio.txt"
  val IOFilename2 = s"../benchmarks/$benchmark/cgra_execute.c"
  val IOFilename3 = s"../benchmarks/$benchmark/golden_model.txt"


  val simulationHelper = new SimulationHelper()
  simulationHelper.init(IOFilename, IOFilename2, IOFilename3)
  val outputCycle = simulationHelper.getOutputCycle()
  val inputnum = simulationHelper.getinputnum()
  val outputnum = simulationHelper.getoutputnum()
  val loadnum = simulationHelper.getloadnum()
  val storenum = simulationHelper.getstorenum()
  println("outputCycle: " + outputCycle)
  println("inputnum: " + inputnum)
  println("outputnum: " + outputnum)
  println("loadnum: " + loadnum)
  println("storenum: " + storenum)

  //set the size of data for simulation 
  val dataSize = simulationHelper.storelen.reduce((t1, t2) => Math.max(t1, t2))
  val loaddataSize = simulationHelper.loadlen.reduce((t1, t2) => Math.max(t1, t2))
  val iob_ens = simulationHelper.iob_en
  println("loaddataSize: " + loaddataSize + " dataSize: " + dataSize)

  var inData = Array.ofDim[Int](inputnum,loaddataSize)
  var outPortRefArrays = Array.ofDim[Int](outputnum,dataSize)
  // var loadData = Array.ofDim[Int](loadnum,loaddataSize)
  // var storeRefArrays = Array.ofDim[Int](storenum,dataSize)
  var loadData = new Array[ArrayBuffer[Int]](loadnum)
  var storeRefArrays = new Array[ArrayBuffer[Int]](storenum)
  var results = new Array[Int](outputnum)
  val inputArray = new ArrayBuffer[Tuple2[Int, Int]]()
  val outputArray = new ArrayBuffer[Tuple2[Int, Int]]()
  var loadport = new ArrayBuffer[Int]()
  var storeport = new ArrayBuffer[Int]()
  var loadmap = Map[Int, Array[Int]]()
  var loadaddr = Array.ofDim[Int](loadnum,loaddataSize)
  var loadaddrmap = Map[Int, Array[Int]]()
  var storeaddr = Array.ofDim[Int](storenum,dataSize)
  var storeaddrmap = Map[Int, Array[Int]]()
  var inputMap = Map[Tuple2[Int, Int], Array[Int]]()
  var outputMap = Map[Tuple2[Int, Int], Array[Int]]()
  val loadArray = new ArrayBuffer[Tuple2[Int, Int]]()
  val storeArray = new ArrayBuffer[Tuple2[Int, Int]]()
  var loadMap = Map[Tuple2[Int, Int], Int]()
  var storeMap = Map[Tuple2[Int, Int], Int]()

  for(j <- 0 until inputnum) {
    inData(j) = (0 until dataSize).map(i => scala.util.Random.nextInt(1000)).toArray
   // inData(j) = (0 until dataSize).map(i => i).toArray
    inputArray.append(simulationHelper.inputArray(j))
    inputMap = inputMap ++ Map(inputArray(j) -> inData(j))
  }
  

  for(j <- 0 until loadnum) {
    loadData(j) = simulationHelper.loadDatas(j)
    loadport.append(simulationHelper.loadPorts(j))
  }
  

  for(j <- 0 until storenum) {
    storeport.append(simulationHelper.storePorts(j))
  }

  var refArray = Array[Int]()
  var ref = 0

  var refstoreArray = Array[Int]()
  var refstore = 0

  // the II of mapped DFG, II = 1 for static configuration
  var testII = 1

  for (i <- 0 until loaddataSize) {
    for (j <- 0 until loadnum) {
      loadaddr(j)(i) = simulationHelper.loadaddrArray(j) + i
      loadArray.append(Tuple2(loadport(j), loadaddr(j)(i)))
      loadMap = loadMap ++ Map(Tuple2(loadport(j), loadaddr(j)(i)) -> loadData(j)(i))
    }
  }
  for (i <- 0 until dataSize) {
    for (j <- 0 until storenum) {
      storeRefArrays(j) = simulationHelper.storeDatas(j)
    }    
    for (j <- 0 until storenum) {
      storeaddr(j)(i) = simulationHelper.storeaddrArray(j) + i
      storeArray.append(Tuple2(storeport(j), storeaddr(j)(i)))
      storeMap = storeMap ++ Map(Tuple2(storeport(j), storeaddr(j)(i)) -> storeRefArrays(j)(i))
    }
  }



  for(j <- 0 until loadnum) {
    loadaddrmap = loadaddrmap ++ Map(loadport(j) -> loadaddr(j))
  }
  for(j <- 0 until storenum) {
    storeport.append(simulationHelper.storePorts(j))
    storeaddrmap = storeaddrmap ++ Map(storeport(j) -> storeaddr(j))
  }
  //val outDataRefArrays = Array(refArray)
  val dataWithAddr = simulationHelper.getDataWithAddr(inDataArrays = inData, outDataArrays = storeRefArrays, refDataArrays = outPortRefArrays)
  val outPortRefs = dataWithAddr(0).asInstanceOf[Map[Tuple2[Int, Int], Array[Int]]]

  val storeRefs = dataWithAddr(1).asInstanceOf[Map[Int, Array[Int]]]
  testII = simulationHelper.getii
  
  val parameters = new Parameters(testII)
  parameters.setOutPortRefs(outPortRefs)
  parameters.setStoreRefs(storeMap)
  parameters.setStoreAddr(storeaddrmap)

  //Pass the input data to tester
  parameters.setInputPortData(inputMap)
  parameters.setLoadData(loadMap)
  parameters.setPortCycle(simulationHelper)
  parameters.setLoadAddr(loadaddrmap)
  
  //set the scale of simulated architecture
  val jsonFile = "src/main/resources/fgra_spec.json"
  // TramSpec.dumpSpec(jsonFile)
  // TramSpec.loadSpec(jsonFile)
  FGRASpec.loadSpec(jsonFile)

  ////////chiseltest
  behavior of "CGRA_SRAM"
  it should "work harder" in {
    test(new CGRA_SRAM(FGRASpec.attrs)) {
      c => new BMTester(c, s"../benchmarks/$benchmark/config.bit", parameters, inputnum, outputnum, loadnum, storenum, iob_ens)

    }
  }



}
////////////////////////////////////////////////////////////////////////////////////////////////////////
/** A class which passes the parameters for apptester.
 *
 * @param testII     the targeted II
 */
class Parameters(testII: Int) {

  /** A map between the targeted output port and the expected data array.
   */
  var outPortRefs = Map[Tuple2[Int, Int], Array[Int]]()

  /** A map between the targeted store and the expected data array.
   */
  var storeRefs = Map[Tuple2[Int, Int], Int]()

  /** A map between the targeted input port and the input data array.
   */
  var inputPortData = Map[Tuple2[Int, Int], Array[Int]]()

  /** A map between the targeted load and the load data array.
   */
  var loadPortData = Map[Tuple2[Int, Int], Int]()

  /** The cycle we can obtain the result.
   */
  var outputCycle = 1

  /** A table of the load data's address.
   */
  var loadAddr = Map[Int, Array[Int]]()

  /** A table of the store data's address.
   */
  var storeAddr = Map[Int, Array[Int]]()

//get parameters from simulate main function

  def setPortCycle(simulationHelper: SimulationHelper): Unit = {
    outputCycle = simulationHelper.outputCycle
  }

  /** Set the map between the targeted output port and the expected data array.
   */
  def setOutPortRefs(arg: Map[Tuple2[Int, Int], Array[Int]]): Unit = {
    outPortRefs = arg
  }

  /** Set the table of the load data's address.
   */
  def setLoadAddr(arg: Map[Int, Array[Int]]): Unit = {
    loadAddr = arg
  }

  /** Set the table of the store data's address.
   */
  def setStoreAddr(arg: Map[Int, Array[Int]]): Unit = {
    storeAddr = arg
  }
  /** Set the map between the targeted store and the expected data array.
   */
  def setStoreRefs(arg: Map[Tuple2[Int, Int], Int]): Unit = {
    storeRefs = arg
  }
  
  /** Set the map between the targeted input port and the input data array.
   */
  def setInputPortData(arg: Map[Tuple2[Int, Int], Array[Int]]): Unit = {
    inputPortData = arg
  }

  /** Set the map between the targeted load and the load data array.
   */
  def setLoadData(arg: Map[Tuple2[Int, Int], Int]): Unit = {
    loadPortData = arg
  }
//pass parameters to Apptester  

  /** Get the targeted II.
   */
  def getTestII(): Int = {
    testII
  }

  /** Get the cycle we can obtain the result.
   */
  def getOutputCycle(): Int = {
    outputCycle
  }

  /** Get the map between the targeted output port and the expected data array.
   */
  def getOutPortRefs(): Map[Tuple2[Int, Int], Array[Int]] = {
    outPortRefs
  }

  /** Get the map between the targeted input port and the input data array.
   */
  def getInputPortData(): Map[Tuple2[Int, Int], Array[Int]] = {
    inputPortData
  }

    /** Get the table of the load data's address.
   */
  def getLoadAddr(): Map[Int, Array[Int]] = {
    loadAddr
  }

  /** Get the table of the store data's address.
   */
  def getStoreAddr(): Map[Int, Array[Int]] = {
    storeAddr
  }
  /** Get the map between the targeted store and the expected data array.
   */
  def getStoreRefs(): Map[Tuple2[Int, Int], Int] = {
    storeRefs
  }

  /** Get the map between the targeted load and the load data array.
   */
  def getLoadData(): Map[Tuple2[Int, Int], Int] = {
    loadPortData
  }
}

/** A base class of testers for benchmarks.
 * It help users to test the architecture and benchmark in the specific format of Chisel testers using the Verilator backend.
 *
 * @param c             the top design
 * @param parameters the class which passes the parameters for apptester
 */
class ApplicationTester(c: CGRA_SRAM, parameters: Parameters) extends AnyFlatSpec with ChiselScalatestTester {
  /** Translate a signed Int as unsigned BigInt.
   * @param signedInt the signed Int
   * @return the unsigned BigInt
   */
   // behavior of "c"
  def asUnsignedInt(signedInt: Int): BigInt = (BigInt(signedInt >>> 1) << 1) + (signedInt & 1)

  /** Save data to be loaded in mem.
   *
   */
  def loads: Unit = {
    val loadDataMap = parameters.getLoadData()
    val loadAddrMap = parameters.getLoadAddr()
      //c.io.iob_ens.poke(scala.math.pow(2, ))
      for (port <- loadDataMap.keys) {
        val data = loadDataMap(port)
        val Addr = port._2
        //c.io.iob_ens(port._1).poke(1.B)
        c.io.hostInterface(port._1).en.poke(1.B)
        c.io.hostInterface(port._1).we.poke(1.U)
        c.io.hostInterface(port._1).addr.poke(asUnsignedInt(Addr).U)
        c.io.hostInterface(port._1).din.poke(asUnsignedInt(data).U)
        println("LoadMemID: " + port._1.toString + " Addr: " + Addr.toString + "  inData: " + data.toString)
        c.clock.step(1)
      }
    for (port <- loadDataMap.keys) {
      c.io.hostInterface(port._1).en.poke(0.B)
      //c.io.iob_ens(port._1).poke(0.B)
      c.clock.step(1)
    }
  }

    /** Run.
   *
   * @param testII the targeted II
   */
  def run(testII: Int): Unit = {
    var f = 0
    var times = 0
    while (f < 1) {
      if (c.io.done.peek().toString == "Bool(false)") {
        //print(".")
        c.clock.step(1)
        times = times + 1
        //println("done is: " + c.io.done.peek().toString() + "  ** run time is " + times)
      }
      else {
        f = 2
        println("done is: " + c.io.done.peek().toString() + "  ** run time is " + times)
      }
      if((times%10)==0) {
        print(".")
      }
    }
  }

  /** Check data in mem.
   */
  def checkStore: Unit = {
    val refs = parameters.getStoreRefs()
    val storeAddrMap = parameters.getStoreAddr()
      for (port <- refs.keys) {
        val data = refs(port)
        val Addr = port._2
        //c.io.iob_ens(port._1).poke(1.B)
        c.io.hostInterface(port._1).en.poke(1.B)
        c.io.hostInterface(port._1).we.poke(0.B)
        c.io.hostInterface(port._1).addr.poke(asUnsignedInt(Addr).U)
      //  c.io.hostInterface(port._1).read_data.ready.poke(1.B)
        c.clock.step(1)
        c.io.hostInterface(port._1).dout.expect(asUnsignedInt(data).U)
        println("StoreMemID: " + port._1 + " Addr: " + Addr.toString)
        println(asUnsignedInt(data).toString + " " + c.io.hostInterface(port._1).dout.peek().toString())
      }
  }
}

class BMTester(c: CGRA_SRAM, cfgFilename: String, parameters: Parameters, innum: Int, outnum: Int, loadnum: Int, storenum:Int, iob_ensString:String) extends ApplicationTester(c, parameters) {
  val rows = c.param.rows
  val cols = c.param.cols
  c.clock.setTimeout(0)
  // read config bit file
  Source.fromFile(cfgFilename).getLines().foreach{
    line => {
      val items = line.split(" ")
      val addr = Integer.parseInt(items(0), 16);        // config bus address
      val data = BigInt(new BigInteger(items(1), 16));  // config bus data
      c.io.cfg_en.poke(1.B)
      c.io.cfg_addr.poke(addr.U)
      c.io.cfg_data.poke(data.U)
      c.clock.step(1)
    }
  }

  // delay for config done
  c.clock.step(c.cfgRegNum + 2)
  c.io.cfg_en.poke(0.B)
  val iob_ens = BigInt(iob_ensString, 16)
  println(iob_ensString + " -> " + f"${iob_ens}%x")
  c.io.iob_ens.poke(iob_ens.U)
  if(loadnum != 0) {
    loads
  }
  // enable computation
  c.io.start.poke(1.B)
  c.clock.step(1)
  c.io.start.poke(0.B)
    c.io.en.poke(1.B)
  val testII = parameters.getTestII()
  
run(testII)


  // disable computation
    c.io.en.poke(0.B)

  if(storenum != 0) {
    checkStore
  }
  // input test data

  
}

