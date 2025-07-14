      
   #define NUM 32
  int A[NUM][NUM];
  int B[NUM][NUM];
  int X[NUM][NUM];
   //kernel 18
   void adi() { 
    #ifdef CGRA_COMPILER
    	loop_begin();
    #endif 
       for (int i1=0 ; i1 < NUM ; i1++) {
          #pragma unroll 2
          for (int i2=0 ; i2 < NUM-2 ; i2++){
	    X[i1][i2] = X[i1][i2] - X[i1][i2+1] * A[i1][i2] * B[i1][i2+1];
	    B[i1][i2] = B[i1][i2] - A[i1][i2] * A[i1][i2] * B[i1][i2+1];
 	} 
 	}
    #ifdef CGRA_COMPILER
    	loop_end();
    #endif 
   }
