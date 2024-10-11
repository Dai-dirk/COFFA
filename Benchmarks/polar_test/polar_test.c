#include <stdio.h>

#define BOUND 10000
#define upper 15
#define lower 5
volatile int para0 = 0; 
volatile int para1 = 32; 
int array0[32][64];
int array1[32][64];
int array2[32][64];
int array3[32][64];
// granularity: 0 represents 1 word; 1 represents 2 words
void kernel()
{
    loop_begin();
  for (int i = 0; i < 32; i++) {
            int a3_temp = 0;
        for (int j = i + 1; j < 63; j++) {
            int a2ij = array2[i][j];	
            int a2ji = array2[i][j + 1];	
            if (a2ij >= BOUND && a2ji < BOUND) {
                a3_temp += a2ij * upper;
            } else {
                if (a2ij < BOUND && a2ji < BOUND) {
                    a3_temp += a2ji * array0[i][j + 1];
                } else {
                    a3_temp += array0[i][j] * lower;
                }
            }
            array3[i][j] = a3_temp * 2;
        }
    }
    loop_end();
}
