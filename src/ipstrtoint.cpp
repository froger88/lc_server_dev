
#include "inttostr.h"

#include "ipstrtoint.h"

using namespace std;

void ipStrToInt(string IP, long long int &IPNum, string &IPNumStr)
{
  IPNum = -1;
  int error = 0;
  long long int tmp[4];
  int x = 0;

  if(IP.length() < 7)
  {
    IPNum = -1;
  }
  string num = "";

  for(int i = 0; i < IP.length(); i++)
  {
    if(IP[i] != '.')
    {
      num += IP[i];
    }
    else
    {
      if(x < 4)
      {
        tmp[x] = strToInt(num, error);
        num.clear();
        x++;
      }
    }
  }
  tmp[3] = strToInt(num,error);
  IPNum = tmp[0]*16777216+tmp[1]*65536+tmp[2]*256+tmp[3];
  if(IPNum <0)
  {
    IPNum = -1;
  }
  IPNumStr = "";
  IPNumStr = intToStr(IPNum);
  return;
}
