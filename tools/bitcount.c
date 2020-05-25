#include "stdio.h"
#include <unistd.h>

int main(int argc, char* argv[]) {
  unsigned int count = 0;
  char ch;
  while(read(0, &ch, 1) > 0) {
    switch (ch) {
      case '1':
      case '2':
      case '4':
      case '8':
        count += 1;
      break;
      case '3':
      case '5':
      case '6':
      case '9':
      case 'A':
      case 'C':
        count += 2;
      break;
      case '7':
      case 'B':
      case 'D':
      case 'E':
        count += 3;
        break;
      case 'F':
        count += 4;
      break;
      default:
      break;
    }
    
  }
  printf("Bit count : %d ", count);
  return 0;
}
