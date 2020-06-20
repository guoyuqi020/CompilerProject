#include "../run2.h"

void grad_case5(float (&C)[32][32], float (&D)[4][32], float (&dA)[16][32], float (&dB)[16][32][4]) {
  for (int index1 = 0;index1 < 16;++index1){
    for (int index2 = 0;index2 < 32;++index2){
      for (int index3 = 0;index3 < 4;++index3){
        float temp1[16][32];
        dB[index1][index2][index3] = 0.0;
        for (int i = 0;i < 16;++i){
          for (int j = 0;j < 32;++j){
            temp1[i][j] = 0;
            for (int k = 0;k < 32;++k){
              for (int l = 0;l < 4;++l){
                if (0 <= i && i < 16) {
                  if (0 <= k && k < 32) {
                    if (0 <= l && l < 4) {
                      if (0 <= k && k < 32) {
                        if (0 <= j && j < 32) {
                          if (0 <= l && l < 4) {
                            if (0 <= j && j < 32) {
                              temp1[i][j] += (( index1 == i && index2 == k && index3 == l ) ? ( 1 ) : ( 0 )) * C[k][j] * D[l][j];
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
        for (int i = 0;i < 16;++i){
          for (int j = 0;j < 32;++j){
            dB[index1][index2][index3] += dA[i][j] * temp1[i][j];
          }
        }
      }
    }
  }
}
