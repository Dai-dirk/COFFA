#define len_cA 128
#define lpr_len 128
#define istride 1
#define ostride 2
int lpr[512];
int hpr[512];
int cA[512];
int cD[512];
int X[2048];
int X_i[2048];

void idwt_sym_stride() {
	int len_avg, i, l, m, n, t, v;
	int ms, ns, is;
	len_avg = lpr_len;
	m = -2;
	n = -1;

	// loop_begin();
	for (v = 0; v < len_cA; ++v) {
		i = v;
		m += 2;
		n += 2;
		ms = m * ostride;
		ns = n * ostride;
		//X[ms] = 0;
		//X[ns] = 0;
		for (l = 0; l < len_avg / 2; ++l) {
			t = 2 * l;
			if ((i - l) >= 0 && (i - l) < len_cA) {
				is = (i - l) * istride;
				X_i[ms] += lpr[t] * cA[is] + hpr[t] * cD[is];
				X_i[ns] += lpr[t + 1] * cA[is] + hpr[t + 1] * cD[is];
			}
		}
	}
	// loop_end();
}
