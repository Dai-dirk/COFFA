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
const DATA_TYPE beta1 = 3, beta2 = 5, const1 = 1, const2 = -3;     // 1.0 -2.0 beta1: 1/ beta beta2: beta

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
    DATA_TYPE exp_frac_x  = 1.0f + frac_x * (1.0f + frac_x * (0.5f + frac_x * (1.0f/6.0f + frac_x * (1.0f/24.0f))));
    return exp_x * exp_frac_x;
}
inline DATA_TYPE my_log(DATA_TYPE x) {
    const DATA_TYPE log2e = 1.4426950408889634f;  
    const int bias = -127;
    const DATA_TYPE logc1 = 1.0f;
    const DATA_TYPE logc2 = -1.0f / 2.0f;
    const DATA_TYPE logc3 = 1.0f / 3.0f;
    int x_bin = *(int*)&x;
    int exp_part = ((x_bin >> 23) & 0xff) - 127;
    int mantissa_bits = (x_bin & 0x007FFFFF) | 0x3F800000;
    DATA_TYPE mantissa = *(DATA_TYPE*)&mantissa_bits;
    DATA_TYPE frac_x = mantissa - 1.0f;
    DATA_TYPE log_mantissa = frac_x * (1.0f + frac_x * (-0.5f + frac_x * (1.0f/3.0f + frac_x * (-0.25f))));
    DATA_TYPE result = (DATA_TYPE)exp_part + log_mantissa * log2e;
    return result;
}
volatile DATA_TYPE input[LOOP_LENGTH];
DATA_TYPE output[LOOP_LENGTH];
void kernel()
/*   input :           input sample array */
/*   output:           output sample array */
{
    #ifdef CGRA_COMPILER
        loop_begin();
    #endif
    for (int i = 0; i < LOOP_LENGTH; i++) {
        DATA_TYPE x = Convert(input[i]);
        // const2 should be beta1 * -2
        DATA_TYPE tmp = my_exp(beta2 * x) + (DATA_TYPE)(const1);
        DATA_TYPE softplus_x = (DATA_TYPE)(const2) * my_log(tmp);
        DATA_TYPE exp_2x = my_exp(softplus_x);
        output[i] = input[i] * ((DATA_TYPE)(const1) - exp_2x) / ((DATA_TYPE)(const1) + exp_2x);
    }
    #ifdef CGRA_COMPILER
        loop_end();
    #endif
}