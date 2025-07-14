package fgramem.mg

import fgramem.spec._
import fgramem.op._
import fgramem.dsa._
//import fgramem.ppa.ppa_cgra.FGRA_area

import java.io.File

// TODO: add to command options
//case class Config(
//  loadSpec: Boolean = true,
//  dumpOperations: Boolean = true,
//  dumpIR: Boolean = true,
//  genVerilog: Boolean = true,
//)

// FGRA Modeling and Generation
object FGRAMG extends App{
  val loadSpec : Boolean = true
  val dumpSpec : Boolean = false
  val dumpOperationSet : Boolean = true
  val dumpADG : Boolean = true
  val genVerilog : Boolean = true
  val getArea : Boolean = false
  val rootDirPath = (new File("")).getAbsolutePath()
  val fgra_spec_filename = rootDirPath + "/src/main/resources/fusion_spec.json"
  val operation_set_filename = rootDirPath + "/src/main/resources/operations.json"
  val fgra_adg_filename = rootDirPath + "/src/main/resources/fgra_adg.json"
  if(dumpSpec){ FusionSpec.dumpSpec(fgra_spec_filename) }
  if(loadSpec){ FusionSpec.loadSpec(fgra_spec_filename) }
  FusionSpec.attrs("dumpOperationSet") = dumpOperationSet
  if(dumpOperationSet){ FusionSpec.attrs("operation_set_filename") = operation_set_filename }
  FusionSpec.attrs("dumpOperationSet") = dumpADG
  if(dumpADG){ FusionSpec.attrs("fgra_adg_filename") = fgra_adg_filename }

//  if(loadSpec){
//    val jsonFile = rootDirPath + "src/main/resources/fgra_spec.json"
//    FusionSpec.dumpSpec(jsonFile)
//    FusionSpec.loadSpec(jsonFile)
////    FusionSpec.dumpSpec(jsonFile)
//  }
//  if(dumpOperations){
//    val jsonFile = rootDirPath + "src/main/resources/operations.json"
//    OpInfo.dumpOpInfo(jsonFile)
//  }
  if(genVerilog){
    (new chisel3.stage.ChiselStage).emitVerilog(new FGRA(FusionSpec.attrs), args)
  }else{ // not emit verilog to speedup
    (new chisel3.stage.ChiselStage).emitChirrtl(new FGRA(FusionSpec.attrs), args)
  }
//  if(getArea){
//    val area = FGRA_area(FusionSpec.attrs)
//  }
}
