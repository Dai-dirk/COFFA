    #define _PB_NI 32
    #define _PB_NJ 32
    #define _PB_NK 32
    int C[_PB_NI][_PB_NI];
    int A[_PB_NI][_PB_NI];
    int B[_PB_NI][_PB_NI];
   //kernel 23
void kernel() { 
    #ifdef CGRA_COMPILER
    	loop_begin();
    #endif 
      int alpha = 5;
      for (int i = 0; i < _PB_NI; i++)
        for (int j = 0; j < _PB_NJ; j++)
	      {
	        for (int k = 0; k < _PB_NK; k= k + 4)
	          C[i][j] = alpha *( A[i][k] * B[k][j] +  A[i][k+1] * B[k+1][j]+  A[i][k+2] * B[k+2][j]+  A[i][k+3] * B[k+3][j]) ;
	      }
                       
    #ifdef CGRA_COMPILER
    	loop_end();
    #endif 
}
