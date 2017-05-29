#include <stdio.h>
#include <system.h>
#include "bmp_lib.h"

int main(){
  print_img("logo.bmp", 0, 0);
  print_img("subaru.bmp", 0, 100);
  return 0;
}
