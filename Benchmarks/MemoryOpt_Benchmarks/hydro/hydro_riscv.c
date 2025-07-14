#include <string.h>
#include <stdio.h>
#include "include/encoding.h"
#include "include/ISA.h"

__attribute__((noinline))
void cgra_execute(void** din_addr, void** dout_addr)
{
  	long long unsigned start;
  	long long unsigned end;
	static unsigned short cin[59][3] __attribute__((aligned(16))) = {
		{0x2000, 0xfd00, 0x0028},
		{0x0000, 0x0000, 0x0029},
		{0x0000, 0x0820, 0x002a},
		{0x0004, 0x0200, 0x002b},
		{0x2000, 0xfd00, 0x0030},
		{0x0000, 0x0000, 0x0031},
		{0x0000, 0x0220, 0x0032},
		{0x0106, 0x0000, 0x0033},
		{0x0002, 0x0000, 0x0068},
		{0x000f, 0x0000, 0x00b0},
		{0x0001, 0x0001, 0x00b1},
		{0x2001, 0x0001, 0x00b9},
		{0x1a01, 0x0001, 0x00d1},
		{0x5a03, 0x0000, 0x00d9},
		{0x0000, 0x0000, 0x00f0},
		{0x0000, 0x0040, 0x00f8},
		{0x0080, 0x0000, 0x0100},
		{0x0010, 0x0000, 0x0108},
		{0x0000, 0x0000, 0x0110},
		{0x1203, 0x0001, 0x0159},
		{0x0005, 0x0000, 0x0160},
		{0x2003, 0x0000, 0x0161},
		{0x0003, 0x0000, 0x0168},
		{0x2007, 0x0000, 0x0169},
		{0x0003, 0x0000, 0x0170},
		{0x0807, 0x0000, 0x0171},
		{0x0005, 0x0000, 0x0178},
		{0x2003, 0x0000, 0x0179},
		{0x000f, 0x0000, 0x0180},
		{0x4001, 0x0000, 0x0181},
		{0x0020, 0x0000, 0x0198},
		{0x6000, 0x0000, 0x01a0},
		{0x0801, 0x0000, 0x01a8},
		{0x0800, 0x0000, 0x01b0},
		{0x0000, 0x0000, 0x01b8},
		{0x2001, 0xfd00, 0x0200},
		{0x0000, 0x0000, 0x0201},
		{0x0000, 0x0120, 0x0202},
		{0x0106, 0x0000, 0x0203},
		{0x2001, 0xfd00, 0x0208},
		{0x0000, 0x0000, 0x0209},
		{0x0000, 0x0720, 0x020a},
		{0x0004, 0x0000, 0x020b},
		{0x2003, 0xfd00, 0x0210},
		{0x0000, 0x0000, 0x0211},
		{0x0000, 0x0120, 0x0212},
		{0x0104, 0x0000, 0x0213},
		{0x2003, 0xfd00, 0x0218},
		{0x0000, 0x0000, 0x0219},
		{0x0000, 0x0020, 0x021a},
		{0x0104, 0x0000, 0x021b},
		{0x2004, 0xfd00, 0x0220},
		{0x0000, 0x0000, 0x0221},
		{0x0000, 0x0120, 0x0222},
		{0x0104, 0x0000, 0x0223},
		{0x2002, 0xfd00, 0x0228},
		{0x0000, 0x0000, 0x0229},
		{0x0000, 0x0020, 0x022a},
		{0x0104, 0x0000, 0x022b},
	};
	start = rdcycle();	
	load_cfg((void*)cin, 0xc000, 354, 0, 0);
	load_data(din_addr[0], 0x6000, 4048, 1, 0, 0, 1, 0);
	load_data(din_addr[0], 0x7000, 4048, 0, 0, 0, 1, 0);
	load_data(din_addr[1], 0x8000, 4052, 1, 0, 0, 1, 0);
	load_data(din_addr[1], 0x9000, 4052, 0, 0, 0, 1, 0);
  	volatile int result = fence(1);
  	end = rdcycle();
  	printf("It takes %d cycles for CGRA to finish loading data.\n", end - start);
  	start = rdcycle();
	config(0x0, 59, 0, 0);
  	result = fence(1);
  	end = rdcycle();
  	printf("It takes %d cycles for CGRA to finish configuration.\n", end - start);
  	start = rdcycle();
	execute(0xff0, 0, 0);
  	result = fence(1);
  	end = rdcycle();
  	printf("It takes %d cycles for CGRA to finish execution.\n", end - start);
  	start = rdcycle();
	store(dout_addr[0], 0x4000, 4048, 1, 0, 0, 1, 0);
	store(dout_addr[0], 0x5000, 4048, 0, 0, 0, 1, 0);
  	result = fence(1);
  	end = rdcycle();
  	printf("It takes %d cycles for CGRA to finish storing data.\n", end - start);
}
// #define N 8
	 #define size 1024
    int x[size]__attribute__((aligned(16)));
    int x_i[size]__attribute__((aligned(16)));
    int y[size]__attribute__((aligned(16)));
    int z[size]__attribute__((aligned(16)));
   //kernel 23
void hydro() { 
      
    int q = 15;
    int r = 5;
    int t = 8;
    for ( int k=0 ; k<1012 ; k++ ) {
            x_i[k] = q + y[k]*( r*z[k+10] + t*z[k+11] );
    } 
                 

}

void result_check()
{
  int i, j;    
    for ( int k=0 ; k<1012 ; k++ ) {
    		if (x_i[k] != x[k]) printf("There is an error in location (%d)[%d, %d]\n", k, x[k], x_i[k]);
        } 
	// if(i == 8) continue;


}


int main(){

  int i,j;
  long long unsigned start;
  long long unsigned end;

    for (int i = 0; i < size ; i++){
	    x_i[i] = 0;
		x[i] = 0;
		y[i] = i;
		z[i] = i;

	}

  printf("Initialization finished!\n");
  
//   printf("CPU add finished!\n");
  start = rdcycle();
  /* Run kernel. */
  hydro();
  end = rdcycle();
  printf("It takes %d cycles for CPU to finish the task.\n", end - start);
 
  void* cgra_din_addr[2] = {y, (void*)z+40};
  void* cgra_dout_addr[1] = {x};
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
