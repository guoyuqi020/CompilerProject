#include "../run2.h"

void grad_case10(float (&dA)[8][8], float (&dB)[10][8]) {
  for (int index1 = 0;index1 < 10;++index1){
    for (int index2 = 0;index2 < 8;++index2){
      float temp1[8][8];
      dB[index1][index2] = 0.0;
      for (int i = 0;i < 8;++i){
        for (int j = 0;j < 8;++j){
          temp1[i][j] = 0;
          if (0 <= i && i < 10) {
            if (0 <= j && j < 8) {
              if (0 <= i + 1 && i + 1 < 10) {
                if (0 <= j && j < 8) {
                  if (0 <= i + 2 && i + 2 < 10) {
                    if (0 <= j && j < 8) {
                      temp1[i][j] += ((( index1 == i && index2 == j ) ? ( 1 ) : ( 0 )) + (( index1 == i + 1 && index2 == j ) ? ( 1 ) : ( 0 )) + (( index1 == i + 2 && index2 == j ) ? ( 1 ) : ( 0 ))) / 3.0;
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
          dB[index1][index2] += dA[i][j] * temp1[i][j];
        }
      }
    }
  }
}
