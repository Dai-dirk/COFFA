#define size 1088
#define rows 256
#define columns 4
#define gaussian_row 16
#define gaussian_col 16
int uniformH[size];
int uniformV[size];
int m_GaussianKernel[gaussian_row][gaussian_col];
int m_DispH_i[size];
int m_DispV_i[size];
int m_DispH[size];
int m_DispV[size];

void GenerateDistortionMap( )
{
	int row, col;

	int fConvolvedH, fConvolvedV;
	int fSampleH, fSampleV;
	int elasticScale = 15;
	int xxx, yyy, xxxDisp, yyyDisp;
	int iiMid = 11/2;  // GAUSSIAN_FIELD_SIZE is strictly odd
	loop_begin();	//pragma for LLVM-based front-end tool
	for ( row=0; row<rows; ++row )
	{
		fConvolvedH = 0;
		fConvolvedV = 0;

		for ( xxx=0; xxx<11; ++xxx )
		{
			for ( yyy=0; yyy<11; ++yyy )
			{
				xxxDisp = 0 - iiMid + xxx;
				yyyDisp = row - iiMid + yyy;

				if ( xxxDisp<0 || xxxDisp>=columns || yyyDisp<0 || yyyDisp>=rows )
				{
					fSampleH = 0;
					fSampleV = 0;
				}
				else
				{
					int location = yyyDisp * columns + xxxDisp;
					fSampleH = uniformH[ location ];
					fSampleV = uniformV[ location ]; 
				}

				fConvolvedH += fSampleH * m_GaussianKernel[ yyy ][ xxx ];
				fConvolvedV += fSampleV * m_GaussianKernel[ yyy ][ xxx ];
			}
		}
		// int location = row * columns + col;
		int location = row * columns;
		m_DispH_i[ location ] = elasticScale * fConvolvedH;
		m_DispV_i[ location ] = elasticScale * fConvolvedV;
	}
	loop_end();
}
