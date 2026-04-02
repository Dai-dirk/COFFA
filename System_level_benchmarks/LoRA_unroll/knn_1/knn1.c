#define __CGRA_HARDWARE_OP(x) __attribute__((annotate("___CGRA_HARDWARE_OP__"x"___"), noinline, optnone))
__CGRA_HARDWARE_OP("mySqrt") float mySqrt(float x) {return *(float *) __hardware_imp();}

// 宏定义
#define NUM_FEATURES 16
#define NUM_KNOWN_POINTS 128
// 全局矩阵声明
float xFeatures[NUM_FEATURES];
float knownFeatures[NUM_KNOWN_POINTS][NUM_FEATURES];
float knownClasses[NUM_KNOWN_POINTS];
float distance[NUM_KNOWN_POINTS];

void kernel()
{

    // --- Step 1: 矩阵乘法 temp_C = A * B + D ---
#ifdef CGRA_COMPILER
    loop_begin();
#endif
    for (int i = 0; i < NUM_KNOWN_POINTS; i++)
    {
        float temp = 0;

        // perform Euclidean distance
        #pragma unroll 4
        for (int j = 0; j < NUM_FEATURES; j++)
        {
            temp += (xFeatures[j] - knownFeatures[i][j]) * (xFeatures[j] - knownFeatures[i][j]);
        }
        distance[i] = mySqrt(temp);
        //printf("distance %e\n", distance);
}
#ifdef CGRA_COMPILER
    loop_end();
#endif
}
