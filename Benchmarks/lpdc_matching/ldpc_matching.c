#include <string.h>
#include <stdio.h>

#define E 1792
#define KL 350
#define KH 360
#define Ncb  660
int W[Ncb];
int W_i[Ncb];
int fint[E];

void matching(){
    int Kn = KH-KL;
    int Wn = Ncb-Kn;
    int maxCnt = (E+Wn-1)/Wn;
    int step = 2;
    loop_begin();
    for(int i = 0; i < Ncb; i++){
    	int sum = 0;
    	for(int j = 0; j < maxCnt; j++){
    		int idx = i+j*Wn;
    		if(i < KL){
    			sum += fint[idx];
    		}else if(i >= KH && idx-Kn < E){
    			sum += fint[idx-Kn];
   		    }else if(i >= KL && i < KH){
   		 	    sum = 0x81;
    		}
    	}
    	W_i[i] = sum >> step; // round(sum >> step)
    }
    loop_end();
}
