#define num_rows 32
#define num_vectors 32
#define idxstride_y 2
#define vecstride_y 16
#define idxstride_x 8
#define vecstride_x 16
volatile int A_j [64];
volatile int A_i [48];
volatile int B_i [48];
volatile int y_data [640];
volatile int A_data [128];
volatile int x_data [768];                                                                                                                                                                       
// ns = 16; ne =32; A_j[i] = i;  
void csr_matvec_custom(){
   loop_begin();
   for (int i = 0; i < num_rows; i++)
            {
               for ( int jv=0; jv<num_vectors; ++jv )
               {
                  //for (int jj = A_i[i]; jj < A_i[i+1]; jj++)
                  for (int jj = A_i[i]; jj < B_i[i+1]; jj++)
                  {
                     int j = A_j[jj];
                     if (j > 0 && j < 64)
                        y_data[ j*idxstride_y + jv*vecstride_y ] +=
                           A_data[jj] * x_data[ i*idxstride_x + jv*vecstride_x];
                  }
               }
            }
   loop_end();
}
