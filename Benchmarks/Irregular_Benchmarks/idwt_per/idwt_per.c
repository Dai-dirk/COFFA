#define len_cA 128
#define lpr_len 128
#define istride 1
#define ostride 2
int lpr[512];
int hpr[512];
int cA[768];
int cD[768];
int X[2048];
int X_i[2048];

void idwt_per_stride() {
	int len_avg, i, l, m, n, t, l2;
	int is, ms, ns;

	len_avg = lpr_len;
	l2 = len_avg / 2;
	m = -2;
	n = -1;
	loop_begin();
	for (i = 0; i < len_cA + l2 - 1; ++i) {
		m += 2;
		n += 2;
		ms = m * ostride;
		ns = n * ostride;
		// X[ms] = 0;
		// X[ns] = 0;
		int i_ms = 0;
		int i_ns = 0;
		for (l = 0; l < l2; ++l) {
			t = 2 * l;
			int lpr1 = lpr[t];
			int lpr2 = lpr[t + 1];
			int hpr1 = hpr[t];
			int hpr2 = hpr[t + 1];
			if ((i - l) >= 0 && (i - l) < len_cA) {
				is = (i - l) * istride;
				i_ms += lpr1 * cA[is] + hpr1 * cD[is];
				i_ns += lpr2 * cA[is] + hpr2 * cD[is];
			}
			else if ((i - l) >= len_cA && (i - l) < len_cA + len_avg - 1) {
				is = (i - l - len_cA) * istride;
				i_ms += lpr1 * cA[is] + hpr1 * cD[is];
				i_ns += lpr2 * cA[is] + hpr2 * cD[is];
			}
			else if ((i - l) < 0 && (i - l) > -l2) {
				is = (len_cA + i - l) * istride;
				i_ms += lpr1 * cA[is] + hpr1 * cD[is];
				i_ns += lpr2 * cA[is] + hpr2 * cD[is];
			}
		}
		X_i[ms] = i_ms;
		X_i[ns] = i_ns;
	}
	loop_end();
}
