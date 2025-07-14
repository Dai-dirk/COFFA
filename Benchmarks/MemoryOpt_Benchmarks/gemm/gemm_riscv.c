#include <string.h>
#include <stdio.h>
#include "include/encoding.h"
#include "include/ISA.h"

__attribute__((noinline))
void cgra_execute(void** din_addr, void** dout_addr)
{
  	long long unsigned start;
  	long long unsigned end;
	static unsigned short cin[58][3] __attribute__((aligned(16))) = {
		{0x1000, 0x1000, 0x0020},
		{0x0010, 0x0010, 0x0021},
		{0x0000, 0x0a10, 0x0022},
		{0x0000, 0x0200, 0x0023},
		{0x1060, 0x1000, 0x0028},
		{0x7e10, 0x0010, 0x0029},
		{0x0000, 0x0010, 0x002a},
		{0x0158, 0x0000, 0x002b},
		{0x1040, 0x1000, 0x0030},
		{0x7e10, 0x0010, 0x0031},
		{0x0000, 0x0010, 0x0032},
		{0x0158, 0x0000, 0x0033},
		{0x0002, 0x0000, 0x0060},
		{0x1000, 0x0000, 0x0068},
		{0x1801, 0x0001, 0x00c1},
		{0x9821, 0x0000, 0x00c9},
		{0x1023, 0x0001, 0x00d1},
		{0x0923, 0x0001, 0x00d9},
		{0x2010, 0x0000, 0x00f8},
		{0x2000, 0x0001, 0x0100},
		{0x0000, 0x0007, 0x0108},
		{0x4008, 0x0000, 0x0110},
		{0x0005, 0x0000, 0x0118},
		{0x9823, 0x0000, 0x0161},
		{0x9823, 0x0000, 0x0169},
		{0x0a01, 0x0001, 0x0179},
		{0x0005, 0x0000, 0x0180},
		{0x1803, 0x0000, 0x0181},
		{0x0004, 0x0000, 0x0198},
		{0x0004, 0x0000, 0x01a0},
		{0x2000, 0x0000, 0x01a8},
		{0x2000, 0x0000, 0x01b0},
		{0x2002, 0x0000, 0x01b8},
		{0x0000, 0x0000, 0x01c0},
		{0x1020, 0x1000, 0x0200},
		{0x7e10, 0x0010, 0x0201},
		{0x0000, 0x0010, 0x0202},
		{0x0158, 0x0000, 0x0203},
		{0x1000, 0x1000, 0x0208},
		{0x7e10, 0x0010, 0x0209},
		{0x0000, 0x0010, 0x020a},
		{0x0158, 0x0000, 0x020b},
		{0x0001, 0x1000, 0x0210},
		{0x0200, 0x0010, 0x0211},
		{0x0000, 0x0010, 0x0212},
		{0x0108, 0x0000, 0x0213},
		{0x0000, 0x1000, 0x0218},
		{0x0200, 0x0010, 0x0219},
		{0x0000, 0x0010, 0x021a},
		{0x0108, 0x0000, 0x021b},
		{0x0002, 0x1000, 0x0220},
		{0x0200, 0x0010, 0x0221},
		{0x0000, 0x0010, 0x0222},
		{0x0108, 0x0000, 0x0223},
		{0x0003, 0x1000, 0x0228},
		{0x0200, 0x0010, 0x0229},
		{0x0000, 0x0010, 0x022a},
		{0x0108, 0x0000, 0x022b},
	};

  	start = rdcycle();
	load_cfg((void*)cin, 0xc000, 348, 0, 0);
	load_data(din_addr[0], 0x8000, 3984, 1, 0, 0, 2, 0);
	load_data(din_addr[0], 0x9000, 3984, 1, 0, 0, 2, 0);
	load_data(din_addr[0], 0xa000, 3984, 1, 0, 0, 2, 0);
	load_data(din_addr[0], 0xb000, 3984, 0, 0, 0, 2, 0);
	load_data(din_addr[1], 0x4000, 512, 1, 0, 0, 2, 5);
	load_data(din_addr[1], 0x5000, 512, 1, 0, 0, 2, 5);
	load_data(din_addr[1], 0x6000, 512, 1, 0, 0, 2, 5);
	load_data(din_addr[1], 0x7000, 512, 0, 0, 0, 2, 5);
  	volatile int result = fence(1);
  	end = rdcycle();
  	printf("It takes %d cycles for CGRA to finish loading data.\n", end - start);
  	start = rdcycle();
	config(0x0, 58, 0, 0);
  	result = fence(1);
  	end = rdcycle();
  	printf("It takes %d cycles for CGRA to finish configuration.\n", end - start);
  	start = rdcycle();
	execute(0xff8, 0, 0);
  	result = fence(1);
  	end = rdcycle();
  	printf("It takes %d cycles for CGRA to finish execution.\n", end - start);
  	start = rdcycle();
	store(dout_addr[0], 0x0, 4096, 0, 0, 0, 0, 0);
  	result = fence(1);
  	end = rdcycle();
  	printf("It takes %d cycles for CGRA to finish storing data.\n", end - start);
}
// #define N 8
	#define _PB_NI 32
    #define _PB_NJ 32
    #define _PB_NK 32
    int C[_PB_NI][_PB_NI]__attribute__((aligned(16)));
    int C_i[_PB_NI][_PB_NI]__attribute__((aligned(16)));
    int A[_PB_NI][_PB_NI]__attribute__((aligned(16)));
    int B[_PB_NI][_PB_NI]__attribute__((aligned(16)));
   //kernel 23
void gemm() { 
      int alpha = 5;
      for (int i = 0; i < _PB_NI; i++)
        for (int j = 0; j < _PB_NJ; j++)
	      {
	        for (int k = 0; k < _PB_NK; k= k + 4)
	          C_i[i][j] = alpha *( A[i][k] * B[k][j] +  A[i][k+1] * B[k+1][j]+  A[i][k+2] * B[k+2][j]+  A[i][k+3] * B[k+3][j]) ;
	      }
                 

}

void result_check()
{
  int i, j;


    for (int i = 0; i < _PB_NI - 1; i++)
	    for (int j = 0; j < _PB_NJ - 1; j++)
	// if(i == 8) continue;
    		if (C_i[i][j] != C[i][j]) printf("There is an error in location (%d)[%d, %d]\n", i*32+j, C_i[i][j], C[i][j]);


}


int main(){

  int i,j;
  long long unsigned start;
  long long unsigned end;
    for (int i = 0; i < _PB_NI ; i++){
	    for (int j = 0; j < _PB_NJ ; j++){
			A[i][j] = i+j;
			B[i][j] = i+j;
			C[i][j] = 0;
			C_i[i][j] = 0;
		}
	}
  printf("Initialization finished!\n");
  
//   printf("CPU add finished!\n");
  start = rdcycle();
  /* Run kernel. */
  gemm();
  end = rdcycle();
  printf("It takes %d cycles for CPU to finish the task.\n", end - start);
 
  void* cgra_din_addr[2] = {(void*)A+112, (void*)B+3584};
  void* cgra_dout_addr[1] = {C};  
//   start = rdcycle();
  cgra_execute(cgra_din_addr, cgra_dout_addr);
  volatile int result = fence(1);
//   end = rdcycle();
//   printf("It takes %d cycles for CGRA to finish the task(%d).\n", end - start, 1);
  printf("CGRA comput finished!\n");
  result = fence(1);
    
  printf("Checking results!\n");
  result_check();
  printf("Done!\n");
return 0;

}
