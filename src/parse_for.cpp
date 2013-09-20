#include "parse_for.h"

extern pthread_mutex_t mutex;

using namespace std;

string parseFor(const string from,const char* variable,int &err)
{
	err = 0;// 0 when ok
	char Reserved[4] = {'$','+','*','#'};
	string Text = from;
	string Var = variable;
	string Value = "E";
	//sprawdzenie dlugosci tekstu
	int length = Text.length();
	if(length <4)
	{
		err = 1;
		Value = "E";
		return Value.c_str();
	}

	//sprawdzenie pierwszego znaku
	int start = 0;
	for(int i=0;i<4;i++)
	{
		if(Text[0] == Reserved[i])
		{
			start = 1;
		}
    if(Text[0]=='#')
    {
    }
	}
	string tmp;
	tmp="";
  bool found = false;
	for(int i=start; i< length-1;i++)
	{
		if(Text[i] != '=' && Text[i] != '&')
		{
			tmp+= Text[i];
		}
		else
		{
			if(tmp == Var)
			{
        found = true;
				//odczytaj wartosc
				tmp = "";
				Value = "";
				for(int j = i;j<length-1;j++)
				{
					i++;
					if(Text[j] != '&')
					{
						if(Text[j] != '=')
						{
							Value += Text[j];
						}
					}
					else
					{
						return Value.c_str();
					}
				}
			}
			tmp ="";
		}
	}
  if(!found)
  {
    err = 1;
  }
	return Value.c_str();
}
