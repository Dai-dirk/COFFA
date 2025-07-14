#include <string.h>
#include <stdio.h>
#include "include/encoding.h"
#include "include/ISA.h"

__attribute__((noinline))
void cgra_execute(void** din_addr, void** dout_addr)
{
  	long long unsigned start;
  	long long unsigned end;
	static unsigned short cin[68][3] __attribute__((aligned(16))) = {
		{0x4002, 0x0f00, 0x0008},
		{0x0080, 0x001e, 0x0009},
		{0x0000, 0x0460, 0x000a},
		{0x2080, 0x0000, 0x000b},
		{0x4042, 0x0f00, 0x0010},
		{0x0080, 0x001e, 0x0011},
		{0x0000, 0x0260, 0x0012},
		{0x2080, 0x0000, 0x0013},
		{0x4023, 0x0f00, 0x0018},
		{0x0080, 0x001e, 0x0019},
		{0x0000, 0x0060, 0x001a},
		{0x2080, 0x0000, 0x001b},
		{0x4022, 0x0f00, 0x0020},
		{0x0080, 0x001e, 0x0021},
		{0x0000, 0x0060, 0x0022},
		{0x2080, 0x0000, 0x0023},
		{0x4021, 0x0f00, 0x0028},
		{0x0080, 0x001e, 0x0029},
		{0x0000, 0x0060, 0x002a},
		{0x2080, 0x0000, 0x002b},
		{0x4020, 0x0f00, 0x0030},
		{0x0080, 0x001e, 0x0031},
		{0x0000, 0x0060, 0x0032},
		{0x2080, 0x0000, 0x0033},
		{0x4041, 0x0f00, 0x0038},
		{0x0080, 0x001e, 0x0039},
		{0x0000, 0x0260, 0x003a},
		{0x2080, 0x0000, 0x003b},
		{0x4001, 0x0f00, 0x0040},
		{0x0080, 0x001e, 0x0041},
		{0x0000, 0x0460, 0x0042},
		{0x2080, 0x0000, 0x0043},
		{0x0000, 0x0000, 0x0050},
		{0x8000, 0x0000, 0x0058},
		{0x8000, 0x0000, 0x0060},
		{0x0000, 0x0000, 0x0068},
		{0x0000, 0x0000, 0x0070},
		{0x8000, 0x0000, 0x0078},
		{0x8000, 0x0000, 0x0080},
		{0x0000, 0x0000, 0x0088},
		{0x2001, 0x0001, 0x00f1},
		{0xb001, 0x0001, 0x00f9},
		{0x9001, 0x0000, 0x0101},
		{0x3001, 0x0001, 0x0109},
		{0x0000, 0x0000, 0x0128},
		{0x0a00, 0x0030, 0x0130},
		{0x0a00, 0x0030, 0x0138},
		{0x0000, 0x0000, 0x0140},
		{0x2c10, 0x0000, 0x0150},
		{0x8000, 0x0000, 0x0158},
		{0x0005, 0x0000, 0x01b8},
		{0x2003, 0x0000, 0x01b9},
		{0xa001, 0x0000, 0x01c1},
		{0xa001, 0x0000, 0x01c9},
		{0x2001, 0x0001, 0x01d9},
		{0x9001, 0x0000, 0x01e1},
		{0x0005, 0x0000, 0x01e8},
		{0x1003, 0x0000, 0x01e9},
		{0x0000, 0x0000, 0x0208},
		{0x0000, 0x0000, 0x0238},
		{0x4002, 0x0f00, 0x0298},
		{0x0080, 0x001e, 0x0299},
		{0x0000, 0x0e60, 0x029a},
		{0x0040, 0x0000, 0x029b},
		{0x4001, 0x0f00, 0x02c8},
		{0x0080, 0x001e, 0x02c9},
		{0x0000, 0x0e60, 0x02ca},
		{0x0040, 0x0000, 0x02cb},
	};

  	start = rdcycle();
	load_cfg((void*)cin, 0x10000, 408, 0, 0);
	load_data(din_addr[0], 0x0, 4092, 1, 0, 0, 2, 0);
	load_data(din_addr[0], 0x1000, 4092, 1, 0, 0, 2, 0);
	load_data(din_addr[0], 0x2000, 4092, 1, 0, 0, 2, 0);
	load_data(din_addr[0], 0x3000, 4092, 0, 0, 0, 2, 0);
  	volatile int result = fence(1);
  	end = rdcycle();
  	printf("It takes %d cycles for CGRA to finish loading data.\n", end - start);
  	start = rdcycle();
	config(0x0, 68, 0, 0);
  	result = fence(1);
  	end = rdcycle();
  	printf("It takes %d cycles for CGRA to finish configuration.\n", end - start);
  	start = rdcycle();
	execute(0x82ff, 0, 0);
  	result = fence(1);
  	end = rdcycle();
  	printf("It takes %d cycles for CGRA to finish execution.\n", end - start);
  	start = rdcycle();
	store(dout_addr[0], 0x8000, 3836, 1, 0, 0, 1, 0);
	store(dout_addr[0], 0x9000, 3836, 0, 0, 0, 1, 0);
  	result = fence(1);
  	end = rdcycle();
  	printf("It takes %d cycles for CGRA to finish storing data.\n", end - start);
}


#define size 32
int A[size][size]__attribute__((aligned(16)));
int B[size][size]__attribute__((aligned(16)));
int B_i[size][size]__attribute__((aligned(16)));
   //kernel 23
void kernel() { 
     int alpha = 41;
    for(int i = 1; i < 31; i++){
     for(int j =1; j < 31; j=j+2){
		B_i[i][j] = 5 * (A[i][j] + A[i][j-1] + A[i][1+j] + A[1+i][j] + A[i-1][j]);
	 	B_i[i][j+1] = 5 * (A[i][j+1] + A[i][j] + A[i][2+j] + A[1+i][j+1] + A[i-1][j+1]);
       }
    }
}

void result_check()
{
  for (int i=1 ; i < 31 ; i++) {
	for(int j =1; j < 31; j++)
    	if (B_i[i][j] != B[i][j]) printf("There is an error in location (%d)[%d, %d]\n", i*32+j, B_i[i][j], B[i][j]);  
  }
}

int main(){

//   int i,j;
  long long unsigned start;
  long long unsigned end;

   for (int i=0 ; i < 32 ; i++) {
		for(int j = 0; j < 32; j++){
    		A[i][j] = i;
    		B[i][j] = 0;
    		B_i[i][j] = 0;
		}
  }

  printf("Initialization finished!\n");
  
//   printf("CPU add finished!\n");
  start = rdcycle();
  /* Run kernel. */
  kernel();
  end = rdcycle();
  printf("It takes %d cycles for CPU to finish the task.\n", end - start);
 
  void* cgra_din_addr[2] = {A};
  void* cgra_dout_addr[1] = {(void*)B+128};		
//   start = rdcycle();
  cgra_execute(cgra_din_addr, cgra_dout_addr);
  volatile int result = fence(1);
//   end = rdcycle();
//   printf("It takes %d cycles for CGRA to finish the task(%d).\n", end - start, 1);
  printf("CGRA comput finished!\n");
  result = fence(1);

//   for (i=0;i<32; i++) {
// 	printf("%d\\", X[i]);
// 	// printf("%d", sizeof(C[i]));
// 	// printf("%s", ",,");
// 	// printf("%d\n", i);
//   }
    
  printf("Checking results!\n");
  result_check();
  printf("Done!\n");

return 0;

}
