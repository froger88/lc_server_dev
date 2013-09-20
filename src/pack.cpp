
#include "pack.h"

int pack(string &dataIn, unsigned char*dataOut, unsigned int opt1, unsigned char opt2, int Length)
{
  char *in = new char[dataIn.length()];
  for(int i = 0; i < dataIn.length(); i++)
  {
    in[i] = dataIn[i];
  }
  long L = Length - 11;
  unsigned char * dataOut2 = new unsigned char[Length - 11];
  int result = compress2((Bytef*) dataOut2, (uLongf*) & L, (const Bytef*) in, dataIn.length(),Z_BEST_SPEED);
  if(result == 0)
  {
    if(L > 0 && L < Length - 11)
    {
      //oznacz header
      dataOut[0] = 0x5b; //'['
      dataOut[5] = 0x5d; //']'
      //konwertuj L do headera
      dataOut[1] = ((L + 11) / 16777216) % 256;
      dataOut[2] = ((L + 11) / 65536) % 256;
      dataOut[3] = ((L + 11) / 256) % 256;
      dataOut[4] = (L + 11) % 256;
      //konwertuj opt1 do opcji1 headera
      dataOut[6] = (opt1 / 16777216) % 256;
      dataOut[7] = (opt1 / 65536) % 256;
      dataOut[8] = (opt1 / 256) % 256;
      dataOut[9] = opt1 % 256;
      //konwertuj opt2 do opcji2 headera
      dataOut[10] = opt2;
      //dolacz wiadomosc
      for(int i = 11; i < L; i++)
      {
        dataOut[i] = dataOut2[i - 11];
      }
      //posprzataj po sobie
      for(int i = 0; i < L; i++)
      {
        dataOut2[i] = NULL;
      }
      delete [] dataOut2;
      delete [] in;

      //oddaj wynik
      return (L + 11);
    }
  }
  return 0;
}