package fgra.dsa

import scala.collection.mutable.ArrayBuffer
import scala.io.Source

/** A class that helps users to automatically generate simulation codes for an architecture.
 * A result TXT can be interpreted in this class.
 * The result TXT indicate the mapping result of DFG IO nodes,
 * which consists of the name of these nodes, fire time and skew.
 */
class SimulationHelper() {

  var size = 0
  val opArray = new ArrayBuffer[String]()
  val fireTimeArray = new ArrayBuffer[Int]()
  var inputnum = 0
  var outputnum = 0
  var outputCycle = 0
  var loadnum = 0
  var storenum = 0
  val loadPorts = new ArrayBuffer[Int]()
  val storePorts = new ArrayBuffer[Int]()
  var II = 1
  val inputArray = new ArrayBuffer[Tuple2[Int, Int]]()
  val outputArray = new ArrayBuffer[Tuple2[Int, Int]]()
  var inputMap = Map[Tuple2[Int, Int], Array[Int]]()
  var outputMap = Map[Tuple2[Int, Int], Array[Int]]()
  val loadaddrArray = new ArrayBuffer[Int]()
  val loadlen = new ArrayBuffer[Int]()
  val storeaddrArray = new ArrayBuffer[Int]()
  val storelen = new ArrayBuffer[Int]()
  var iob_en = "abc"
  var loadDatas = new ArrayBuffer[ArrayBuffer[Int]]()
  var storeDatas = new ArrayBuffer[ArrayBuffer[Int]]()

  /** Add the mapping result of a mapped DFG node.
   *
   * @param result a line in the result TXT indicating the mapping result of a mapped DFG IO node
   */
  def addResult(result: String): Unit = {
    val tempList = result.replaceAll(",","").split(" ").toList

    //Add the name of the DFG node ID into opArray.
    val op = tempList(0)
    if (op.contains("#")) {
      return
    }
    else {
      if (op.contains("II")) {
        II = tempList(1).toInt
        return
      }
    }
    

    val moduleNum = tempList(5)
    if (op.contains("output") || op.contains("input")) {
      if (op.contains("output")) {
        //Add the identification number of the mapped output ports into outPorts.
        opArray.append(op)
        outputArray.append(Tuple2(tempList(1).toInt, tempList(2).toInt))
        outputnum = outputnum + 1
      } else if (op.contains("input")) {
        //Add the identification number of the mapped input ports into inputPorts.
        opArray.append(op)
        inputArray.append(Tuple2(tempList(1).toInt, tempList(2).toInt))
        inputnum = inputnum + 1
      }
    }
    if (op.contains("INPUT") || op.contains("OUTPUT") || op.contains("STORE") || op.contains("LOAD")) {
      if (op.contains("INPUT") || op.contains("LOAD")) {
        if(!loadPorts.contains(moduleNum.toInt)){
          //Add the identification number of the mapped output ports into outPorts.
          opArray.append(op)
          loadPorts.append(moduleNum.toInt)
          loadnum = loadnum + 1
        }
      
      } else if (op.contains("OUTPUT") || op.contains("STORE")) {
        if(!storePorts.contains(moduleNum.toInt)){
          //Add the identification number of the mapped input ports into inputPorts.
          opArray.append(op)
          storePorts.append(moduleNum.toInt)
          storenum = storenum + 1
        }

      }
    }
    //println(tempList(1))
    //Add the fire time of the mapped IO node into fireTimeArray.
    val fireTime = tempList(4).toInt
    fireTimeArray.append(fireTime)
  }

  def SRAM_addResult(result: String): Unit = {
    // val tempList = result.replaceAll("\\(","").replaceAll("\\);","").split(", ").toList
    // if (tempList(0).contains("load_data")) {
    //   val load_data_addr = Integer.parseInt(tempList(1).stripPrefix("0x"), 16)
    //   loadaddrArray.append(load_data_addr)
    //   loadlen.append(tempList(2).toInt/4)

    //   println("load_data_addr: " + load_data_addr + "load_data_len: " + tempList(2).toInt/4)
      
    // }
    // if (tempList(0).contains("store")) {
    //   val store_data_addr = Integer.parseInt(tempList(1).stripPrefix("0x"), 16)
    //   storeaddrArray.append(store_data_addr)
    //   storelen.append(tempList(2).toInt/4)

    //   println("store_data_addr: " + store_data_addr + "store_data_len: " + tempList(2).toInt/4)
    // }
    if (result.contains("(")) {
    val tempList = result.replaceAll("\\);","").split("\\(")
    val tempList1 = tempList(1).split(", ").toList
    if (tempList(0).contains("load_data")) {
      val load_data_addr = Integer.parseInt(tempList1(1).stripPrefix("0x"), 16)
      loadaddrArray.append(load_data_addr)
      loadlen.append(tempList1(2).toInt/4)

      println("load_data_addr: " + load_data_addr + "load_data_len: " + tempList1(2).toInt/4)
      
    }
    if (tempList(0).contains("store")) {
      val store_data_addr = Integer.parseInt(tempList1(1).stripPrefix("0x"), 16)
      storeaddrArray.append(store_data_addr)
      storelen.append(tempList1(2).toInt/4)

      println("store_data_addr: " + store_data_addr + "store_data_len: " + tempList1(2).toInt/4)
    }
    if (tempList(0).contains("execute")) {
      iob_en = tempList1(0).stripPrefix("0x")
      println("iob_en: " + iob_en)
    }
  }
  }

