#include "../run2.h"

void grad_case8(float (&dB)[32], float (&dA)[2][16]) {
  for (int index1 = 0;index1 < 2;++index1){
    for (int index2 = 0;index2 < 16;++index2){
      float temp1[32];
      dA[index1][index2] = 0.0;
      for (int i = 0;i < 32;++i){
        temp1[i] = 0;
        if (0 <= i / 16 && i / 16 < 2) {
          if (0 <= i % 16 && i % 16 < 16) {
            temp1[i] += (( index1 == i / 16 && index2 == i % 16 ) ? ( 1 ) : ( 0 ));
          }
        }
      }
      for (int i = 0;i < 32;++i){
        dA[index1][index2] += dB[i] * temp1[i];
      }
    }
  }
}
