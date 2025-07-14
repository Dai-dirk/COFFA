#include <string.h>
#include <stdio.h>
#include "include/encoding.h"
#include "include/ISA.h"

__attribute__((noinline))

void cgra_execute(void** din_addr, void** dout_addr)
{
  	long long unsigned start;
  	long long unsigned end;
	static unsigned short cin[49][3] __attribute__((aligned(16))) = {
		{0x1001, 0x0f80, 0x0008},
		{0x0020, 0x0010, 0x0009},
		{0x0000, 0x0020, 0x000a},
		{0x0104, 0x0000, 0x000b},
		{0x1000, 0x0f80, 0x0018},
		{0x0020, 0x0010, 0x0019},
		{0x0000, 0x0620, 0x001a},
		{0x0004, 0x0200, 0x001b},
		{0x1000, 0x0f80, 0x0020},
		{0x0020, 0x0010, 0x0021},
		{0x0000, 0x0020, 0x0022},
		{0x0104, 0x0000, 0x0023},
		{0x1000, 0x0f80, 0x0028},
		{0x0020, 0x0010, 0x0029},
		{0x0000, 0x0820, 0x002a},
		{0x0008, 0x0200, 0x002b},
		{0x1000, 0x0f80, 0x0030},
		{0x0020, 0x0010, 0x0031},
		{0x0000, 0x0220, 0x0032},
		{0x0108, 0x0000, 0x0033},
		{0x0000, 0x0000, 0x0048},
		{0x0000, 0x0001, 0x0050},
		{0x0000, 0x0000, 0x0058},
		{0x0400, 0x0000, 0x0060},
		{0x1801, 0x0000, 0x0068},
		{0x0823, 0x0001, 0x00b9},
		{0x1022, 0x0001, 0x00c9},
		{0xd042, 0x0000, 0x00d1},
		{0x0000, 0x0010, 0x00f0},
		{0x0104, 0x0020, 0x00f8},
		{0xe000, 0x0020, 0x0100},
		{0x2000, 0x0031, 0x0108},
		{0x0000, 0x0005, 0x0110},
		{0x0004, 0x0000, 0x0118},
		{0x1203, 0x0001, 0x0159},
		{0x8803, 0x0000, 0x0169},
		{0x9903, 0x0000, 0x0171},
		{0x0000, 0x0000, 0x0198},
		{0x0000, 0x0000, 0x01a8},
		{0x0000, 0x0000, 0x01b0},
		{0x2000, 0x0000, 0x01b8},
		{0x1001, 0x0f80, 0x0200},
		{0x0020, 0x0010, 0x0201},
		{0x0000, 0x0020, 0x0202},
		{0x0108, 0x0000, 0x0203},
		{0x1000, 0x0f80, 0x0220},
		{0x0020, 0x0010, 0x0221},
		{0x0000, 0x0020, 0x0222},
		{0x0100, 0x0000, 0x0223},
	};

  	start = rdcycle();
	load_cfg((void*)cin, 0xc000, 294, 0, 0);
	load_data(din_addr[0], 0x8000, 4092, 0, 0, 0, 0, 0);
	load_data(din_addr[1], 0x0, 4096, 1, 0, 0, 1, 0);
	load_data(din_addr[1], 0x1000, 4096, 0, 0, 0, 1, 0);
	load_data(din_addr[2], 0x4000, 4096, 1, 0, 0, 2, 0);
	load_data(din_addr[2], 0x5000, 4096, 1, 0, 0, 2, 0);
	load_data(din_addr[2], 0x6000, 4096, 1, 0, 0, 2, 0);
	load_data(din_addr[2], 0x7000, 4096, 0, 0, 0, 2, 0);
  	volatile int result = fence(1);
  	end = rdcycle();
  	printf("It takes %d cycles for CGRA to finish loading data.\n", end - start);
  	start = rdcycle();
	config(0x0, 49, 0, 0);
  	result = fence(1);
  	end = rdcycle();
  	printf("It takes %d cycles for CGRA to finish configuration.\n", end - start);
  	start = rdcycle();
	execute(0x47d, 0, 0);
  	result = fence(1);
  	end = rdcycle();
  	printf("It takes %d cycles for CGRA to finish execution.\n", end - start);
  	start = rdcycle();
	store(dout_addr[0], 0x4000, 4096, 1, 0, 0, 2, 0);
	store(dout_addr[0], 0x5000, 4096, 1, 0, 0, 2, 0);
	store(dout_addr[0], 0x6000, 4096, 1, 0, 0, 2, 0);
	store(dout_addr[0], 0x7000, 4096, 0, 0, 0, 2, 0);
	store(dout_addr[1], 0x0, 4096, 1, 0, 0, 1, 0);
	store(dout_addr[1], 0x1000, 4096, 0, 0, 0, 1, 0);
  	result = fence(1);
  	end = rdcycle();
  	printf("It takes %d cycles for CGRA to finish storing data.\n", end - start);
}
// #define N 8
#define NUM 32
  int A[NUM][NUM]__attribute__((aligned(16)));
  int B[NUM][NUM]__attribute__((aligned(16)));
  int X[NUM][NUM]__attribute__((aligned(16)));
  int B_i[NUM][NUM]__attribute__((aligned(16)));
  int X_i[NUM][NUM]__attribute__((aligned(16)));
   //kernel 23
void adi() { 
    for (int i1=0 ; i1 < NUM ; i1++) {
        for (int i2=0 ; i2 < NUM-1 ; i2++){
	    	X_i[i1][i2] = X_i[i1][i2] - X_i[i1][i2+1] * A[i1][i2] * B_i[i1][i2+1];
	    	B_i[i1][i2] = B_i[i1][i2] - A[i1][i2] * A[i1][i2] * B_i[i1][i2+1];
 		} 
 	}
}

void result_check()
{
  for (int i=0 ; i < NUM ; i++) {
          for (int j=0 ; j < NUM-1 ; j++){
    		if (X_i[i][j] != X[i][j]) printf("There is an error in location (%d)[%d, %d]\n", i*NUM+j, X_i[i][j], X[i][j]);
        } 
  }
	// if(i == 8) continue;


}


int main(){

//   int i,j;
  long long unsigned start;
  long long unsigned end;

   for (int i=0 ; i < NUM ; i++) {
          for (int j=0 ; j < NUM ; j++){
    		A[i][j] = i + j;
			B[i][j] = i + j;
			X[i][j] = i + j;
			B_i[i][j] = i + j;
			X_i[i][j] = i + j;
        } 
  }

  printf("Initialization finished!\n");
  
//   printf("CPU add finished!\n");
  start = rdcycle();
  /* Run kernel. */
  adi();
  end = rdcycle();
  printf("It takes %d cycles for CPU to finish the task.\n", end - start);
 
  void* cgra_din_addr[3] = {A, B, X};
  void* cgra_dout_addr[2] = {X, B};
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
