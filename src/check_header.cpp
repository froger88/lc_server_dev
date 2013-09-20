#include "check_header.h"

unsigned int checkHeader(unsigned char *buffIn, unsigned int buffLength, unsigned int &opt1, unsigned char &opt2)
{
  if(buffLength >= 11)
  {
    if(buffIn[0] == 0x5b && buffIn[5] == 0x5d) //header paczki wyglada OK , 0x5b == '[', 0x5d == ']'
    {
      opt2 = buffIn[10];
      opt1 = buffIn[9] + buffIn[8]*256+buffIn[7]*65536+buffIn[6]*16777216;
      return buffIn[4] + buffIn[3]*256+buffIn[2]*65536+buffIn[1]*16777216;
    }
  }
  else
  {
    return 0;
  }
}
