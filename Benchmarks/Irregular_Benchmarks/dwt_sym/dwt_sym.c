/*
Copyright (c) 2018, Rafat Hussain
*/
#include "wtmath.h"
#define len_cA 64
#define lpd_len 64
#define istride 1
#define ostride 2
int lpd[512];
int hpd[512];
int cA[1024];
int cD[1024];
int inp[1024];


void dwt_sym_stride() {
	int i, l, t, len_avg;
	int is, os;
	len_avg = lpd_len;

	for (i = 0; i < len_cA; ++i) {
		t = 2 * i + 1;
		os = i *ostride;
		// cA[os] = 0.0;
		// cD[os] = 0.0;
		int i_cA = 0;
		int i_cD = 0;
		for (l = 0; l < len_avg; ++l) {
			int lpd_i = lpd[l];
			int hpd_i = hpd[l];
			if ((t - l) >= 0 && (t - l) < len_cA) {
				is = (t - l) * istride;
				i_cA += lpd_i * inp[is];
				i_cD += hpd_i * inp[is];
			}
			else if ((t - l) < 0) {
				is = (-t + l - 1) * istride;
				i_cA += lpd_i * inp[is];
				i_cD += hpd_i * inp[is];
			}
			else if ((t - l) >= len_cA) {
				is = (2 * len_cA - t + l - 1) * istride;
				i_cA += lpd_i * inp[is];
				i_cD += hpd_i * inp[is];
			}

		}
		cA[os] = i_cA;
		cD[os] = i_cD;
	}


}




