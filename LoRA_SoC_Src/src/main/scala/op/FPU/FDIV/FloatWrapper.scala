package fgramemfp.op

import chisel3._
import chisel3.util._

// Wraps a Chisel Flo or Dbl datatype to allow easy
// extraction of the different parts (sign, exponent, mantissa)

class FloatWrapper(val num: UInt) {  // 使用 UInt 类型代替 Bits
  val (sign, exponent, mantissa, zero) = num.getWidth match {
    case 32 =>
      (
        num(31),  // sign: 第 32 位
        num(30, 23),  // exponent: 23 位指数
        // mantissa: 如果指数部分为 0，表示这是一个非规格化数，使用 Mux 进行判断
        Cat(Mux(num(30, 23) === 0.U, 0.U(1.W), 1.U(1.W)), num(22, 0)),
        num(30, 0) === 0.U  // zero: 判断 num 是否全为 0
      )
    case 64 =>
      (
        num(63),  // sign: 第 64 位
        num(62, 52),  // exponent: 11 位指数
        // mantissa: 如果指数部分为 0，表示这是一个非规格化数，使用 Mux 进行判断
        Cat(Mux(num(62, 52) === 0.U, 0.U(1.W), 1.U(1.W)), num(51, 0)),
        num(62, 0) === 0.U  // zero: 判断 num 是否全为 0
      )
  }
}