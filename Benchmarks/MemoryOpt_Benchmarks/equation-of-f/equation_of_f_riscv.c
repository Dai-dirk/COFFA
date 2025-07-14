#include <string.h>
#include <stdio.h>
#include "include/encoding.h"
#include "include/ISA.h"

__attribute__((noinline))
void cgra_execute(void** din_addr, void** dout_addr)
{
  	long long unsigned start;
  	long long unsigned end;
	static unsigned short cin[72][3] __attribute__((aligned(16))) = {
		{0x2002, 0xe300, 0x0008},
		{0x0003, 0x0000, 0x0009},
		{0x0000, 0x0020, 0x000a},
		{0x20c0, 0x0000, 0x000b},
		{0x2001, 0xe300, 0x0018},
		{0x0003, 0x0000, 0x0019},
		{0x0000, 0x0020, 0x001a},
		{0x20c0, 0x0000, 0x001b},
		{0x2004, 0xe300, 0x0020},
		{0x0003, 0x0000, 0x0021},
		{0x0000, 0x0020, 0x0022},
		{0x20c0, 0x0000, 0x0023},
		{0x2000, 0xe300, 0x0028},
		{0x0003, 0x0000, 0x0029},
		{0x0000, 0x0020, 0x002a},
		{0x20c0, 0x0000, 0x002b},
		{0x2003, 0xe300, 0x0030},
		{0x0003, 0x0000, 0x0031},
		{0x0000, 0x0020, 0x0032},
		{0x20c0, 0x0000, 0x0033},
		{0x2006, 0xe300, 0x0038},
		{0x0003, 0x0000, 0x0039},
		{0x0000, 0x0020, 0x003a},
		{0x20c0, 0x0000, 0x003b},
		{0x2005, 0xe300, 0x0040},
		{0x0003, 0x0000, 0x0041},
		{0x0000, 0x0020, 0x0042},
		{0x20c0, 0x0000, 0x0043},
		{0x0000, 0x0000, 0x0050},
		{0x0000, 0x0000, 0x0060},
		{0x8000, 0x0000, 0x0068},
		{0x0000, 0x0000, 0x0070},
		{0x0000, 0x0000, 0x0078},
		{0x8000, 0x0001, 0x0080},
		{0x0000, 0x0000, 0x0088},
		{0x4401, 0x0002, 0x00e1},
		{0xa041, 0x0001, 0x00e9},
		{0xa001, 0x0000, 0x0101},
		{0x3481, 0x0002, 0x0109},
		{0xb081, 0x0001, 0x0111},
		{0x0040, 0x0000, 0x0130},
		{0x0000, 0x0020, 0x0138},
		{0x0100, 0x0020, 0x0140},
		{0x0080, 0x0010, 0x0148},
		{0x0080, 0x0010, 0x0150},
		{0x0004, 0x0010, 0x0158},
		{0x8880, 0x0000, 0x0160},
		{0x1001, 0x0002, 0x01b9},
		{0x0003, 0x0000, 0x01c0},
		{0x8003, 0x0000, 0x01c1},
		{0x0005, 0x0000, 0x01d0},
		{0x1003, 0x0000, 0x01d1},
		{0x0003, 0x0000, 0x01d8},
		{0x1003, 0x0000, 0x01d9},
		{0x1401, 0x0001, 0x01e1},
		{0xa041, 0x0000, 0x01e9},
		{0x0005, 0x0000, 0x01f0},
		{0x1003, 0x0000, 0x01f1},
		{0x0000, 0x0000, 0x0208},
		{0x0000, 0x0000, 0x0238},
		{0x2800, 0xe300, 0x0290},
		{0x0003, 0x0000, 0x0291},
		{0x0000, 0x0020, 0x0292},
		{0x2000, 0x0000, 0x0293},
		{0x2400, 0xe300, 0x0298},
		{0x0003, 0x0000, 0x0299},
		{0x0000, 0x0020, 0x029a},
		{0x2000, 0x0000, 0x029b},
		{0x2000, 0xe300, 0x02c8},
		{0x0003, 0x0000, 0x02c9},
		{0x0000, 0x1420, 0x02ca},
		{0x0000, 0x0000, 0x02cb},
	};

  	start = rdcycle();
	load_cfg((void*)cin, 0x10000, 432, 0, 0);
	load_data(din_addr[0], 0x0, 4004, 1, 0, 0, 3, 0);
	load_data(din_addr[0], 0x1000, 4004, 1, 0, 0, 3, 0);
	load_data(din_addr[0], 0x2000, 4004, 1, 0, 0, 3, 0);
	load_data(din_addr[0], 0x3000, 4004, 1, 0, 0, 3, 0);
	load_data(din_addr[0], 0x4000, 4004, 1, 0, 0, 3, 0);
	load_data(din_addr[0], 0x5000, 4004, 1, 0, 0, 3, 0);
	load_data(din_addr[0], 0x6000, 4004, 1, 0, 0, 3, 0);
	load_data(din_addr[0], 0x7000, 4004, 0, 0, 0, 3, 0);
	load_data(din_addr[1], 0x9000, 3980, 0, 0, 0, 0, 0);
	load_data(din_addr[2], 0xa000, 3980, 0, 0, 0, 0, 0);
  	volatile int result = fence(1);
  	end = rdcycle();
  	printf("It takes %d cycles for CGRA to finish loading data.\n", end - start);
  	start = rdcycle();
	config(0x0, 72, 0, 0);
  	result = fence(1);
  	end = rdcycle();
  	printf("It takes %d cycles for CGRA to finish configuration.\n", end - start);
  	start = rdcycle();
	execute(0x83fd, 0, 0);
  	result = fence(1);
  	end = rdcycle();
  	printf("It takes %d cycles for CGRA to finish execution.\n", end - start);
  	start = rdcycle();
	store(dout_addr[0], 0x8000, 3980, 0, 0, 0, 0, 0);
  	result = fence(1);
  	end = rdcycle();
  	printf("It takes %d cycles for CGRA to finish storing data.\n", end - start);
}


