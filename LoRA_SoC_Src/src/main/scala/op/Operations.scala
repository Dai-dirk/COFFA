package fgramemfp.op
// ALU operations

import chisel3._
import chisel3.util._
import scala.collection.mutable
import scala.collection.mutable.ListBuffer
import fgramemfp.ir._
import fgramemfp.common.CompileMacroVar._

/**
 * Operation Code
 */
//object OPC extends Enumeration {
//	type OPC = Value
//	// Operation Code
//	val PASS,  // passthrough from in to out
//	  NOT,    // not
//		ADD,     // add
//	  SUB,     // substrate
////		ADC,     // add with carrier
////		SBC,     // substrate with carrier
//	  MUL,     // multiply
//	  // DIV,     // divide
//	  // MOD,     // modulo
//	  // MIN,
//	  // NOT,
//	  AND,
//	  OR,
//	  XOR,
//	  SHL,     // shift left
//	  LSHR,    // logic shift right
//		ASHR,    // arithmetic shift right
////	  CSHL,    // cyclic shift left
////	  CSHR,    // cyclic shift right
//	  EQ,      // equal to
//	  NE,      // not equal to
//	  ULT,      // unsigned less than
//	  ULE,      // unsigned less than or equal to
////		SAT,		 // saturate value to a threshold
////		MGE,		 // merge two data
////		SPT,	   // split one data to two
//	  SEL,
//		INPUT,
//		OUTPUT,
//		LOAD,
//		STORE = Value
//
//	val numOPC = this.values.size - 4 // not including INPUT/OUTPUT/LOAD/STORE, since ALU OPC config width is determined by numOPC
//
//	def printOPC = {
//		this.values.foreach{ op => println(s"$op\t: ${op.id}")}
//	}
//}

/**
 *  Operation Information
 */
object OpInfo {
//	import OPC._
// Basic ALU Operation Information
val BasicOpInfoMap: Map[String, ListBuffer[Int]] = Map(
	// OPC -> List(NumOperands, NumRes, Latency, Operands-Commutative)
	// latency including the register outside ALU
	"PASS" -> ListBuffer(1, 1, 1, 0),
	"NOT" -> ListBuffer(1, 1, 1, 0),
	"ADD" -> ListBuffer(3, 2, 1, 0),
	"SUB" -> ListBuffer(2, 1, 1, 0),
	"SBC" -> ListBuffer(3, 2, 1, 0),
	"MUL" -> ListBuffer(2, 1, 1, 1),
	"UDIV" -> ListBuffer(2, 1, DIV_LATENCY, 0, 0),
	"SDIV" -> ListBuffer(2, 1, DIV_LATENCY, 0, 0),
	"MOD" -> ListBuffer(2, 1, 1, 0),
	"MIN" -> ListBuffer(2, 1, 1, 1),
	"MAX" -> ListBuffer(2, 1, 1, 1),
	"AND" -> ListBuffer(2, 1, 1, 1),
	"OR" -> ListBuffer(2, 1, 1, 1),
	"XOR" -> ListBuffer(2, 1, 1, 1),
	"XNOR" -> ListBuffer(2, 1, 1, 1),
	"SHL" -> ListBuffer(2, 1, 1, 0),
	"LSHR" -> ListBuffer(2, 1, 1, 0),
	"ASHR" -> ListBuffer(2, 1, 1, 0),
	"CSHL" -> ListBuffer(2, 1, 1, 0),
	"CSHR" -> ListBuffer(2, 1, 1, 0),
	"EQ" -> ListBuffer(2, 2, 1, 1),
	"NE" -> ListBuffer(2, 2, 1, 1),
	"ULT" -> ListBuffer(2, 2, 1, 0),
	"ULE" -> ListBuffer(2, 2, 1, 0),
	"UGT" -> ListBuffer(2, 2, 1, 0),
	"UGE" -> ListBuffer(2, 2, 1, 0),
	"SLT" -> ListBuffer(2, 2, 1, 0),
	"SLE" -> ListBuffer(2, 2, 1, 0),
	"SGT" -> ListBuffer(2, 2, 1, 0),
	"SGE" -> ListBuffer(2, 2, 1, 0),
	"FADD" -> ListBuffer(2, 1, 1, 1),//@yuan: float-point operations
	"FSUB" -> ListBuffer(2, 1, 1, 0),
	"FMUL" -> ListBuffer(2, 1, 3, 1),
	"FDIV" -> ListBuffer(2, 1, 6, 0),
	"FSQRT" -> ListBuffer(2, 1, 17, 0),
	"FEQ" -> ListBuffer(2, 2, 1, 1),
	"FLT" -> ListBuffer(2, 2, 1, 0),
	"FLE" -> ListBuffer(2, 2, 1, 0),
	"FEXADD" -> ListBuffer(2, 1, 1, 0),
	"FEXSUB" -> ListBuffer(2, 1, 1, 0),
	"XCORE" -> ListBuffer(2, 1, 4, 0),
	"SEL" -> ListBuffer(3, 1, 1, 0),
	"SEXT" -> ListBuffer(3, 1, 1, 0),
	"ZEXT" -> ListBuffer(3, 1, 1, 0),	
	"FPTOSI" -> ListBuffer(1, 1, 1, 0),
	"SITOFP" -> ListBuffer(1, 1, 1, 0),	
	"FMIN" -> ListBuffer(2, 1, 1, 1),
	"FMAX" -> ListBuffer(2, 1, 1, 1),
	"PASS_CF"-> ListBuffer(1, 2, 1, 0), //@yuan: for coarse-only
	"NOT_CF" -> ListBuffer(1, 2, 1, 0),
	"AND_CF" -> ListBuffer(2, 2, 1, 1),
	"OR_CF" -> ListBuffer(2, 2, 1, 1)
)
	// Accumulative Operation Information
	// Periodic initialization and accumulation
	// res = f(op)
	//TODO: Some FP operations can perform accumulation
	val AccOpInfoMap: Map[String, (ListBuffer[Int], String)] = Map(
		// OpName -> List(NumOperands, NumRes, Latency, Operands-Commutative), Basic OpName
		// latency including the register outside ALU
		"ACC" -> (ListBuffer(1, 1, 1, 0), "ADD"), // +=
		"ASUB" -> (ListBuffer(1, 1, 1, 0), "SUB"), // -=
		"AMUL" -> (ListBuffer(1, 1, 1, 0), "MUL"), // *=
		"ADIV" -> (ListBuffer(1, 1, 1, 0), "DIV"), // /=
		"AMOD" -> (ListBuffer(1, 1, 1, 0), "MOD"), // %=
		"AAND" -> (ListBuffer(1, 1, 1, 0), "AND"), // &=
		"AOR" -> (ListBuffer(1, 1, 1, 0), "OR"), // |=
		"AXOR" -> (ListBuffer(1, 1, 1, 0), "XOR"), // ^=
		"ASHL" -> (ListBuffer(1, 1, 1, 0), "SHL"), // <<=
		"ALSHR" -> (ListBuffer(1, 1, 1, 0), "LSHR"), // >>=
		"AASHR" -> (ListBuffer(1, 1, 1, 0), "ASHR"), // >>>=		
		"FACC" -> (ListBuffer(1, 1, 1, 0), "FADD"), // += //floating-point accumulation
	)

