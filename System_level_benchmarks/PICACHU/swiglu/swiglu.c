#define Convert(x) x
#define DATA_TYPE float
// #define STREAMING_ENBALED                               // Indicate that the streaming mode is enabled
#define STREAMING_WIDTH 100                            // The width of the streaming mode
#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((a)<(b))?(a):(b))
#define sigmoid(x) (1.0f/(1.0f+my_exp(-x)))
const DATA_TYPE c1 = 5, c2 = 6, c3 = 1;       // 0.5 1/6 1.0
const DATA_TYPE logc1 = 11, logc2 = 12, logc3 = 13;
const DATA_TYPE log2e = 10;                    // 1.4426950408889634    
const DATA_TYPE bias = 127;


#define MAT_DIM_I 32
#define MAT_DIM_K 32
#define MAT_DIM_J 32
const DATA_TYPE beta = 3, const1 = 1;      // 0.1 0.0 1.0
#define alpha 0.33333333333333f
// 全局矩阵声明
float A[MAT_DIM_I][MAT_DIM_K];
float B[MAT_DIM_K][MAT_DIM_J];
float D[MAT_DIM_I][MAT_DIM_J];
float C[MAT_DIM_I][MAT_DIM_J];
float E[MAT_DIM_I][MAT_DIM_J];
float igelu_C[MAT_DIM_I][MAT_DIM_J];


const DATA_TYPE pi2 = 7;        // 2 * 3.14
const DATA_TYPE pi = 8;         // 3.14
const DATA_TYPE pi_2 = 9;       // 3.14 / 2
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


void kernel()
{

    // --- Step 1: 矩阵乘法 temp_C = A * B + D ---
#ifdef CGRA_COMPILER
    loop_begin();
#endif
    for (int i = 0; i < MAT_DIM_I; ++i)
    {	
        for (int j = 0; j < MAT_DIM_J; ++j)
        {
            float sum = 0.0f;
            for (int k = 0; k < MAT_DIM_K; ++k)
            {
                sum += A[i][k] * B[k][j];
            }
            sum += D[i][j];
            DATA_TYPE ai = Convert(sum);
            DATA_TYPE bi = Convert(E[i][j]);
            DATA_TYPE silu_ai = ai * sigmoid(ai);
            igelu_C[i][j] = Convert(bi * sigmoid(silu_ai));
        }
    }
#ifdef CGRA_COMPILER
    loop_end();
#endif
}
