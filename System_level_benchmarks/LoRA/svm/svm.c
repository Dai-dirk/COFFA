#define __CGRA_HARDWARE_OP(x) __attribute__((annotate("___CGRA_HARDWARE_OP__"x"___"), noinline, optnone))
__CGRA_HARDWARE_OP("myExp") float myExp(float exp) {return *(float *) __hardware_imp();}

#define N_SUP_VECT 64
#define N_FEATURES 64
#define gamma 0.05f
#define DATA_TYPE float
float sv_coeff[N_FEATURES];
float test_vector[N_SUP_VECT];
float sup_vectors[N_SUP_VECT][N_FEATURES];
int sum = 0;
int kernel()
{
    float diff;
    float norma;
    #ifdef CGRA_COMPILER
    	loop_begin();
    #endif 
    int tmp = 0;
    for (int i = 0; i < N_FEATURES; i++)
    {
        //#pragma unroll 4
        for (int j = 0; j < N_SUP_VECT; j++)
        {
            diff = test_vector[j] - sup_vectors[j][i];
            diff = diff * diff;
            norma = norma + diff;
        }
        tmp += (int)(myExp(-gamma * norma) * sv_coeff[i]);
        norma = 0;
    }
    sum = tmp;
    #ifdef CGRA_COMPILER
    	loop_end();
    #endif 
    return sum;
}
