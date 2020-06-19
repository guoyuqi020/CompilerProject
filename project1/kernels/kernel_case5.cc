#include "../run.h"

void kernel_case5(float (&B)[16][32], float (&C)[32][32], float (&D)[16][32], float (&alpha), float (&beta), float (&A)[16][32]) {
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
        if (0 <= i && i < 16) {
          if (0 <= k && k < 32) {
            if (0 <= k && k < 32) {
              if (0 <= j && j < 32) {
                temp1[i][j] += alpha * (B[i][k] * C[k][j]);
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
  float temp2[16][32];
  for (int i = 0;i < 16;++i){
    for (int j = 0;j < 32;++j){
      temp2[i][j] = 0;
      if (0 <= i && i < 16) {
        if (0 <= j && j < 32) {
          temp2[i][j] += A[i][j];
        }
      }
      if (0 <= i && i < 16) {
        if (0 <= j && j < 32) {
          temp2[i][j] += beta * D[i][j];
        }
      }
    }
  }
  for (int i = 0;i < 16;++i){
    for (int j = 0;j < 32;++j){
      A[i][j] = temp2[i][j];
    }
  }
}
