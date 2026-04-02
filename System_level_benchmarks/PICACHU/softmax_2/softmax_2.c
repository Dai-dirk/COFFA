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

inline DATA_TYPE my_exp(DATA_TYPE x) {
    const DATA_TYPE LN2     = 0.6931471805599453f;
    const DATA_TYPE INV_LN2 = 1.4426950408889634f;
    const DATA_TYPE c3 = 1.0f;
    const DATA_TYPE c2 = 1.0f / 6.0f;
    const DATA_TYPE c1 = 1.0f / 2.0f;
    int integer = (int)(x * INV_LN2);
    DATA_TYPE frac_x = x - (DATA_TYPE)integer * LN2;
    int bits = (127 + integer) << 23;
    DATA_TYPE exp_x = *(DATA_TYPE*)&bits;
    DATA_TYPE frac_x_square = frac_x * frac_x;
    DATA_TYPE tmp_poly = c2 * frac_x + c1;
    DATA_TYPE exp_frac_x = 1.0f + frac_x * (1.0f + frac_x * (0.5f + frac_x * (1.0f/6.0f + frac_x * (1.0f/24.0f))));
    return exp_x * exp_frac_x;
}
DATA_TYPE summ[1];
DATA_TYPE maxx[1];
void kernel(DATA_TYPE input[], DATA_TYPE output[])
{
    #ifdef CGRA_COMPILER
    	loop_begin();
    #endif
    float temp = 0;
    for (int i = 0; i < NTAPS; i++) {
        DATA_TYPE x0 = Convert(input[i]);
        DATA_TYPE e0 = my_exp(x0 - maxx[0]);
        output[i] = Convert(e0);
        temp += e0;
    }
    summ[0] = temp;
    #ifdef CGRA_COMPILER
    	loop_end();
    #endif
}
