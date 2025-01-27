#include "../run.h"

void kernel_case2(float (&A)[16][8]) {
  float temp1[16][8];
  for (int i = 0;i < 16;++i){
    for (int j = 0;j < 8;++j){
      temp1[i][j] = 0;
      if (0 <= i && i < 16) {
        if (0 <= j && j < 8) {
          temp1[i][j] += A[i][j];
        }
      }
      temp1[i][j] += 2;
    }
  }
  for (int i = 0;i < 16;++i){
    for (int j = 0;j < 8;++j){
      A[i][j] = temp1[i][j];
    }
  }
}