	// Conditionally accumulative Operation Information
	// Periodic initialization and irregular conditional accumulation
	// res = f(op, en)
	val CondAccOpInfoMap: Map[String, (ListBuffer[Int], String)] = Map(
		// OpName -> List(NumOperands, NumRes, Latency, Operands-Commutative)
		// latency including the register outside ALU
		// the second operand is accumulation enable
		"CACC" -> (ListBuffer(2, 1, 1, 0), "ADD"), // +=
		"CASUB" -> (ListBuffer(2, 1, 1, 0), "SUB"), // -=
		"CAMUL" -> (ListBuffer(2, 1, 1, 0), "MUL"), // *=
		"CADIV" -> (ListBuffer(2, 1, 1, 0), "DIV"), // /=
		"CAMOD" -> (ListBuffer(2, 1, 1, 0), "MOD"), // %=
		"CAAND" -> (ListBuffer(2, 1, 1, 0), "AND"), // &=
		"CAOR" -> (ListBuffer(2, 1, 1, 0), "OR"), // |=
		"CAXOR" -> (ListBuffer(2, 1, 1, 0), "XOR"), // ^=
		"CASHL" -> (ListBuffer(2, 1, 1, 0), "SHL"), // <<=
		"CALSHR" -> (ListBuffer(2, 1, 1, 0), "LSHR"), // >>=
		"CAASHR" -> (ListBuffer(2, 1, 1, 0), "ASHR"), // >>>=
	)

	// Conditionally initial and accumulative Operation Information
	// irregular conditional initialization and accumulation
	// res = f(op, en, init)
	val CondInitAccOpInfoMap: Map[String, (ListBuffer[Int], String)] = Map(
		// OpName -> List(NumOperands, NumRes, Latency, Operands-Commutative)
		// latency including the register outside ALU
		// the second operand is accumulation enable
		// the third operand is initialize
		"CIACC" -> (ListBuffer(3, 1, 1, 0), "ADD"), // +=
		"CIASUB" -> (ListBuffer(3, 1, 1, 0), "SUB"), // -=
		"CIAMUL" -> (ListBuffer(3, 1, 1, 0), "MUL"), // *=
		"CIADIV" -> (ListBuffer(3, 1, 1, 0), "DIV"), // /=
		"CIAMOD" -> (ListBuffer(3, 1, 1, 0), "MOD"), // %=
		"CIAAND" -> (ListBuffer(3, 1, 1, 0), "AND"), // &=
		"CIAOR" -> (ListBuffer(3, 1, 1, 0), "OR"), // |=
		"CIAXOR" -> (ListBuffer(3, 1, 1, 0), "XOR"), // ^=
		"CIASHL" -> (ListBuffer(3, 1, 1, 0), "SHL"), // <<=
		"CIALSHR" -> (ListBuffer(3, 1, 1, 0), "LSHR"), // >>=
		"CIAASHR" -> (ListBuffer(3, 1, 1, 0), "ASHR"), // >>>=
	)
//@yuan:
// Conditionally initial and accumulative Operation Information, with double initial value
// irregular conditional initialization and accumulation
// res = f(op1, op2, en, init)
val CondDualInitAccOpInfoMap: Map[String, (ListBuffer[Int], String)] = Map(
	// OpName -> List(NumOperands, NumRes, Latency, Operands-Commutative)
	// latency including the register outside ALU
	// the second operand is accumulation enable
	// the third operand is initialize
	"CDIACC" -> (ListBuffer(4, 1, 1, 0), "ADD"), // +=
	"CDIASUB" -> (ListBuffer(4, 1, 1, 0), "SUB"), // -=
	"CDIAMUL" -> (ListBuffer(4, 1, 1, 0), "MUL"), // *=
	"CDIADIV" -> (ListBuffer(4, 1, 1, 0), "DIV"), // /=
	"CDIAMOD" -> (ListBuffer(4, 1, 1, 0), "MOD"), // %=
	"CDIAAND" -> (ListBuffer(4, 1, 1, 0), "AND"), // &=
	"CDIAOR" -> (ListBuffer(4, 1, 1, 0), "OR"), // |=
	"CDIAXOR" -> (ListBuffer(4, 1, 1, 0), "XOR"), // ^=
	"CDIASHL" -> (ListBuffer(4, 1, 1, 0), "SHL"), // <<=
	"CDIALSHR" -> (ListBuffer(4, 1, 1, 0), "LSHR"), // >>=
	"CDIAASHR" -> (ListBuffer(4, 1, 1, 0), "ASHR"), // >>>=
)


	// Load/Store Operation Information
	val LSOpInfoMap: Map[String, ListBuffer[Int]] = Map(
		// OpName -> List(NumOperands, NumRes, Latency, Operands-Commutative)
		// latency including the register outside LUT
		"INPUT" -> ListBuffer(0, 1, 2, 0),
		"OUTPUT" -> ListBuffer(1, 0, 1, 0),
		"LOAD" -> ListBuffer(1, 1, 2, 0),
		"STORE" -> ListBuffer(2, 0, 1, 0),
		"CINPUT" -> ListBuffer(1, 1, 2, 0),
		"COUTPUT" -> ListBuffer(2, 0, 1, 0),
		"CLOAD" -> ListBuffer(2, 1, 2, 0),
		"CSTORE" -> ListBuffer(3, 0, 1, 0),
		"TLOAD" -> ListBuffer(2, 1, 2, 0),
		"TSTORE" -> ListBuffer(3, 0, 1, 0),
		"TCLOAD" -> ListBuffer(3, 1, 2, 0),
		"TCSTORE" -> ListBuffer(4, 0, 1, 0)
	)
	//@yuan
	// Load/Store Operation Information
	val LUTOpInfoMap: Map[String, ListBuffer[Int]] = Map(
		// OpName -> List(NumOperands, NumRes, Latency, Operands-Commutative)
		// latency including the register outside ALU
		"LUT" -> ListBuffer(3, 1, 1, 0)
	)

