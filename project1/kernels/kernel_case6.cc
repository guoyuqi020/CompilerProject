#include "../run.h"

void kernel_case6(float (&B)[2][16][7][7], float (&C)[8][16][3][3], float (&A)[2][8][5][5]) {
  float temp1[2][8][5][5];
  for (int k = 0;k < 8;++k){
    for (int n = 0;n < 2;++n){
      for (int p = 0;p < 5;++p){
        for (int q = 0;q < 5;++q){
          temp1[n][k][p][q] = 0;
          if (0 <= n && n < 2) {
            if (0 <= k && k < 8) {
              if (0 <= p && p < 5) {
                if (0 <= q && q < 5) {
                  temp1[n][k][p][q] += A[n][k][p][q];
                }
              }
            }
          }
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
                                temp1[n][k][p][q] += B[n][c][p + r][q + s] * C[k][c][r][s];
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
          A[n][k][p][q] = temp1[n][k][p][q];
        }
      }
    }
  }
}
