#include "VFDManager.h"

unsigned int VFDManager::char_to_segments(char inputChar) {
  unsigned int outputSegCode = 0x0000;
  switch(inputChar) {
    case 0:
      outputSegCode = segment_A | segment_B | segment_C | segment_D | segment_E | segment_F;
    break;
    case 1:
      outputSegCode = segment_B | segment_C;
    break;
    case 2:
      outputSegCode = segment_A | segment_B | segment_G | segment_E | segment_D;
    break;
    case 3:
      outputSegCode = segment_A | segment_B | segment_C | segment_G | segment_D;
    break;
    case 4:
      outputSegCode = segment_B | segment_C | segment_F | segment_G;
    break;
    case 5:
      outputSegCode = segment_A | segment_C | segment_D | segment_F | segment_G;
    break;
    case 6:
      outputSegCode = segment_A | segment_C | segment_D | segment_E | segment_F | segment_G;
    break;
    case 7:
      outputSegCode = segment_A | segment_B | segment_C;
    break;
    case 8:
      outputSegCode = segment_A | segment_B | segment_C | segment_D | segment_E | segment_F | segment_G;
    break;
    case 9:
      outputSegCode = segment_A | segment_B | segment_C | segment_D | segment_F | segment_G;
    break;
    case '0':
      outputSegCode = segment_A | segment_B | segment_C | segment_D | segment_E | segment_F;
    break;
    case '1':
      outputSegCode = segment_B | segment_C;
    break;
    case '2':
      outputSegCode = segment_A | segment_B | segment_G | segment_E | segment_D;
    break;
    case '3':
      outputSegCode = segment_A | segment_B | segment_C | segment_G | segment_D;
    break;
    case '4':
      outputSegCode = segment_B | segment_C | segment_F | segment_G;
    break;
    case '5':
      outputSegCode = segment_A | segment_C | segment_D | segment_F | segment_G;
    break;
    case '6':
      outputSegCode = segment_A | segment_C | segment_D | segment_E | segment_F | segment_G;
    break;
    case '7':
      outputSegCode = segment_A | segment_B | segment_C;
    break;
    case '8':
      outputSegCode = segment_A | segment_B | segment_C | segment_D | segment_E | segment_F | segment_G;
    break;
    case '9':
      outputSegCode = segment_A | segment_B | segment_C | segment_D | segment_F | segment_G;
    break;
    case ' ':
      outputSegCode = 0;
    break;
    case 'c':
      outputSegCode = segment_G | segment_E | segment_D;
    break;
    case 'C':
      outputSegCode = segment_A | segment_E | segment_F | segment_D;
    break;
    case 'u':
      outputSegCode = segment_E | segment_D | segment_C;
    break;
    case 'w':
      outputSegCode = segment_D | segment_C;
    break;
    case 'U':
      outputSegCode = segment_E | segment_D | segment_C | segment_B | segment_F;
    break;
    case 'W':
      outputSegCode = segment_D | segment_C | segment_B;
    break;
    case 'h':
      outputSegCode = segment_F | segment_E | segment_G | segment_C;
    break;
    case 'H':
      outputSegCode = segment_F | segment_E | segment_G | segment_C | segment_B;
      break;
    case 'O':
      outputSegCode = segment_A | segment_B | segment_C | segment_D | segment_E | segment_F;
    break;
    case 'o':
      outputSegCode = segment_E | segment_G | segment_C | segment_D;
    break;
    case 'P':
      outputSegCode = segment_A | segment_B | segment_E | segment_F | segment_G;
    break;
    case 't':
      outputSegCode = segment_F | segment_E | segment_D | segment_G;
    break;
    case 'r':
      outputSegCode = segment_E | segment_G;
    break;
    case 'n':
      outputSegCode = segment_E | segment_G | segment_C;
    break;
    case 'N':
      outputSegCode = segment_E | segment_A | segment_C | segment_B | segment_F;
    break;
    case 'M':
      outputSegCode = segment_A | segment_C | segment_B;
    break;
    case 'm':
      outputSegCode = segment_G | segment_C;
    break;
    case 'f':
      outputSegCode = segment_A | segment_E | segment_F | segment_G;
    break;
    case 'F':
      outputSegCode = segment_A | segment_E | segment_F | segment_G;
    break;
    case 'i':
      outputSegCode = segment_E;
    break;
    case 'E':
      outputSegCode = segment_A | segment_D | segment_E | segment_F | segment_G;
    break;
    case 'e':
      outputSegCode = segment_A | segment_B | segment_D | segment_E | segment_F | segment_G;
    break;
    case 'S':
      outputSegCode = segment_A | segment_C | segment_D | segment_F | segment_G;
    break;
    case 's':
      outputSegCode = segment_A | segment_C | segment_D | segment_F | segment_G;
    break;
    case 'y':
      outputSegCode = segment_B | segment_C | segment_D | segment_F | segment_G;
    break;
    case 'Y':
      outputSegCode = segment_B | segment_C | segment_F | segment_G;
    break;
    case 'A':
      outputSegCode = segment_A | segment_B | segment_C | segment_E | segment_F | segment_G;
    break;
    case 'I':
      outputSegCode = segment_B | segment_C;
    break;
    case 'd':
      outputSegCode = segment_B | segment_C | segment_D | segment_E | segment_G;
    break;
    case 'B':
      outputSegCode = segment_A | segment_B | segment_C | segment_D | segment_E | segment_F | segment_G;
    break;
    case 'b':
      outputSegCode = segment_C | segment_D | segment_E | segment_F | segment_G;
    break;
    case 'L':
      outputSegCode = segment_D | segment_E | segment_F;
    break;
    case '*':
      outputSegCode = segment_A | segment_B | segment_F | segment_G;
    break;
    //segment_A | segment_B | segment_C | segment_D | segment_E | segment_F | segment_G;
  }
  return outputSegCode;
}