	//@yuan: isel operation
	//Periodic initialization selection
	// res = f(op1, op2)
	// this operation is similar to CDIACC, the selection is done in the DMR
	// hence, the ALU only execute pass operation to pass one operand
	// another operand is connect to DMR directly
	val ISelInfoMap: Map[String, (ListBuffer[Int], String)] = Map(
		"ISEL" -> (ListBuffer(2, 1, 1, 0), "PASS"), // isel
	)
	//@yuan: extent to CISEL, which has condition input, i.e. res = f(op1, op2, en, init)
		val CondISelInfoMap: Map[String, (ListBuffer[Int], String)] = Map(
			"CISEL" ->(ListBuffer(4, 1, 1, 0), "PASS"),
		)
	
	//@yuan: multiply-accumulation operation (MAC)
	val MACInfoMap: Map[String, ListBuffer[Int]] = Map(
		"MAC" -> ListBuffer(2, 1, 1, 1)
	)
	//@yuan: conditiinal multiply-accumulation operation (MAC)
	val CMACInfoMap: Map[String, (ListBuffer[Int], String)] = Map(
		"CMAC" -> (ListBuffer(3, 1, 1, 0), "MAC")
	)

	// OPName -> List(NumOperands, NumRes, Latency, Operands-Commutative, Accumulative-operation)
	// latency including the register outside ALU
	val OpInfoMap: Map[String, ListBuffer[Int]] = {
		BasicOpInfoMap.map { case (name, info) => name -> (info :+ 0) } ++
		AccOpInfoMap.map { case (name, (info, basic)) => name -> (info :+ 1) } ++
		CondAccOpInfoMap.map { case (name, (info, basic)) => name -> (info :+ 1) } ++
		CondInitAccOpInfoMap.map { case (name, (info, basic)) => name -> (info :+ 1) } ++
		CondDualInitAccOpInfoMap.map { case (name, (info, basic)) => name -> (info :+ 1) } ++
		ISelInfoMap.map{case (name, (info, basic)) => name -> (info :+ -1)} ++
		CondISelInfoMap.map { case (name, (info, basic)) => name -> (info :+ -1) } ++
		LSOpInfoMap.map { case (name, info) => name -> (info :+ 0) } ++
		LUTOpInfoMap.map { case (name, info) => name -> (info :+ 0) }++
		MACInfoMap.map { case (name, info) => name -> (info :+ 0) }
	}

	def getOperandNum(op: String): Int = {
		OpInfoMap(op)(0)
	}

	def getResNum(op: String): Int = {
		OpInfoMap(op)(1)
	}

	def getLatency(op: String): Int = {
		OpInfoMap(op)(2)
	}

	def setLatency(op: String, lat: Int): Unit = {
		OpInfoMap(op)(2) = lat
	}

	def isCommutative(op: String): Int = {
		OpInfoMap(op)(3)
	}

	def isAccumulative(op: String): Int = {
		if(op == "MAC" || op == "CMAC"){
			1
		}else{
			OpInfoMap(op)(4)
		}
	}

	def isISelection(op: String): Int = {
		OpInfoMap(op)(4)
	}
	
	//@hw_yuan: identify the type of the operations	
	//@yuan_fp: adding support for float-point operations	
	def getOpTypeId(op: String): Int = {
		if (op == "ADD" || op == "SUB" || op == "ULE" || op == "ULT" || op == "UGT" || op == "UGE" || op == "SLT" || op == "SLE" || op == "SGT" || op == "SGE") {
			1
		} else if (op == "AND" || op == "OR" || op == "NOT" || op == "XOR" || op == "XNOR"|| op == "NE"|| op == "EQ") {
			2
		} else if (op == "SHL" || op == "LSHR" || op == "CSHL" || op == "CSHR" || op == "ASHR") {
			4
		} else if (op == "MUL") {
			8
		} else if (op == "SEL") {
			16
		} else if (op == "FADD" || op == "FSUB" ){
			32
		} else if (op == "FEQ" || op == "FLT" || op == "FLE"){
			64
		} else if (op == "FEXADD" || op == "FEXSUB"){
			128
		}else{// for the unique operation, we using 4 input ports in ALU
			31
		}
	}


	def getOpType(op: String): String = {
		if (op == "ADD" || op == "SUB" || op == "ULE" || op == "ULT" || op == "UGT" || op == "UGE" || op == "SLT" || op == "SLE"|| op == "SGT" || op == "SGE") {
			"ARITH"
		} else if (op == "AND" || op == "OR" || op == "NOT" || op == "XOR" || op == "XNOR" || op == "NE" || op == "EQ") {
			"LOGIC"
		} else if (op == "SHL" || op == "LSHR" || op == "CSHL" || op == "CSHR" || op == "ASHR") {
			"SHIFT"
		} else if (op == "FADD" || op == "FSUB" ){
			"FADDSUB"
		}else if (op == "FEQ" || op == "FLT" || op == "FLE"){
			"FCMP"
		} else if (op == "FEXADD" || op == "FEXSUB") {
			"FEAS"
		}else{
			op
		}
	}

	def getTypicalOp(op: String): String = {
		if(op == "ARITH"){
			"ADD"
		}else if(op == "LOGIC"){
			"AND"
		}else if(op == "SHIFT"){
			"SHL"
		} else if (op == "FADDSUB") {
			"FADD"
		} else if (op == "FCMP") {
			"FEQ"
		} else if (op == "FEAS") {
			"FEXADD"
		} else if (op == "FDIVSQRT") {
			"FDIV"
		}else{
			op
		}
	}

	def getALUOp(op: String): String = {
		if (AccOpInfoMap.contains(op)) {
			AccOpInfoMap(op)._2
		} else if (CondAccOpInfoMap.contains(op)) {
			CondAccOpInfoMap(op)._2
		} else if (CondInitAccOpInfoMap.contains(op)) {
			CondInitAccOpInfoMap(op)._2
		} else if (CondDualInitAccOpInfoMap.contains(op)) {
			CondDualInitAccOpInfoMap(op)._2
		} else if (ISelInfoMap.contains(op)) {
			ISelInfoMap(op)._2
		} else if (CondISelInfoMap.contains(op)) {
			CondISelInfoMap(op)._2
		} else {
			op
		}
	}

