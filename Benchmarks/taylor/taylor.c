#include <stdio.h>

#define BOUND 10000
int data[2048];
int data0[2048];
int dfx[2048];
int dfy[2048];
int z[2048];
// granularity: 0 represents 1 word; 1 represents 2 words
void kernel()
{
    	int i, j;
    	int BlockSize =32;
    	int x_length =32;
    	loop_begin();
	for (i = 0; i < BlockSize; i++)
	{
		for (j = 0; j < BlockSize; j++)
		{
			int temp_dfx=0;
			int temp_dfy=0;
			int temp_ixj = data[i*(x_length) + j];
			int temp_ixj1 = data[i*(x_length) + j + 1];
			int temp_ixjs1 = data[i*(x_length) + j - 1];
			int temp_i1xj = data[(i + 1)*(x_length) + j];
			int temp_is1xj = data[(i - 1)*(x_length) + j];
			if (j == 0)
				temp_dfx = (temp_ixj1 - temp_ixj);
			else if (j == (BlockSize - 1))
				temp_dfx = (temp_ixj - temp_ixjs1);
			else
				temp_dfx = (temp_ixj1 - temp_ixjs1)*2;

			if (i == 0)
				temp_dfy = (temp_i1xj - temp_ixj);
			else if (i == (BlockSize - 1))
				temp_dfy = (temp_ixj - data[(i - 1)*(x_length) + j]);
			else
				temp_dfy = (temp_i1xj - data[(i - 1)*(x_length) + j]) * 2;
			dfx[i*BlockSize + j]= temp_dfx;
			dfy[i*BlockSize + j] = temp_dfy;
			z[i*BlockSize + j] = data0[i*(x_length) + j] - temp_ixj;
			//z[i] = j;
		}
	}
    	loop_end();
}
