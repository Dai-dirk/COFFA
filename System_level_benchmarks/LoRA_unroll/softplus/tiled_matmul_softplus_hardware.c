#define __CGRA_HARDWARE_OP(x) __attribute__((annotate("___CGRA_HARDWARE_OP__"x"___"), noinline, optnone))
__CGRA_HARDWARE_OP("mySoftplus") float mySoftplus(float exp) {return *(float *) __hardware_imp();}

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
            //#pragma unroll 4
            for (int k = 0; k < MAT_DIM_K; ++k)
            {
                sum += A[i][k] * B[k][j];
            }
            sum += D[i][j];
            igelu_C[i][j] = ((DATA_TYPE)(const1) / beta) * mySoftplus(sum);
        }
    }
#ifdef CGRA_COMPILER
    loop_end();
#endif
}
