#include "acc8.h"

void acc8(double * in_array, double * out_array, int size){

  int i, j;
  for (i=0; i<size/20; i++){
    for (j=0; j<20; j++){
      out_array[i] += in_array[20*i+j];
    }
  }
}
