#define __CGRA_HARDWARE_OP(x) __attribute__((annotate("___CGRA_HARDWARE_OP__"x"___"), noinline, optnone))
__CGRA_HARDWARE_OP("myExp") float myExp(float exp) {return *(float *) __hardware_imp();}

#define Convert(x) x
#define DATA_TYPE float
#define NTAPS 2048
#define LOOP_LENGTH  NTAPS
// #define STREAMING_ENBALED                               // Indicate that the streaming mode is enabled
#define STREAMING_WIDTH 100                            // The width of the streaming mode
#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((a)<(b))?(a):(b))
const DATA_TYPE c1 = 5, c2 = 6, c3 = 1;       // 0.5 1/6 1.0
const DATA_TYPE logc1 = 11, logc2 = 12, logc3 = 13;
const DATA_TYPE log2e = 10;                    // 1.4426950408889634    
const DATA_TYPE bias = 127;

const DATA_TYPE pi2 = 7;        // 2 * 3.14
const DATA_TYPE pi = 8;         // 3.14
const DATA_TYPE pi_2 = 9;       // 3.14 / 2
DATA_TYPE input[NTAPS], output[NTAPS], input_buf[STREAMING_WIDTH], output_buf[STREAMING_WIDTH];


DATA_TYPE sum_arr;
DATA_TYPE max_arr;
DATA_TYPE input[NTAPS], output[NTAPS];
DATA_TYPE summ[1];
DATA_TYPE maxx;
void kernel()
{
    #ifdef CGRA_COMPILER
    	loop_begin();
    #endif
    float temp = 0;
    for (int i = 0; i < NTAPS; i++) {
        DATA_TYPE e0 = myExp(input[i] - maxx);
        output[i] = Convert(e0);
        temp += e0;
    }
    summ[0] = temp;
    #ifdef CGRA_COMPILER
    	loop_end();
    #endif
}
