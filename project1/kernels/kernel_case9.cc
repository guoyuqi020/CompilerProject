#include "../run.h"

void kernel_case9(float (&B)[16][32][8], float (&C)[32][32], float (&D)[8][32], float (&A)[16][32]) {
  float temp1[16][32];
  for (int i = 0;i < 16;++i){
    for (int j = 0;j < 32;++j){
      temp1[i][j] = 0;
      if (0 <= i && i < 16) {
        if (0 <= j && j < 32) {
          temp1[i][j] += A[i][j];
        }
      }
      for (int k = 0;k < 32;++k){
        for (int l = 0;l < 8;++l){
          if (0 <= i && i < 16) {
            if (0 <= k && k < 32) {
              if (0 <= l && l < 8) {
                if (0 <= k && k < 32) {
                  if (0 <= j && j < 32) {
                    if (0 <= l && l < 8) {
                      if (0 <= j && j < 32) {
                        temp1[i][j] += B[i][k][l] * C[k][j] * D[l][j];
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
      A[i][j] = temp1[i][j];
    }
  }
}