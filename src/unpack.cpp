#include "unpack.h"
using namespace std;

bool unpack(const unsigned char* buffIn, int buffLength, string &buffOut, int maxOutSize) //zwraca true jezeli OK, false jezeli critical error
{
  //paczka ma postac
  //[<4>]<4><1><wiadomosc_spakowana>
  if(buffLength > 11)
  {
    int buffLength2 = buffLength - 11;
    unsigned char * buffIn2 = new unsigned char [buffLength2];
    for(int i = 11; i < buffLength; i++)
    {
      buffIn2[i - 11] = buffIn[i];
    }
    //rozapkuj
    char *out = new char[maxOutSize];
    long outLength = maxOutSize;
    int result = uncompress((Bytef*) out, (uLongf*) & outLength, (const Bytef*) buffIn2, (uLong) buffLength2);

    //posprzataj po sobie
    for(int i=0;i<buffLength-11;i++)
    {
      buffIn2[i] = NULL;
    }
    delete [] buffIn2;

    //oddaj wynik
    if(result == 0)
    {
      buffOut.assign(out);
      return true;
    }
  }
  else
  {
    return false;
  }
}

