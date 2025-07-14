package fgramem.dsa

import scala.io.Source
import java.io._
import java.math.BigInteger
import scala.math.BigInt
import chisel3._
import chisel3.util._

import scala.collection.mutable
import scala.collection.mutable.{ArrayBuffer, ListBuffer}
import chisel3.iotesters.{ChiselFlatSpec, Driver, PeekPokeTester}
import fgramem.spec.FGRASpec
import scala.util.control.Breaks._


class CGRATest(c: CGRA_SRAM, cfgFilename: String) extends PeekPokeTester(c) {
  val rows = c.param.rows
  val cols = c.param.cols
  // read config bit file
  Source.fromFile(cfgFilename).getLines().foreach{
    line => {
      val items = line.split(" ")
      val addr = Integer.parseInt(items(0), 16);        // config bus address
      val data = BigInt(new BigInteger(items(1), 16));  // config bus data
      poke(c.io.cfg_en, 1)
      poke(c.io.cfg_addr, addr)
      poke(c.io.cfg_data, data)
      step(1)
    }
  }
  // delay for config done
  step(c.cfgRegNum + 2)
  poke(c.io.cfg_en, 0)
  // enable IOB
  val iob = 7
  poke(c.io.iob_ens, iob.U)
  // load data to spd
  var addres = 0
  var v2 = 1
  var v3 = Array(0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1)
  for (i <- 0 until 32) {
    poke(c.io.hostInterface(0).en, 1) // C_INPUT_0
    poke(c.io.hostInterface(0).we, 1)
    poke(c.io.hostInterface(0).addr, addres)
    poke(c.io.hostInterface(0).din, v2)
    step(1)
    v2 = v2 + 1
    addres = addres + 1
  }
//  addres = 0
//  var v1 = 1
//  for(i <- 0 until 32){
//    poke(c.io.hostInterface(1).en, 1) //C_INPUT_1
//    poke(c.io.hostInterface(1).we, 1)
//    poke(c.io.hostInterface(1).addr, addres)
//    poke(c.io.hostInterface(1).din, v3(i))
//    step(1)
//    v1 = v1 + 1
//    addres = addres + 1
//  }
//  poke(c.io.hostInterface(4).en, 0)
//  step(1)
  poke(c.io.hostInterface(0).en, 0)
  step(1)
  // enable computation
  poke(c.io.start,1)
  step(1)
  poke(c.io.start,0)
  poke(c.io.en, 1)
  var ext = 0
  breakable{
    while(true){
      val done = peek(c.io.done)
      step(1)
      println("done: " + done.toString)
      if(done.toString == "1"){
        println("finish execute!!")
        break()
      }
    }
  }
  poke(c.io.en, 0)
  step(1)
  var raddr = 0
  for (i <- 0 until 20) {
    poke(c.io.hostInterface(0).en, 1) //
    poke(c.io.hostInterface(0).we, 0)
    poke(c.io.hostInterface(0).addr, raddr)
    step(1)
    val dout = peek(c.io.hostInterface(0).dout)
    println(" addr: " + raddr + " data: " + dout.toInt)
    raddr = raddr + 1
  }
}


object CGRATester extends App {
  val jsonFile = "src/main/resources/fgra_spec.json"
  FGRASpec.loadSpec(jsonFile)
  // val dut = new CGRA(attrs)
  iotesters.Driver.execute(args, () => new CGRA_SRAM(FGRASpec.attrs)) {
    c => new CGRATest(c, "src/main/resources/config.bit")
  }
}


