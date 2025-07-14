/* 4-cascaded IIR biquad filter processing 64 points */
/* Modified to use arrays - SMP */

#define NPOINTS 64
// #define NSECTIONS_o 8
#define NSECTIONS 4

volatile int input[NPOINTS];
volatile int output[NPOINTS];
volatile int coefficient[NPOINTS][NSECTIONS][NSECTIONS];
volatile int internal_state[NPOINTS][NSECTIONS][2];

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

    #ifdef CGRA_COMPILER
    	loop_begin();
    #endif 
  for (i = 0; i < NPOINTS; ++i) {
    sum = input[i];
    for (j = 0; j < NSECTIONS; ++j) {


      state_2 = internal_state[i][j][0];
      state_1 = internal_state[i][j][1];

      sum += internal_state[i][j][0] * coefficient[i][j][0] +
				internal_state[i][j][1] * coefficient[i][j][1];

      //internal_state[i][j][0] = internal_state[i][j][1];
      internal_state[i][j][1] = sum;

      sum += state_2 * coefficient[i][j][2] + state_1 * coefficient[i][j][3];

    }
    output[i] = sum;
  }
    #ifdef CGRA_COMPILER
    	loop_end();
    #endif 
}