	//@yuan: get maximum CACC/CIACC fine-grain input
	def getAccFGOperand(op: String): Int = {
		if (CondAccOpInfoMap.contains(op) ) {
			1
		} else if (CondInitAccOpInfoMap.contains(op) || CondDualInitAccOpInfoMap.contains(op) || CondISelInfoMap.contains(op)) {
			2
		} else {
			0
		}
	}
	def getALUOperandNum(op: String): Int = {
		if (AccOpInfoMap.contains(op)) {
			OpInfoMap(AccOpInfoMap(op)._2)(0)
		} else if (CondAccOpInfoMap.contains(op)) {
			OpInfoMap(CondAccOpInfoMap(op)._2)(0)
		} else if (CondInitAccOpInfoMap.contains(op)) {
			OpInfoMap(CondInitAccOpInfoMap(op)._2)(0)
		} else if (CondDualInitAccOpInfoMap.contains(op)) {
			OpInfoMap(CondDualInitAccOpInfoMap(op)._2)(0)
		} else if (ISelInfoMap.contains(op)) {
			OpInfoMap(ISelInfoMap(op)._2)(0)
		} else if (CondISelInfoMap.contains(op)) {
			OpInfoMap(CondISelInfoMap(op)._2)(0)
		} else if (CMACInfoMap.contains(op)) {
			OpInfoMap(CMACInfoMap(op)._2)(0)
		}else {
			OpInfoMap(op)(0)
		}
	}

	def setLUTWidth(op: String, width: Int): Unit = {
		OpInfoMap(op)(0) = width
	}

	private var width = 32
	private var high = width - 1

	def apply(opWidth: Int) = {
		width = opWidth
		high = width - 1
		this
	}

	private var OPSet: ListBuffer[String] = ListBuffer(
		"PASS", // passthrough from in to out
		"PASS_CF", // passthrough from in to out, for
		"ADD", // add
		"SUB", // substrate
		"MUL", // multiply
		//		"UDIV",     // divide
		//		"MOD",     // modulo
		//		"MIN",
		"NOT",
		"AND",
		"XNOR",
		"OR",
		//		"OR",
		"XOR",
		"SHL",     // shift left
		"LSHR",    // logic shift right
		"ASHR",    // arithmetic shift right
		"CSHL",    // cyclic shift left
		"CSHR",    // cyclic shift right
		"EQ", // equal to
		"NE", // not equal to
		"ULT", // less than
		"ULE", // less than or equal to
		"UGT", // greater than
		"UGE", // greater than or equal to
		"SLT",
		"SLE",
		"SGT",
		"SGE",
		"SEL",
		"LUT",
		"FADD",
		"FSUB",
		"FMUL",
		"FDIV",
		"FEQ",
		"FLT",
		"FLE",
		"FEXADD",
		"FEXSUB",		
		"FPTOSI",
		"SITOFP",
		"ACC", // +=
		"FACC",
		//		"ASUB",  // -=
		//		"AMUL",  // *=
		//		"ADIV",  // /=
		//		"AMOD",  // %=
		//		"AAND",  // &=
		//		"AOR",   // |=
		//		"AXOR",  // ^=
		//		"ASHL",  // <<=
		//		"ALSHR", // >>=
		//		"AASHR", // >>>=
		"INPUT",
		"OUTPUT",
		"LOAD",
		"STORE"
	)

	val OPCMap = mutable.Map[String, Int]("PASS" -> 0) // "op name" -> opc number
	var BasicOPCWidth: Int = 0
	var ALUOPCWidth: Int = 0
	var LSOPCWidth: Int = 0

