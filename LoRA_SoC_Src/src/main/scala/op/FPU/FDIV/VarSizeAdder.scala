
// Süleyman Savas, 2016-12-16
// Halmstad University

package fgramemfp.op

import chisel3._
import chisel3.util._
 
class VarSizeAdder(val size1 : Int, val size2 : Int, val size3 : Int) extends Module {
// tested and works fine

	val io = IO(new Bundle{
		val in1 = Input(UInt(size1.W))
		val in2 = Input(UInt(size2.W))
		val out = Output(UInt(size3.W))
	})

	
	//io.out := io.in1 + io.in2 //this takes ls bits whereas we want ms bits
	io.out := (io.in1 + io.in2)(size1 - 1, size1 - size3)
	//printf("Adder in1: %d , in2: %d out: %d\n", io.in1, io.in2, io.in1 + io.in2)
}

class VarSizeSub(val size1 : Int, val size2 : Int, val size3 : Int) extends Module {
// tested and works fine

	val io = IO(new Bundle{
		val in1 = Input(UInt(size1.W))
		val in2 = Input(UInt(size2.W))
		val out = Output(UInt(size3.W))
	})


	//io.out := Cat((io.in1 + io.in2)(size1 - 1 , 0), "b00".U)
	io.out := (io.in1 + io.in2)
//	printf("Adder in1: %d , in2: %d out: %d\n", io.in1, io.in2, io.out)
}
