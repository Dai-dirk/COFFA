#define __CGRA_HARDWARE_OP(x) __attribute__((annotate("___CGRA_HARDWARE_OP__"x"___"), noinline, optnone))
__CGRA_HARDWARE_OP("mySqrt") float mySqrt(float x) {return *(float *) __hardware_imp();}

// 宏定义
#define NUM_FEATURES 16
#define NUM_KNOWN_POINTS 128
#define K 3
// 全局矩阵声明
volatile float BestPointsDistances[K];
volatile float BestPointsClasses[K];
volatile float knownFeatures[NUM_KNOWN_POINTS][NUM_FEATURES];
volatile float knownClasses[NUM_KNOWN_POINTS];
volatile float distance[NUM_KNOWN_POINTS];

void kernel()
{

    // --- Step 1: 矩阵乘法 temp_C = A * B + D ---
#ifdef CGRA_COMPILER
    loop_begin();
#endif
    for (int i = 0; i < NUM_KNOWN_POINTS; i++)
    {
        float max = 0;
        int index = 0;

        // perform Euclidean distance
        // #pragma unroll 2
        for (int j = 0; j < K; j++)
        {
            float dbest = BestPointsDistances[j];
            max = (dbest > max) ? dbest : max;
            index = (dbest > max) ? j : index;
        }
        float dbest = BestPointsDistances[index];
        float cbest = BestPointsClasses[index];
    
        BestPointsDistances[index] = (distance[i] < max) ? distance[i] : dbest;
        BestPointsClasses[index] = (distance[i] < max) ? knownClasses[i] : cbest;
        //printf("distance %e\n", distance);
}
#ifdef CGRA_COMPILER
    loop_end();
#endif
}
