      
   int A[32][32];
   int B[32][32];
   int array1[1024];
   int path[32][32];
   //kernel 18
   void jacobi() { 
    #ifdef CGRA_COMPILER
    	loop_begin();
    #endif 
      
    //for (int k = 0; k < 32; k++){
     for(int i = 1; i < 31; i++){
      #pragma unroll 1
      for(int j =1; j < 31; j=j+2){
	 B[i][j] = 5 * (A[i][j] + A[i][j-1] + A[i][1+j] + A[1+i][j] + A[i-1][j]);
	 B[i][j+1] = 5 * (A[i][j+1] + A[i][j] + A[i][2+j] + A[1+i][j+1] + A[i-1][j+1]);
        }
     }
    //}
    #ifdef CGRA_COMPILER
    	loop_end();
    #endif 
   }
