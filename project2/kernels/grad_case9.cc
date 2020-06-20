#include "../run2.h"

void grad_case9(float (&dB)[4][6], float (&dA)[4]) {
  for (int index1 = 0;index1 < 4;++index1){
    float temp1[4][6];
    dA[index1] = 0.0;
    for (int i = 0;i < 4;++i){
      for (int j = 0;j < 6;++j){
        temp1[i][j] = 0;
        if (0 <= i && i < 4) {
          temp1[i][j] += (( index1 == i ) ? ( 1 ) : ( 0 ));
        }
      }
    }
    for (int i = 0;i < 4;++i){
      for (int j = 0;j < 6;++j){
        dA[index1] += dB[i][j] * temp1[i][j];
      }
    }
  }
}
