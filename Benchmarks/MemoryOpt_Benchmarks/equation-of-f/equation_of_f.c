    int x[1001];
    int u[1001];
    int z[1001];
    int y[1001];
    void kernel(){ 
       int t = 1;
       int r = 3;
       int q = 5;
       int n = 995;
    #ifdef CGRA_COMPILER
    	loop_begin();
    #endif 
       for ( int k=0 ; k<n ; k++ ) {
            x[k] = u[k] + r*( z[k] + r*y[k] ) +
                   t*( u[k+3] + r*( u[k+2] + r*u[k+1] ) +
                      t*( u[k+6] + q*( u[k+5] + q*u[k+4] ) ) );
       }
    #ifdef CGRA_COMPILER
    	loop_end();
    #endif 
  }