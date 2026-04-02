// 宏定义
#define MAT_DIM_I 32
#define MAT_DIM_K 32
#define MAT_DIM_J 32
#define DATA_TYPE float
const DATA_TYPE beta = 3, const1 = 1;      // 0.1 0.0 1.0
#define alpha 0.33333333333333f
// 全局矩阵声明
float A[MAT_DIM_I][MAT_DIM_K];
float B[MAT_DIM_K][MAT_DIM_J];
float D[MAT_DIM_I][MAT_DIM_J];
float C[MAT_DIM_I][MAT_DIM_J];
float igelu_C[MAT_DIM_I][MAT_DIM_J];

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
    DATA_TYPE log_mantissa = frac_x * (1.0f + frac_x * (-0.5f + frac_x * (1.0f/3.0f + frac_x * (-0.25f + frac_x *(0.5f)))));
    DATA_TYPE result = (DATA_TYPE)exp_part + log_mantissa * log2e;
    return result;
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
            #pragma unroll 2
            for (int k = 0; k < MAT_DIM_K; ++k)
            {
                sum += A[i][k] * B[k][j];
            }
            sum += D[i][j];
            DATA_TYPE tmp = my_exp(beta * sum) + (const1);
            igelu_C[i][j] = ((DATA_TYPE)(const1) / beta) * my_log(tmp);
        }
    }
#ifdef CGRA_COMPILER
    loop_end();
#endif
}
