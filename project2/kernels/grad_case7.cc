#include "../run2.h"

void grad_case7(float (&dB)[16][32], float (&dA)[32][16]) {
  for (int index1 = 0;index1 < 32;++index1){
    for (int index2 = 0;index2 < 16;++index2){
      float temp1[16][32];
      dA[index1][index2] = 0.0;
      for (int i = 0;i < 16;++i){
        for (int j = 0;j < 32;++j){
          temp1[i][j] = 0;
          if (0 <= j && j < 32) {
            if (0 <= i && i < 16) {
              temp1[i][j] += (( index1 == j && index2 == i ) ? ( 1 ) : ( 0 ));
            }
          }
        }
      }
      for (int i = 0;i < 16;++i){
        for (int j = 0;j < 32;++j){
          dA[index1][index2] += dB[i][j] * temp1[i][j];
        }
      }
    }
  }
}
