    #define _PB_LENGTH 31
volatile int A[_PB_LENGTH+1][_PB_LENGTH+1];
volatile int B[_PB_LENGTH+1][_PB_LENGTH+1];
volatile int C[_PB_LENGTH+1][_PB_LENGTH+1];
volatile int sum_c[_PB_LENGTH+1][_PB_LENGTH+1][_PB_LENGTH+1];
   //kernel 23
void kernel() { 
    #ifdef CGRA_COMPILER
    	loop_begin();
    #endif 
     int alpha = 41;
     for (int i = 0; i <= _PB_LENGTH ; i++)
        {
          for (int j = 0; j <= _PB_LENGTH ; j++)
          {
            //#pragma unroll 2
            int temp = 0;
            #pragma unroll 4
            for (int k = 0; k <= _PB_LENGTH; k++){
	      temp += alpha * A[i][k] * B[k][j];
	      //temp += alpha * B[i][k] * A[j][k];
	    }
	    C[i][j] = temp;
          }
        }
                       
    #ifdef CGRA_COMPILER
    	loop_end();
    #endif 
}