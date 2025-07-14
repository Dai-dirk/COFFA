#include <string.h>
#include <stdio.h>
#include "include/encoding.h"
#include "include/ISA.h"

__attribute__((noinline))
void cgra_execute(void** din_addr, void** dout_addr)
{
	long long unsigned start;
	long long unsigned end;
	static unsigned short cin[76][3] __attribute__((aligned(16))) = {
		{0x8000, 0x0400, 0x0018},
		{0x0080, 0x0040, 0x0019},
		{0x0000, 0x0060, 0x001a},
		{0x2000, 0x0000, 0x001b},
		{0x8401, 0x0400, 0x0028},
		{0x0080, 0x0040, 0x0029},
		{0x0000, 0x0060, 0x002a},
		{0x2000, 0x0000, 0x002b},
		{0x0c00, 0x0400, 0x0030},
		{0x0020, 0x0040, 0x0031},
		{0x0000, 0x1460, 0x0032},
		{0x0000, 0x0000, 0x0033},
		{0x0001, 0x0000, 0x0034},
		{0x0800, 0x0400, 0x0040},
		{0x0020, 0x0040, 0x0041},
		{0x0000, 0x0a60, 0x0042},
		{0x2000, 0x0000, 0x0043},
		{0x0000, 0x0000, 0x0060},
		{0x6000, 0x0000, 0x0068},
		{0x0100, 0x0000, 0x0070},
		{0x0000, 0x0000, 0x0078},
		{0x0000, 0x0000, 0x0080},
		{0x0000, 0x0000, 0x0088},
		{0xb483, 0x0000, 0x00f1},
		{0xb001, 0x0000, 0x0101},
		{0x9043, 0x0001, 0x0109},
		{0x2081, 0x0002, 0x0111},
		{0x0030, 0x0000, 0x0138},
		{0x0000, 0x0002, 0x0140},
		{0x0210, 0x0000, 0x0148},
		{0x0840, 0x0010, 0x0150},
		{0x0080, 0x0000, 0x0158},
		{0x0000, 0x0000, 0x0160},
		{0x4043, 0x0002, 0x01c9},
		{0x3043, 0x0002, 0x01d1},
		{0xa401, 0x0000, 0x01d9},
		{0x9401, 0x0000, 0x01e1},
		{0x1021, 0x0000, 0x01e9},
		{0x0000, 0x0600, 0x01ea},
		{0x0200, 0x0004, 0x01eb},
		{0x0008, 0x0000, 0x01ec},
		{0x8000, 0x0000, 0x0208},
		{0x2000, 0x0001, 0x0210},
		{0x0008, 0x0000, 0x0218},
		{0x0002, 0x0000, 0x0220},
		{0x1800, 0x0000, 0x0228},
		{0x0800, 0x0000, 0x0230},
		{0x8403, 0x0400, 0x0290},
		{0x0080, 0x0040, 0x0291},
		{0x0000, 0x0060, 0x0292},
		{0x2000, 0x0000, 0x0293},
		{0x4000, 0x0400, 0x0298},
		{0x0040, 0x0040, 0x0299},
		{0x0000, 0x0060, 0x029a},
		{0x2050, 0x0000, 0x029b},
		{0x4000, 0x0400, 0x02a0},
		{0x0040, 0x0040, 0x02a1},
		{0x0000, 0x0260, 0x02a2},
		{0x2050, 0x0000, 0x02a3},
		{0x8002, 0x0400, 0x02a8},
		{0x0080, 0x0040, 0x02a9},
		{0x0000, 0x0060, 0x02aa},
		{0x2000, 0x0000, 0x02ab},
		{0x4001, 0x0400, 0x02b0},
		{0x0040, 0x0040, 0x02b1},
		{0x0000, 0x0e60, 0x02b2},
		{0x0050, 0x0000, 0x02b3},
		{0x0001, 0x0000, 0x02b4},
		{0x4001, 0x0400, 0x02b8},
		{0x0040, 0x0040, 0x02b9},
		{0x0000, 0x0060, 0x02ba},
		{0x2050, 0x0000, 0x02bb},
		{0x4001, 0x0400, 0x02c0},
		{0x0040, 0x0040, 0x02c1},
		{0x0000, 0x0260, 0x02c2},
		{0x2050, 0x0000, 0x02c3},
	};
	start = rdcycle();
	load_cfg((void*)cin, 0x10000, 456, 0, 0);
	load_data(din_addr[0], 0x0, 4096, 1, 0, 0, 0, 0);
	load_data(din_addr[1], 0x1000, 4096, 1, 0, 0, 0, 0);
	load_data(din_addr[2], 0x8000, 4096, 1, 0, 0, 0, 0);
	load_data(din_addr[3], 0x9000, 4096, 0, 0, 0, 0, 0);
	load_data(din_addr[4], 0x2000, 256, 0, 0, 0, 0, 0);
	load_data(din_addr[5], 0xa000, 2048, 1, 0, 0, 1, 0);
	load_data(din_addr[5], 0xb000, 2048, 0, 0, 0, 1, 0);
	volatile int result = fence(1);
	end = rdcycle();
	printf("It takes %d cycles for CGRA to finish loading data.\n", end - start);
	start = rdcycle();
	config(0x0, 76, 0, 0);
	result = fence(1);
	end = rdcycle();
	printf("It takes %d cycles for CGRA to finish configuration.\n", end - start);
	start = rdcycle();
	execute(0x7fb4, 0, 0);
	result = fence(1);
	end = rdcycle();
	printf("It takes %d cycles for CGRA to finish execution.\n", end - start);
	start = rdcycle();
	store(dout_addr[0], 0x3000, 256, 0, 0, 0, 0, 0);
	store(dout_addr[1], 0xa000, 2048, 1, 0, 0, 1, 0);
	store(dout_addr[1], 0xb000, 2048, 0, 0, 0, 1, 0);
	result = fence(1);
	end = rdcycle();
	printf("It takes %d cycles for CGRA to finish storing data.\n", end - start);
}
#define NPOINTS 64
// #define NSECTIONS_o 8
#define NSECTIONS 4