	def apply(ops: ListBuffer[String]): Unit = {
		OPSet = ops
		var hasAcc = false
		var hasMAC = false
		var hasCondAcc = false
		var hasCondRstAcc = false
		var hasInitSelection = false
		var hasCondDualRstAcc = false
		var hasCondInitSelection = false
		val ALUOpc = mutable.Map[String, Int]("PASS" -> 0)
		val UnifiedALUOpc = mutable.Map[String, Int]("PASS" -> 0)
		var LSOpNum = 0
//		var LUTOpNum = 0
		// first traversal
		// println("first OPCMap: " + OPCMap + " ops: " + ops)
		ops.foreach { op =>
			val basicOp = {
				if (BasicOpInfoMap.contains(op)) {
					op
				} else if (AccOpInfoMap.contains(op)) {
					hasAcc = true
					AccOpInfoMap(op)._2
				} else if (CondAccOpInfoMap.contains(op)) {
					hasCondAcc = true
					CondAccOpInfoMap(op)._2
				} else if (CondInitAccOpInfoMap.contains(op)) {
					hasCondRstAcc = true
					CondInitAccOpInfoMap(op)._2
				} else if (CondDualInitAccOpInfoMap.contains(op)){
					hasCondDualRstAcc = true;
					CondDualInitAccOpInfoMap(op)._2
				} else if (ISelInfoMap.contains(op)) {
					hasInitSelection = true;
					ISelInfoMap(op)._2
				} else if (CondISelInfoMap.contains(op)) {
					hasCondInitSelection = true;
					CondISelInfoMap(op)._2
				}else if(LUTOpInfoMap.contains(op)){
					println("lut? : " + op)
					OPCMap += (op -> 0)
					"NULL"
				}else{
					if (LSOpInfoMap.contains(op)) {
						OPCMap += (op -> LSOpNum)
						LSOpNum += 1
					}
					"NULL"
				}
			}
			if (basicOp != "NULL" && !ALUOpc.contains(basicOp)) {
//				val newopc = ALUOpc.size
				val newopc = { //@hw_yuan: the same unified unit share the same opc
					val tempOpc = UnifiedALUOpc.size
//					println("basicOp: " + basicOp + " getOpType(basicOp): " + getOpType(basicOp) + " UnifiedALUOpc: " + UnifiedALUOpc)
					if(UnifiedALUOpc.contains(getOpType(basicOp))){
						UnifiedALUOpc(getOpType(basicOp))
					}else{
						UnifiedALUOpc += (getOpType(basicOp) -> tempOpc)
//						OPCMap += (getOpType(basicOp) -> tempOpc)
						tempOpc
					}
				}
//				println("basicOp: " + basicOp + " newopc: " + newopc)
				ALUOpc += (basicOp -> newopc)
				OPCMap += (basicOp -> newopc)
			}
			if (MACInfoMap.contains(op) || CMACInfoMap.contains(op)) {
				hasMAC = true
			}
		}
		// println("OPCMap: " + OPCMap)
		LSOPCWidth = {
			if (LSOpNum > 0) log2Ceil(LSOpNum) else 0
		}
		BasicOPCWidth = if(hasMAC) log2Ceil(UnifiedALUOpc.size + 1) else log2Ceil(UnifiedALUOpc.size)
//		BasicOPCWidth = if (hasMAC) log2Ceil(ALUOpc.size + 1) else log2Ceil(ALUOpc.size)
		ALUOPCWidth = BasicOPCWidth + {
			if(hasMAC){
				4
			}else if(hasCondDualRstAcc || hasInitSelection || hasCondInitSelection){
				3
			}else if (hasCondRstAcc || hasCondAcc) {
				2
			} else if (hasAcc) {
				1
			} else {
				0
			}
		}
		// second traversal
		ops.foreach { op =>
			if (MACInfoMap.contains(op) || CMACInfoMap.contains(op)) {
				val newopc = UnifiedALUOpc.size
//				val newopc = ALUOpc.size
				UnifiedALUOpc += (op -> (newopc + (7 << BasicOPCWidth)))
				ALUOpc += (op -> (newopc + (7 << BasicOPCWidth)))
				OPCMap += (op -> (newopc + (7 << BasicOPCWidth)))
			}
		}
		// third traversal
		ops.foreach { op =>
			if (AccOpInfoMap.contains(op)) {
				val basicOp = AccOpInfoMap(op)._2
				val newopc = ALUOpc(basicOp) + (1 << BasicOPCWidth)
				OPCMap += (op -> newopc)
			} else if (CondAccOpInfoMap.contains(op)) {
				val basicOp = CondAccOpInfoMap(op)._2
				val newopc = ALUOpc(basicOp) + (2 << BasicOPCWidth)
				OPCMap += (op -> newopc)
			} else if (CondInitAccOpInfoMap.contains(op)) {
				val basicOp = CondInitAccOpInfoMap(op)._2
				val newopc = ALUOpc(basicOp) + (3 << BasicOPCWidth)
				OPCMap += (op -> newopc)
			} else if (CondDualInitAccOpInfoMap.contains(op)){
				val basicOp = CondDualInitAccOpInfoMap(op)._2
				val newopc = ALUOpc(basicOp) + (4 << BasicOPCWidth)
				OPCMap += (op -> newopc)
			} else if (ISelInfoMap.contains(op)) {
				val basicOp = ISelInfoMap(op)._2
				val newopc = ALUOpc(basicOp) + (5 << BasicOPCWidth)
				OPCMap += (op -> newopc)
			} else if (CondISelInfoMap.contains(op)) {
				val basicOp = CondISelInfoMap(op)._2
				val newopc = ALUOpc(basicOp) + (6 << BasicOPCWidth)
				OPCMap += (op -> newopc)
			} else if (CMACInfoMap.contains(op)) {
				val basicOp = op
				val newopc = ALUOpc(basicOp) + (8 << BasicOPCWidth)
				OPCMap += (op -> newopc)
			}
		}
//		println("OPCMap second: " + OPCMap)
	}
	apply(OPSet)

	def isnotAccOp(op: UInt): Bool = {
		op(ALUOPCWidth - 1, BasicOPCWidth) <= 0.U || op(ALUOPCWidth - 1, BasicOPCWidth) >= 5.U
	}

	def isMACop(op: UInt): Bool = {
		op(ALUOPCWidth - 1, BasicOPCWidth) >= 7.U
	}

	def isDualInitAccOp(op: UInt): Bool = {
		op(ALUOPCWidth - 1, BasicOPCWidth) === 4.U
	}

//	def isAccOp(op: UInt): Bool = {
//		op(ALUOPCWidth - 1, BasicOPCWidth) === 1.U
//	}
//
//	def isCondAccOp(op: UInt): Bool = {
//		op(ALUOPCWidth - 1, BasicOPCWidth) === 2.U
//	}
//
//	def isCondRstAccOp(op: UInt): Bool = {
//		op(ALUOPCWidth - 1, BasicOPCWidth) === 3.U
//	}