int x_i[1001]__attribute__((aligned(16)));
int x[1001]__attribute__((aligned(16)));
int u[1001]__attribute__((aligned(16)));
int z[1001]__attribute__((aligned(16)));
int y[1001]__attribute__((aligned(16)));
void kernel(){ 
   int t = 1;
   int r = 3;
   int q = 5;
   int n = 995;
   for ( int k=0 ; k<n ; k++ ) {
        x_i[k] = u[k] + r*( z[k] + r*y[k] ) +
               t*( u[k+3] + r*( u[k+2] + r*u[k+1] ) +
                  t*( u[k+6] + q*( u[k+5] + q*u[k+4] ) ) );
   }
}

void result_check()
{
  int i, j;
  int n = 995;
  for ( int k=0 ; k<n ; k++ ) {
    if (x_i[k] != x[k]) printf("There is an error in location (%d)[%d, %d]\n", k, x_i[k], x[k]);
  } 
}

int main(){

  int i,j;
  long long unsigned start;
  long long unsigned end;

  for (i=0;i<1001; i++){
      x[i] = i ;
    //   x_i[i] = i;
      u[i] = i ;
      z[i] = i ;
      y[i] = i ;
    }
  // for(int j = 0; j< 4; j++){
  //   for(int i=0; i< 512; i++){
  //     output[j][i] = 0;
  //   }
  // }
//   for(int i=0; i< Ncb; i++){
//       W[i] = 0;
//       W_i[i] = 0;
//   }
// for (i=0;i<SIZE; i++) printf("%d\n", C[i]);

  printf("Initialization finished!\n");
  
  // printf("CPU add finished!\n");
  start = rdcycle();
  /* Run kernel. */
  kernel();
  end = rdcycle();
  printf("It takes %d cycles for CPU to finish the task.\n", end - start);
 
  void* cgra_din_addr[3] = {u, y, z};
  void* cgra_dout_addr[1] = {x};
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
