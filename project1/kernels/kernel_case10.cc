#include "../run.h"

void kernel_case10(float (&B)[10][10], float (&A)[8][8]) {
  float temp1[8][8];
  for (int i = 0;i < 8;++i){
    for (int j = 0;j < 8;++j){
      temp1[i][j] = 0;
      if (0 <= i && i < 10) {
        if (0 <= j && j < 10) {
          if (0 <= i + 1 && i + 1 < 10) {
            if (0 <= j && j < 10) {
              if (0 <= i + 2 && i + 2 < 10) {
                if (0 <= j && j < 10) {
                  temp1[i][j] += (B[i][j] + B[i + 1][j] + B[i + 2][j]) / 3;
                }
              }
            }
          }
        }
      }
    }
  }
  for (int i = 0;i < 8;++i){
    for (int j = 0;j < 8;++j){
      A[i][j] = temp1[i][j];
    }
  }
}
