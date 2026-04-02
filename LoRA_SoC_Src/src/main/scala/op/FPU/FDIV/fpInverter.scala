
// Süleyman Savas, 2016-12-15
// Halmstad University

package fgramemfp.op

import chisel3._
import chisel3.util._
//import chisel3.iotesters.{PeekPokeTester, Driver, ChiselFlatSpec}

class fpInverter(val w : Int) extends Module {
	val io = IO(new Bundle{
		val in1 = Input(UInt(w.W))
		val out = Output(UInt((w+1).W))

	})

	// instantiate lookup tables
	val tableC = Module(new lookupC())
	val tableL = Module(new lookupL())
	val tableJ = Module(new lookupJ())
	
	// Most significant 9 bits of the input (mantissa) is used as address
	// to the coefficient lookup tables
	val coeffAddr   = io.in1(w - 1, w - 6)
	tableC.io.addr := coeffAddr
	tableL.io.addr := coeffAddr
	tableJ.io.addr := coeffAddr

	val sub1 = (io.in1 ^ "b11111111111111111111111".U) + 1.U

	val mul1 = Module(new VarSizeMul(23, 17, 24))
	mul1.io.in1 := tableJ.io.out
	mul1.io.in2 := io.in1(w - 7, 0)
	// result will be input to adder1

	val mul2 = Module(new mul2(24, 17, 29)) 
	mul2.io.in1 := tableC.io.out
	mul2.io.in2 := (io.in1(w - 7, 0) * io.in1(w - 7, 0))(33, 17)
	// result will be input to sub2

	// workaround registers (sub1 reg, mul1 reg and mul2 reg does not delay their input)
	val w_sub1_reg = RegNext(sub1)
	val w_mul1_reg = RegNext(mul1.io.out)
	val w_mul2_reg = RegNext(mul2.io.out)
	//val w_tableL_reg = Reg(init = 0.U, next = tableL.io.out)
/*
	// stage1 registers
	val tableL_out_reg = Reg(init = 0.U, next = tableL.io.out)
	val sub1_out_reg1  = Reg(init = 0.U, next = sub1)
	val mul1_out_reg   = Reg(init = 0.U, next = mul1.io.out)
	val mul2_out_reg   = Reg(init = 0.U, next = mul2.io.out)
*/
	//stage1 registers with workaround
	val tableL_out_reg = RegNext(tableL.io.out)
//	val tableL_out_reg = Reg(init = 0.U, next = w_tableL_reg)
	val sub1_out_reg1  = RegNext(w_sub1_reg)
	val mul1_out_reg   = RegNext(w_mul1_reg)
	val mul2_out_reg   = RegNext(w_mul2_reg)


// using sub due to the sign value of the j coefficients
	val sub2     = Module(new VarSizeSub(27, 27, 27))
	val sub2_in2 = (mul1_out_reg ^ "b111111111111111111111111".U) + 1.U
	val temp3    = Cat(sub2_in2, "b0".U)
	val temp4    = Cat(temp3, "b0".U)
	sub2.io.in2 := Cat(temp4, "b0".U)
	sub2.io.in1 := tableL_out_reg
	// result will be input to sub2

//	using an adder due to the sign value of the c coefficients
	val adder     = Module(new VarSizeAdder(29, 29, 25))
	val temp1     = Cat(sub2.io.out, "b0".U)
	val temp2     = Cat(temp1, "b0".U)
	adder.io.in1 := temp2
	adder.io.in2 := mul2_out_reg
	// result will be input to mul3

	//stage2 registers
	val sub1_out_reg2 = RegNext(sub1_out_reg1)
	val adder_out_reg = RegNext(adder.io.out)

	val mul3     = Module(new mul3(w, 25, 24))
	mul3.io.in1 := sub1_out_reg2
	mul3.io.in2 := adder_out_reg

	//val outReg = Reg(init = UInt(0, width = w + 1), next = mul3.io.out)
	//io.out := outReg

	io.out := mul3.io.out

/*
	printf("\ninput: %d\n", io.in1)
	printf("z: %d\n", Cat(1.U, mul3.io.out))
	printf("sub1 output: %d\n", sub1)
	printf("sub1 reg1: %d reg2: %d\n", sub1_out_reg1, sub1_out_reg2)
	printf("mul3 output: %d\n", mul3.io.out)
	printf("adder output : %d\n", adder.io.out)
	printf("adder_reg out: %d\n", adder_out_reg)
	printf("sub2 output : %d\n", sub2.io.out)
	printf("mul2 output : %d\n", mul2.io.out)
	printf("mul2_reg out: %d\n", mul2_out_reg)
	printf("squarer output : %d \n", (io.in1(w - 7, 0) * io.in1(w - 7, 0))(33, 17))
	printf("mul1 output : %d\n", mul1.io.out)
	printf("mul1_reg out: %d\n", mul1_out_reg)
	printf("L: %d, C: %d, J: %d\n", tableL.io.out, tableC.io.out, tableJ.io.out)
	printf("L_reg :%d\n", tableL_out_reg)
*/
}

