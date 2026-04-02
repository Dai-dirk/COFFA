#define N_SUP_VECT 64
#define N_FEATURES 64
#define gamma 0.05f
#define DATA_TYPE float
float sv_coeff[N_FEATURES];
float test_vector[N_SUP_VECT];
float sup_vectors[N_SUP_VECT][N_FEATURES];
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
int sum = 0;
void kernel()
{
    float diff;
    float norma;
    #ifdef CGRA_COMPILER
    	loop_begin();
    #endif 
    int tmp;
    for (int i = 0; i < N_FEATURES; i++)
    {
        for (int j = 0; j < N_SUP_VECT; j++)
        {
            diff = test_vector[j] - sup_vectors[j][i];
            diff = diff * diff;
            norma = norma + diff;
        }
        tmp += (int)(my_exp(-gamma * norma) * sv_coeff[i]);
        norma = 0;
    }
    sum = tmp;
    #ifdef CGRA_COMPILER
    	loop_end();
    #endif 
}