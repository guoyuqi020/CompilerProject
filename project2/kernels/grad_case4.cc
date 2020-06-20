#include "../run2.h"

void grad_case4(float (&B)[16][32], float (&C)[32][32], float (&dA)[16][32], float (&dB)[16][32], float (&dC)[32][32]) {
  for (int index1 = 0;index1 < 16;++index1){
    for (int index2 = 0;index2 < 32;++index2){
      float temp1[16][32];
      dB[index1][index2] = 0.0;
      for (int i = 0;i < 16;++i){
        for (int j = 0;j < 32;++j){
          temp1[i][j] = 0;
          for (int k = 0;k < 32;++k){
            if (0 <= i && i < 16) {
              if (0 <= k && k < 32) {
                if (0 <= k && k < 32) {
                  if (0 <= j && j < 32) {
                    temp1[i][j] += (( index1 == i && index2 == k ) ? ( 1 ) : ( 0 )) * C[k][j];
                  }
                }
              }
            }
          }
        }
      }
      for (int i = 0;i < 16;++i){
        for (int j = 0;j < 32;++j){
          dB[index1][index2] += dA[i][j] * temp1[i][j];
        }
      }
    }
  }
  for (int index1 = 0;index1 < 32;++index1){
    for (int index2 = 0;index2 < 32;++index2){
      float temp2[16][32];
      dC[index1][index2] = 0.0;
      for (int i = 0;i < 16;++i){
        for (int j = 0;j < 32;++j){
          temp2[i][j] = 0;
          for (int k = 0;k < 32;++k){
            if (0 <= i && i < 16) {
              if (0 <= k && k < 32) {
                if (0 <= k && k < 32) {
                  if (0 <= j && j < 32) {
                    temp2[i][j] += B[i][k] * (( index1 == k && index2 == j ) ? ( 1 ) : ( 0 ));
                  }
                }
              }
            }
          }
        }
      }
      for (int i = 0;i < 16;++i){
        for (int j = 0;j < 32;++j){
          dC[index1][index2] += dA[i][j] * temp2[i][j];
        }
      }
    }
  }
}
