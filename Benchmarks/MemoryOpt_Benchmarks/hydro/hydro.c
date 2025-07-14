   #define size 1024
    int x[size];
    int y[size];
    int z[size];
   //kernel 23
void kernel() { 
    #ifdef CGRA_COMPILER
    	loop_begin();
    #endif 
    int q = 15;
    int r = 5;
    int t = 8;
    #pragma unroll 2
    for ( int k=0 ; k<1012 ; k++ ) {
            x[k] = q + y[k]*( r*z[k+10] + t*z[k+11] );
        }      
    #ifdef CGRA_COMPILER
    	loop_end();
    #endif 
}