int input[NPOINTS]__attribute__((aligned(16)));
int output[NPOINTS]__attribute__((aligned(16)));
int output_i[NPOINTS]__attribute__((aligned(16)));
int coefficient[NPOINTS][NSECTIONS][NSECTIONS]__attribute__((aligned(16)));
int internal_state[NPOINTS][NSECTIONS][2]__attribute__((aligned(16)));
int internal_state_i[NPOINTS][NSECTIONS][2]__attribute__((aligned(16)));

void kernel()
/* input:           input sample array */
/* output:          output sample array */
/* coefficient:     coefficient array */
/* internal_state:  internal state array */
{
  int i, imod8, imodNSECTIONS;
  int j;

  int state_2, state_1;
  int coef_a21, coef_a11, coef_b21, coef_b11;
  int sum;

  for (i = 0; i < NPOINTS; ++i) {
    sum = input[i];
    for (j = 0; j < NSECTIONS; ++j) {


      state_2 = internal_state_i[i][j][0];
      state_1 = internal_state_i[i][j][1];

      sum += internal_state_i[i][j][0] * coefficient[i][j][0] +
				internal_state_i[i][j][1] * coefficient[i][j][1];

    //   internal_state_i[i][j][0] = internal_state_i[i][j][1];
      internal_state_i[i][j][1] = sum;

      sum += state_2 * coefficient[i][j][2] + state_1 * coefficient[i][j][3];

    }
    output_i[i] = sum;
  }
}

void result_check()
{
  for(int i = 0; i < NPOINTS; i++){
    if (output_i[i] != output[i]) printf("There is an error in location (%d)[%d, %d]\n", i, output_i[i], output[i]);
  }

}


int main(){

  int i,j,k;
  long long unsigned start;
  long long unsigned end;

    
  for(i = 0; i< NPOINTS; i++){
    input[i] = i;
    output[i] = i;
    output_i[i] = i;
  }
  for(i = 0; i < NPOINTS; i++){
    for(j = 0; j < NSECTIONS; j++){
      for(k = 0; k < NSECTIONS; k++){
        coefficient[i][j][k] = i + j + k;
      }
    }
  }
  for(i = 0; i < NPOINTS; i++){
    for(j = 0; j < NSECTIONS; j++){
      for(k = 0; k < 2; k++){
        internal_state[i][j][k] = i + j + k;
        internal_state_i[i][j][k] = i + j + k;
      }
    }
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
 
  void* cgra_din_addr[6] = {coefficient, coefficient, coefficient, coefficient, input, internal_state};
  void* cgra_dout_addr[2] = {output, internal_state};
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
