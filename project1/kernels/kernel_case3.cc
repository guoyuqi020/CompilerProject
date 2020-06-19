#include "../run.h"

void kernel_case3(int (&B)[16][32], int (&C)[16][32], int (&A)[16][32]) {
  int temp1[16][32];
  for (int i = 0;i < 16;++i){
    for (int j = 0;j < 32;++j){
      temp1[i][j] = 0;
      if (0 <= i && i < 16) {
        if (0 <= j && j < 32) {
          temp1[i][j] += B[i][j];
        }
      }
      if (0 <= i && i < 16) {
        if (0 <= j && j < 32) {
          temp1[i][j] += C[i][j];
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
