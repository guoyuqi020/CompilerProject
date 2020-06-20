#include "../run2.h"

void grad_case6(float (&C)[8][16][3][3], float (&dA)[2][8][5][5], float (&dB)[2][16][7][7]) {
  for (int index1 = 0;index1 < 2;++index1){
    for (int index2 = 0;index2 < 16;++index2){
      for (int index3 = 0;index3 < 7;++index3){
        for (int index4 = 0;index4 < 7;++index4){
          float temp1[2][8][5][5];
          dB[index1][index2][index3][index4] = 0.0;
          for (int k = 0;k < 8;++k){
            for (int n = 0;n < 2;++n){
              for (int p = 0;p < 5;++p){
                for (int q = 0;q < 5;++q){
                  temp1[n][k][p][q] = 0;
                  for (int c = 0;c < 16;++c){
                    for (int r = 0;r < 3;++r){
                      for (int s = 0;s < 3;++s){
                        if (0 <= n && n < 2) {
                          if (0 <= c && c < 16) {
                            if (0 <= p + r && p + r < 7) {
                              if (0 <= q + s && q + s < 7) {
                                if (0 <= k && k < 8) {
                                  if (0 <= c && c < 16) {
                                    if (0 <= r && r < 3) {
                                      if (0 <= s && s < 3) {
                                        temp1[n][k][p][q] += (( index1 == n && index2 == c && index3 == p + r && index4 == q + s ) ? ( 1 ) : ( 0 )) * C[k][c][r][s];
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
                }
              }
            }
          }
          for (int k = 0;k < 8;++k){
            for (int n = 0;n < 2;++n){
              for (int p = 0;p < 5;++p){
                for (int q = 0;q < 5;++q){
                  dB[index1][index2][index3][index4] += dA[n][k][p][q] * temp1[n][k][p][q];
                }
              }
            }
          }
        }
      }
    }
  }
}