	def getAccMode(op: UInt): UInt = {
		val mode = Wire(UInt((ALUOPCWidth - BasicOPCWidth).W))
		when(op(ALUOPCWidth - 1, BasicOPCWidth) >= 7.U){
			mode := 1.U
		}.otherwise{
			mode := op(ALUOPCWidth - 1, BasicOPCWidth)
		}
		mode
	}	
// 	def OpFuncs(ops: Seq[UInt], cfg: UInt,hasMAC: Boolean): Map[String, Seq[UInt]] = {
// 		//		def OpFuncs(ops: Seq[UInt], opc: UInt, opset: ListBuffer[OPC]) : Map[String, Seq[UInt]] = {
// 		//		val op_names = opset.map(_.toString)
// 		val op0 = ops.head(high, 0)
// 		val op1 = ops(1)(high, 0)
// 		val op2 = {
// 			if (ops.size > 3) ops(2)(high, 0) // op2 is used as feedback
// 			else if(ops.size == 3 && hasMAC) ops(2)(high, 0)
// 			else 0.U(1.W)
// 		}
// 		val op3 = {
// 			if (ops.size > 3) ops(3)(0)
// 			else if(ops.size == 3 && !hasMAC) ops(2)(0)
// 			else 0.U(1.W)
// 		}
// 		val udiv = Module(new Div(width, false, DIV_LATENCY - 1))
// 		val sdiv = Module(new Div(width, true, DIV_LATENCY - 1))
// 		udiv.io.op0 := DontCare
// 		udiv.io.op1 := DontCare
// 		sdiv.io.op0 := DontCare
// 		sdiv.io.op1 := DontCare

// 		val multipler = Module(new Mul(width))
// 		multipler.io.op0 := op0
// 		multipler.io.op1 := op1

// 		val arith = Module(new ARITH(width))
// 		arith.io.in(0) := op0
// 		arith.io.in(1) := op1
// 		arith.io.config := cfg

// 		val logic = Module(new LOGIC(width))
// 		logic.io.in(0) := op0
// 		logic.io.in(1) := op1
// 		logic.io.config := cfg

// 		val shift = Module(new SHIFT(width))
// 		shift.io.in(0) := op0
// 		shift.io.in(1) := op1
// 		shift.io.config := cfg

// //		println("op size: " + ops.size)
// 		//		val op1_inv = (~op1).asUInt
// 		//		val op1_new = Wire(UInt(width.W))
// 		//		val op2_new = Wire(UInt(1.W))
// 		//		op1_new := op1
// 		//		op2_new := 0.U(1.W)
// 		//		if(op_names.contains("SUB")){
// 		//			when(opc === OPC.withName("SUB").id.U){
// 		//				op1_new := op1_inv
// 		//				op2_new := 1.U(1.W)
// 		//			}
// 		//		}
// 		//		if(op_names.contains("ASUB")){
// 		//			when(opc === OPC.withName("ASUB").id.U){
// 		//				op1_new := op1_inv
// 		//				op2_new := 1.U(1.W)
// 		//			}
// 		//		}
// 		//		val adder = Wire(UInt((width+1).W))
// 		//		adder := Cat(0.U(1.W), op0) + op1_new + op2_new
// 		// shift number
// 		val shn = op1(log2Ceil(width) - 1, 0)
// 		//		val shn0 = op0(log2Ceil(width)-1, 0)
// 		Map(
// 			"PASS" -> Seq(op0),
// 			"ADD" -> {
// 				Seq(op0 + op1)
// 			},
// 			"SUB" -> {
// 				Seq(op0 - op1)
// 			},
// 			"MUL" -> Seq(multipler.io.res),
// 			"MAC" -> {
// 				val adder_mac = Module(new Add(width))
// 				adder_mac.io.op0 := multipler.io.res
// 				adder_mac.io.op1 := op2
// 				Seq(adder_mac.io.res)
// 			},
// 			"UDIV" -> {
// 				udiv.io.op0 := op0
// 				udiv.io.op1 := op1
// 				Seq(udiv.io.quo)
// 			},
// 			"SDIV" -> {
// 				sdiv.io.op0 := op0
// 				sdiv.io.op1 := op1
// 				Seq(sdiv.io.quo)
// 			},
// 			"MOD" -> Seq(op0 % op1),
// 			"MIN" -> Seq(Mux(op0 < op1, op0, op1)),
// 			"MAX" -> Seq(Mux(op0 > op1, op0, op1)),
// 			"AND" -> Seq(op0 & op1),
// 			"OR" -> Seq(op0 | op1),
// 			"NOT" -> Seq(~op0),
// 			"XOR" -> Seq(op0 ^ op1),
// 			"XNOR" -> Seq(~(op0 ^ op1)),
// 			"SHL" -> Seq((op0 << shn).asUInt),
// 			"LSHR" -> Seq((op0 >> shn).asUInt),
// 			"ASHR" -> Seq((op0.asSInt >> shn).asUInt),
// 			"CSHL" -> {
// 				val res1 = (op0 << shn).asUInt
// 				val res2 = (op0 >> (width.U - shn)).asUInt
// 				Seq(res1 | res2)
// 			},
// 			"CSHR" -> {
// 				val res1 = (op0 >> shn).asUInt
// 				val res2 = (op0 << (width.U - shn)).asUInt
// 				Seq(res1 | res2)
// 			},
// 			"EQ" -> {
// 				val res = op0 === op1
// 				Seq(res, res)
// 			},
// 			"NE" -> {
// 				val res = op0 =/= op1
// 				Seq(res, res)
// 			},
// 			"ULT" -> { // unsigned less than
// 				val res = op0 < op1
// 				Seq(res, res)
// 			},
// 			"ULE" -> { // unsigned less than or equal
// 				val res = op0 <= op1
// 				Seq(res, res)
// 			},
// 			"SLT" -> { // signed less than
// 				val res = op0.asSInt < op1.asSInt
// 				Seq(res, res)
// 			},
// 			"SGE" -> { // signed less than
// 				val res = op0.asSInt >= op1.asSInt
// 				Seq(res, res)
// 			},
// 			"SGT" -> { // signed less than
// 				val res = op0.asSInt > op1.asSInt
// 				Seq(res, res)
// 			},
// 			"UGE" -> { // unsigned greater than or equal
// 				val res = op0 >= op1
// 				Seq(res, res)
// 			},
// 			"UGT" -> { // unsigned greater than
// 				val res = op0 > op1
// 				Seq(res, res)
// 			},
// 			"SLE" -> { // signed less than or equal
// 				val res = op0.asSInt <= op1.asSInt
// 				Seq(res, res)
// 			},
// 			"SEL" -> {
// 				Seq(Mux(op3.asBool, op0, op1))
// 					//Seq(Mux(op2.asBool, op1, op0))//@yuan: modify to pass the ldpc test
// 				},
// 				"SEXT" -> {
// 					val res = Cat(Fill(high, op3),op3)
// 					Seq(res, res) //@yuan: signed extension
// 				},
// 				"ZEXT" -> {
// 					val res = Cat(Fill(high, 0.U), op3)
// 					Seq(res, res) //@yuan: signed extension
// 				}

// 				,
// 				"PASS_CF" -> {
// 					val res1 = Cat (Fill (high, op0 (0) ), op0 (0) )
// 					Seq (op0, res1) //@yuan: for coarse-only
// 				},
// 					"NOT_CF" -> {
// 					val res = ~ op0
// 					val res1 = Cat (Fill (high, (res (0) ) ), res (0) )
// 					Seq (res1, res1) //@yuan: for coarse-only
// 				},
// 					"AND_CF" -> {
// 					val res = op0 & op1
// 					val res1 = Cat (Fill (high, (res (0) ) ), res (0) )
// 					Seq (res1, res1) //@yuan: for coarse-only
// 				},
// 					"OR_CF" -> {
// 					val res = op0 | op1
// 					val res1 = Cat (Fill (high, (res (0) ) ), res (0) )
// 					Seq (res1, res1) //@yuan: for coarse-only
// 				},
// 					//			"ACC"   -> Seq(op1 + op0), // the SECOND operand is the register value. the following is similar
// 					//			"ASUB"  -> Seq(op1 - op0),
// 					//			"AMUL"  -> Seq(op1 * op0),
// 					//			"ADIV"  -> Seq(op1 / op0),
// 					//			"AMOD"  -> Seq(op1 % op0),
// 					//			"AAND"  -> Seq(op1 & op0),
// 					//			"AOR"   -> Seq(op1 | op0),
// 					//			"AXOR"  -> Seq(op1 ^ op0),
// 					//			"ASHL"  -> Seq((op1 << shn0).asUInt),
// 					//			"ALSHR" -> Seq((op1 >> shn0).asUInt),
// 					//			"AASHR" -> Seq((op1.asSInt >> shn0).asUInt)
// 					)
// 				}
	def WidthGetFloat(width: Int): (Int, Int) = {
		if(width == 16){
			(5, 11)
		}else if(width == 32){
			(8, 24)
		}else{
			(11, 53)
		}
	}
	def OpFuncs(ops: Seq[UInt], cfg: UInt, hasMAC: Boolean, isFPMul: Bool): Map[String, Seq[UInt]] = {
		//		def OpFuncs(ops: Seq[UInt], opc: UInt, opset: ListBuffer[OPC]) : Map[String, Seq[UInt]] = {
		//		val op_names = opset.map(_.toString)
		val op0 = ops.head(high, 0)
		val op1 = ops(1)(high, 0)
		val op2 = {
			if (ops.size > 3) ops(2)(high, 0) // op2 is used as feedback
			else if (ops.size == 3 && hasMAC) ops(2)(high, 0)
			else 0.U(1.W)
		}
		val op3 = {
			if (ops.size > 3) ops(3)(0)
			else if (ops.size == 3 && !hasMAC) ops(2)(0)
			else 0.U(1.W)
		}
		val floatWidth = WidthGetFloat(width)
		val udiv = Module(new Div(width, false, DIV_LATENCY - 1))
		val sdiv = Module(new Div(width, true, DIV_LATENCY - 1))
		udiv.io.op0 := DontCare
		udiv.io.op1 := DontCare
		sdiv.io.op0 := DontCare
		sdiv.io.op1 := DontCare
		
		val arith = Module(new ARITH(width))
		arith.io.in(0) := op0
		arith.io.in(1) := op1
		arith.io.config := cfg

		val logic = Module(new LOGIC(width))
		logic.io.in(0) := op0
		logic.io.in(1) := op1
		logic.io.config := cfg

		val shift = Module(new SHIFT(width))
		shift.io.in(0) := op0
		shift.io.in(1) := op1
		shift.io.config := cfg


		//float-point addition/subtraction
		val fadd = Module(new FADD(floatWidth._1, floatWidth._2))
		fadd.io.a := op0
		fadd.io.b := op1
		fadd.io.rm := 0.U
		fadd.io.mode := cfg

		//float-point comparison
		val fcmp = Module(new FCMP(floatWidth._1, floatWidth._2))
		fcmp.io.a := op0
		fcmp.io.b := op1
		fcmp.io.mode := cfg

		//float-point exponent addition/subtraction
		val feas = Module(new FEAS(floatWidth._1, floatWidth._2))
		feas.io.a := op0
		feas.io.b := op1
		feas.io.mode := cfg
		
		//float-point multiplication and fixed-point multiplication
		val fmul = Module(new FMUL_share(floatWidth._1, floatWidth._2))
		fmul.io.a := op0
		fmul.io.b := op1
		fmul.io.rm := 0.U

		val multipler = Module(new Mul(width))
		val mulOp1 = Mux(isFPMul, Cat(0.U((floatWidth._1).W), fmul.io.a_sig), op0)
		val mulOp2 = Mux(isFPMul, Cat(0.U((floatWidth._1).W), fmul.io.b_sig), op1)
		multipler.io.op0 := mulOp1
		multipler.io.op1 := mulOp2
		
		fmul.io.sigProduct := multipler.io.res(2*floatWidth._2 - 1, 0) // the floating point share the same multiplier with fixed-point

		//float-point division/sqrt
//		println("lgMaxII: " + lgMaxII)		
		val fdiv = Module(new fpDiv(width))
		fdiv.io.in1 := op0
		fdiv.io.in2 := op1





		//		println("op size: " + ops.size)
		//		val op1_inv = (~op1).asUInt
		//		val op1_new = Wire(UInt(width.W))
		//		val op2_new = Wire(UInt(1.W))
		//		op1_new := op1
		//		op2_new := 0.U(1.W)
		//		if(op_names.contains("SUB")){
		//			when(opc === OPC.withName("SUB").id.U){
		//				op1_new := op1_inv
		//				op2_new := 1.U(1.W)
		//			}
		//		}
		//		if(op_names.contains("ASUB")){
		//			when(opc === OPC.withName("ASUB").id.U){
		//				op1_new := op1_inv
		//				op2_new := 1.U(1.W)
		//			}
		//		}
		//		val adder = Wire(UInt((width+1).W))
		//		adder := Cat(0.U(1.W), op0) + op1_new + op2_new
		// shift number
		val shn = op1(log2Ceil(width) - 1, 0)
		//		val shn0 = op0(log2Ceil(width)-1, 0)
				Map(
					"PASS" -> Seq(op0),
					"ADD" -> {
						Seq(arith.io.out(0), arith.io.out(1))
					},
					"SUB" -> {
						Seq(arith.io.out(0), arith.io.out(1))
					},					
					"MUL" -> {
						Seq(multipler.io.res(high, 0))},
					"MAC" -> {
						val adder_mac = Module(new Add(width))
						adder_mac.io.op0 := multipler.io.res(high, 0)
						adder_mac.io.op1 := op2
						Seq(adder_mac.io.res)
					},
					"UDIV" -> {
						udiv.io.op0 := op0
						udiv.io.op1 := op1
						Seq(udiv.io.quo)
					},
					"SDIV" -> {
						sdiv.io.op0 := op0
						sdiv.io.op1 := op1
						Seq(sdiv.io.quo)
					},
					"MOD" -> Seq(op0 % op1),
					"MIN" -> Seq(Mux(op0 < op1, op0, op1)),
					"MAX" -> Seq(Mux(op0 > op1, op0, op1)),										
					"FMIN" -> {
						//float-point comparison for fmin and fmax
						val fmin = Module(new FCMP(floatWidth._1, floatWidth._2))
						fmin.io.a := op0
						fmin.io.b := op1
						fmin.io.mode := 1.U
						Seq(Mux(fmin.io.result, op0, op1))//mode=1
					},
					"FMAX" -> {
						//float-point comparison for fmin and fmax
						val fmax = Module(new FCMP(floatWidth._1, floatWidth._2))
						fmax.io.a := op0
						fmax.io.b := op1
						fmax.io.mode := 2.U
						Seq(Mux(fmax.io.result, op0, op1)) //mode=1
					},
					"AND" -> {
						Seq(logic.io.out(0), logic.io.out(1))
					},
					"OR" -> {
						Seq(logic.io.out(0), logic.io.out(1))
					},
					"NOT" -> Seq(~op0),
					"XOR" -> {
						Seq(logic.io.out(0), logic.io.out(1))
					},
					"XNOR" -> {
						Seq(logic.io.out(0), logic.io.out(1))
					},
					"SHL" -> Seq((op0 << shn).asUInt),
					"LSHR" -> {
						Seq(shift.io.out(0))
					},
					"ASHR" -> {
						Seq(shift.io.out(0))
					},
					"CSHL" -> {
						Seq(shift.io.out(0))
					},
					"CSHR" -> {
						Seq(shift.io.out(0))
					},
					"EQ" -> {
						Seq(logic.io.out(0), logic.io.out(1))
					},
					"NE" -> {
						Seq(logic.io.out(0), logic.io.out(1))
					},
					"ULT" -> {
						Seq(arith.io.out(0), arith.io.out(1))
					},
					"ULE" -> {
						Seq(arith.io.out(0), arith.io.out(1))
					},
					"SLT" -> {
						Seq(arith.io.out(0), arith.io.out(1))
					},
					"SGE" -> {
						Seq(arith.io.out(0), arith.io.out(1))
					},
					"SGT" -> {
						Seq(arith.io.out(0), arith.io.out(1))
					},
					"UGE" -> {
						Seq(arith.io.out(0), arith.io.out(1))
					},
					"UGT" -> {
						Seq(arith.io.out(0), arith.io.out(1))
					},
					"SLE" -> {
						Seq(arith.io.out(0), arith.io.out(1))
					},
					"SEL" -> {
						Seq(Mux(op3.asBool, op0, op1))
						//Seq(Mux(op2.asBool, op1, op0))//@yuan: modify to pass the ldpc test
					},
					"SEXT" -> {
						val res = Cat(Fill(high, op3), op3)
						Seq(res, res) //@yuan: signed extension
					},
					"ZEXT" -> {
						val res = Cat(Fill(high, 0.U), op3)
						Seq(res, res) //@yuan: signed extension
					},
					"PASS_CF" -> {
						val res1 = Cat(Fill(high, op0(0)), op0(0))
						Seq(op0, res1) //@yuan: for coarse-only
					},
					"NOT_CF" -> {
						val res = ~op0
						val res1 = Cat(Fill(high, (res(0))), res(0))
						Seq(res1, res1) //@yuan: for coarse-only
					},
					"AND_CF" -> {
						val res = op0 & op1
						val res1 = Cat(Fill(high, (res(0))), res(0))
						Seq(res1, res1) //@yuan: for coarse-only
					},
					"OR_CF" -> {
						val res = op0 | op1
						val res1 = Cat(Fill(high, (res(0))), res(0))
						Seq(res1, res1) //@yuan: for coarse-only
					},
					"ARITH" -> {
						Seq(arith.io.out(0), arith.io.out(1))
					},
					"LOGIC" -> {
						Seq(logic.io.out(0), arith.io.out(1))
					},
					"SHIFT" -> {
						Seq(shift.io.out(0))
					},
					"FADDSUB" -> {
						Seq(fadd.io.result)
					},
					"FCMP" -> {
						Seq(fcmp.io.result, fcmp.io.result)
					},
					"FEAS" -> {
						Seq(feas.io.result)
					},
					"FMUL" -> {
						Seq(fmul.io.result)
					},					
					"FDIV" -> {
						Seq(fdiv.io.out)
					},					
					"FPTOSI" -> {
						//float-point to signed int
						val fp2si = Module(new FPToInt(floatWidth._1, floatWidth._2))
						fp2si.io.a := Cat(0.U((64-width).W), op0)
						fp2si.io.rm := 1.U
						fp2si.io.op := 1.U						
						Seq(fp2si.io.result(high, 0))
					},
					"SITOFP" -> {
						//float-point to signed int
						val si2fp = Module(new IntToFP(floatWidth._1, floatWidth._2))
						si2fp.io.int := Cat(0.U((64 - width).W), op0)
						si2fp.io.rm := 1.U
						si2fp.io.sign := 1.U
						si2fp.io.long := 0.U
						Seq(si2fp.io.result)
					},
					// "FDIVSQRT" -> {
					// 	Seq(fdiv.io.result)
					// },
					//			"ACC"   -> Seq(op1 + op0), // the SECOND operand is the register value. the following is similar
					//			"ASUB"  -> Seq(op1 - op0),
					//			"AMUL"  -> Seq(op1 * op0),
					//			"ADIV"  -> Seq(op1 / op0),
					//			"AMOD"  -> Seq(op1 % op0),
					//			"AAND"  -> Seq(op1 & op0),
					//			"AOR"   -> Seq(op1 | op0),
					//			"AXOR"  -> Seq(op1 ^ op0),
					//			"ASHL"  -> Seq((op1 << shn0).asUInt),
					//			"ALSHR" -> Seq((op1 >> shn0).asUInt),
					//			"AASHR" -> Seq((op1.asSInt >> shn0).asUInt)
				)
	}

	def dumpOpInfo(filename: String): Unit = {
		val infos = ListBuffer[Map[String, Any]]();
		OPSet.foreach { op =>
			val info = Map(
				"name" -> op,
				"OPC" -> OPCMap(op),
				"numOperands" -> getOperandNum(op),
				"numRes" -> getResNum(op),
				"latency" -> getLatency(op),
				"commutative" -> isCommutative(op),
				"accumulative" -> isAccumulative(op)
			)
			infos += info
		}
		val ops: mutable.Map[String, Any] = mutable.Map("Operations" -> infos)
		IRHandler.dumpIR(ops, filename)
	}

}


//object test extends App {
//	println(OPC.values)
//	println(s"OPC number: ${OPC.numOPC}")
//	println(OPC(0), OPC.withName("ADD"))
//	OPC.printOPC
//	val outFilename ="src/main/resources/operations.json"
//	OpInfo.dumpOpInfo(outFilename)
//	// println(OpInfo.OpFuncMap(OPC.ADD)(Seq(1.U, 2.U)))
//}