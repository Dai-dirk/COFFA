#define B_inv 0.125f
#define B 32
#define DATA_TYPE float
float cos1[B][B];
float cos2[B][B];
inline DATA_TYPE my_cos(DATA_TYPE rem) {
    const DATA_TYPE pi = 3.14159265358979323846f;
    const DATA_TYPE pi2 = 6.28318530717958647692f;
    const DATA_TYPE pi_2 = 1.57079632679489661923f;
    const DATA_TYPE inv_pi2 = 0.15915494309189533577f;
    
    // 泰勒展开系数: cos(x) ≈ 1 - x²/2
    const DATA_TYPE c0 = 1.0f;
    const DATA_TYPE c2 = -1.0f / 2.0f;

    // cos(x) ≈ 1 - x²/2
    DATA_TYPE x2 = rem * rem;
    return (c0 + x2 * (c2 + (1.0f / 24.0f) * x2));
}
void kernel(){
    float factor1 = 1.57079632679489661923f * B_inv;  // π / (2 * B)
    float factor2 = 0.0f;
    const DATA_TYPE pi = 3.14159265358979323846f;
    const DATA_TYPE pi2 = 6.28318530717958647692f;
    const DATA_TYPE pi_2 = 1.57079632679489661923f;
    const DATA_TYPE inv_pi2 = 0.15915494309189533577f;
    float temp_cos;
    int m,n;
    #ifdef CGRA_COMPILER
    	loop_begin();
    #endif 
    for (m = 0; m < B; ++m) {
        // temp_cos = cos(factor2) * B_inv;
        #pragma unroll 2
        for (n = 1; n <= B; ++n) {
            // float theta = factor2 * (2*n + 1);
            float theta = n == 1 ? factor2 : factor2 * (2*n -1);
            // 规约到 [-π, π]：使用 fmod 逻辑
            int p = (int)(theta * inv_pi2);
            DATA_TYPE rem = theta - (DATA_TYPE)p * pi2;
            
            // 调整到 [-π, π] 范围
            rem = (rem > pi) ? rem - pi2 : ((rem < -pi) ? rem + pi2 : rem);
            // if (rem > pi) rem = rem - pi2;
            // else if (rem < -pi) rem = rem + pi2;
            
            // 利用对称性规约到 [-π/2, π/2]
            DATA_TYPE sign = 1.0f;
            DATA_TYPE use_positive = (rem > pi_2);
            DATA_TYPE use_negative = (rem < -pi_2);
            rem = use_positive ? (pi - rem) : (use_negative ? (-pi - rem) : rem);
            sign = (use_positive || use_negative) ? -1.0f : 1.0f;
            // if (rem > pi_2) {
            //     rem = pi - rem;
            //     sign = -1.0f;
            // } else if (rem < -pi_2) {
            //     rem = -pi - rem;
            //     sign = -1.0f;
            // }           
            temp_cos = sign * my_cos(rem) * B_inv;
            cos1[m][n-1] = temp_cos;
            cos2[n-1][m] = temp_cos;
        }
        factor2 += factor1;
    }
    #ifdef CGRA_COMPILER
    	loop_end();
    #endif
}
