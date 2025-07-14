/*************************************************
 * Define your hardware-implemented operations in the "User-Defined Section" below.
 * 
 * Author: Mion Lou, Date: 202504 
 */

#ifndef APP_COMPILER_HARDWARE_DEFINED_OPS_H
#define APP_COMPILER_HARDWARE_DEFINED_OPS_H
void please_map_me();
void* __hardware_imp__();

/***********************************
 * We wrap a CGRA hardware defined op as a function. 
 * It is annotated with three attributes:
 *  1. annotate("___CGRA_HARDWARE_OP___"): Marks the function as a hardware-implemented operation for LLVM analysis.
 *  2. noinline: Prevents the compiler from inlining this function.
 *  3. optnone: Disables all optimizations on this function (including remove inputs and outputs of the function).
 * *********************************
 */
#define __CGRA_HARDWARE_OP(x) __attribute__((annotate("___CGRA_HARDWARE_OP__"x"___"), noinline, optnone))

//=================================
// User-Defined Section:
// You may define your own functions below. 
// Do NOT modify any other part of this header file.
//=================================
/* === How to define a hardware-implemented operation ===
 * You can wrap a hardware operation as a C function (for example, "myexp") by following these steps:
 *
 * 1. Use the macro __CGRA_HARDWARE_OP("op_name") to annotate the function:
 *      __CGRA_HARDWARE_OP("myexp")
 *    This macro helps the compiler and downstream tools recognize the operation as "myexp" in the CDFG.
 *
 * 2. Define the function using the macro. The input and return values of the function represent the operands and result of the hardware operation:
 *      __CGRA_HARDWARE_OP("myexp") int Myexp(int x)
 *
 * 3. Define a dummy body for the function to satisfy compilation. Use the virtual hardware access call:
 *      { return *(int *)__hardware_imp__(); }
 *
 * A complete definition looks like this:
 *      __CGRA_HARDWARE_OP("myexp") int Exp(int x) { return *(int *)__hardware_imp__(); }
 */
 __CGRA_HARDWARE_OP("myexp") int Mydef(int x){ return *(int *)__hardware_imp__();}

//=================================
// End of User-Defined Section
// DO NOT modify anything beyond this point.
//=================================
#endif // APP_COMPILER_HARDWARE_DEFINED_OPS_H