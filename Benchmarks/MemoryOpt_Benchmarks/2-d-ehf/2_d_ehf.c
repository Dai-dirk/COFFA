      
   int zp[10][100];
   int za[10][100];
   int zq[10][100];
   int zr[10][100];
   int zm[10][100];
   int zb[10][100];
   //kernel 18
   void kernel() { 
       int kn = 9;
       int jn = 100;
    #ifdef CGRA_COMPILER
    	loop_begin();
    #endif 
       for ( int k=1 ; k<kn ; k++ ) {
/* #pragma nohazard */
          for ( int j=1 ; j<jn ; j++ ) {
              za[k][j] = ( zp[k+1][j-1] +zq[k+1][j-1] -zp[k][j-1] -zq[k][j-1] )*
                         ( zr[k][j] +zr[k][j-1] ) / ( zm[k][j-1] +zm[k+1][j-1]);
              zb[k][j] = ( zp[k][j-1] +zq[k][j-1] -zp[k][j] -zq[k][j] ) *
                         ( zr[k][j] +zr[k-1][j] ) / ( zm[k][j] +zm[k][j-1]);
          }
        }
    #ifdef CGRA_COMPILER
    	loop_end();
    #endif 
   }