  def Golden_addResult(lines: List[String], prefix: Int): ArrayBuffer[ArrayBuffer[Int]] = {
    var inputArrays = ArrayBuffer[ArrayBuffer[Int]]()
    var currentArray = ArrayBuffer[Int]()
    var flag = 0
    if (prefix == 1) {
      for (line <- lines) {
        if(flag == 0){
        if (line.contains("INPUT") || line.contains("LOAD")) {
          if (currentArray.nonEmpty) {
            loadlen.append(currentArray.size)
            inputArrays += currentArray.clone()
            currentArray.clear()
          }
        }
        else if (!(line.contains("OUTPUT") || line.contains("STORE"))){
          currentArray += line.toInt
        }
        else {
          flag = 1 
        }
      }
      }
      if (currentArray.nonEmpty) {
        inputArrays += currentArray
        loadlen.append(currentArray.size)
      }
    }
    if (prefix == 2) {
      for (line <- lines) {
        if (line.contains("OUTPUT") || line.contains("STORE")) {
          flag = 1
          if (currentArray.nonEmpty) {
            storelen.append(currentArray.size)
            inputArrays += currentArray.clone()
            currentArray.clear()
          }
        }
        else if (flag == 1){
          currentArray += line.toInt
        } 
      }
      if (currentArray.nonEmpty) {
        inputArrays += currentArray
        storelen.append(currentArray.size)
      }
      
    }
    inputArrays
  }

  /** Reset values in this class.
   */
  def reset(): Unit = {
    size = 0
    opArray.clear()
    fireTimeArray.clear()
    outputArray.clear()
    outputMap = Map[Tuple2[Int, Int], Array[Int]]()
    inputMap = Map[Tuple2[Int, Int], Array[Int]]()
  }

  /** Initialize values in this class according to a result TXT.
   *
   * @param resultFilename the file name of the result TXT
   */
  def init(resultFilename: String, resultFilename2: String, resultFilename3: String): Unit = {
    reset()
    val resultArray = Source.fromFile(resultFilename).getLines().toArray
    val mesageArray = resultArray.tail
    mesageArray.map(r => addResult(r))
    size = opArray.size
    //Set the cycle we can obtain the last result.
    outputCycle = fireTimeArray.reduce((t1, t2) => Math.max(t1, t2))
    val SRAM_resultArray = Source.fromFile(resultFilename2).getLines().toArray
    val SRAM_mesageArray = SRAM_resultArray.tail
    SRAM_mesageArray.map(r => SRAM_addResult(r))
    val golden_model = Source.fromFile(resultFilename3).getLines().toList
    loadDatas = Golden_addResult(golden_model, 1)
    storeDatas = Golden_addResult(golden_model, 2)
    println("Input Arrays:")
    loadDatas.foreach(println)
    println("\nOutput Arrays:")
    storeDatas.foreach(println)
  }

  /** Get the number of inputports
   */
  def getinputnum(): Int = {
    inputnum
  }

  /** Get the number of outputports
   */
  def getoutputnum(): Int = {
    outputnum
  }

  /** Get the number of loadports
   */
  def getloadnum(): Int = {
    loadnum
  }

  /** Get the number of storeports
   */
  def getstorenum(): Int = {
    storenum
  }

  /** Get II
   */
  def getii(): Int = {
    println("II: " + II)
    II
  }
  /** Get data arrays with corresponding address.
   *
   * @param dataSize      the data size of a data array
   * @param inDataArrays  the input data arrays for LSUs
   * @param outDataArrays the expected data arrays for LSUs
   * @param refDataArrays the expected data arrays for the output ports
   * @return the expected data arrays for the output ports with corresponding identification number)
   */
  def getDataWithAddr(dataSize: Int = 0, inDataArrays: Array[Array[Int]] = null,
                      outDataArrays: Array[ArrayBuffer[Int]] = null, refDataArrays: Array[Array[Int]] = null): Array[Any] = {
    val refDataAddr = new ArrayBuffer[Tuple2[Int, Int]]()
    val refstoreAddr = new ArrayBuffer[Int]()
    var inDatas = Map[List[Int], Array[Int]]()
    var outDatas = Map[Int, ArrayBuffer[Int]]()
    var refDatas = Map[Int, ArrayBuffer[Int]]()
    var index = 0
    var p = 0
    var q = 0
    var addrMap = Map[Int, Int]()
    var addr = 0
    
      for (i <- 0 until size) {
        val op = opArray(i)
        if (op.contains("output")) {
          refDataAddr.append(outputArray(p))
          p = p + 1
        }
        if (op.contains("OUTPUT")) {
          refstoreAddr.append(storePorts(q))
          q = q + 1
        }
      }

    for (i <- 0 until refDataAddr.size) {
      outputMap = outputMap ++ Map(refDataAddr(i) -> refDataArrays(i))
    }

    for (i <- 0 until refstoreAddr.size) {
      outDatas = refDatas ++ Map(refstoreAddr(i) -> outDataArrays(i))
    }
    Array(outputMap, outDatas)
  }

  /** Get the cycle we can obtain the result.
   *
   * @return the cycle we can obtain the result
   */
  def getOutputCycle(): Int = {
    outputCycle
  }
}
