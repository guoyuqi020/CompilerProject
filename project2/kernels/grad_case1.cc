#include "../run2.h"

void grad_case1(float (&B)[4][16], float (&dC)[4][16], float (&dA)[4][16]) {
  for (int index1 = 0;index1 < 4;++index1){
    for (int index2 = 0;index2 < 16;++index2){
      float temp1[4][16];
      dA[index1][index2] = 0.0;
      for (int i = 0;i < 4;++i){
        for (int j = 0;j < 16;++j){
          temp1[i][j] = 0;
          if (0 <= i && i < 4) {
            if (0 <= j && j < 16) {
              if (0 <= i && i < 4) {
                if (0 <= j && j < 16) {
                  temp1[i][j] += (( index1 == i && index2 == j ) ? ( 1 ) : ( 0 )) * B[i][j];
                }
              }
            }
          }
          temp1[i][j] += 0;
        }
      }
      for (int i = 0;i < 4;++i){
        for (int j = 0;j < 16;++j){
          dA[index1][index2] += dC[i][j] * temp1[i][j];
        }
      }
    }
  }
}
