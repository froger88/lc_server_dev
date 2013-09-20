#ifndef STATUS_H
#define STATUS_H

#include <iostream>
using namespace std;


//XYY
//X - status
//YY - 2 cyfry odpowiadajace za id avatarka w airze
class Status
{
public:
	Status();
	~Status();

	void setType(string value);
	void setStatus(string type, string desc);
	void setDesc(string desc);

	void reset();

	string getType();
	string getDesc();
private:
	string TYPE;
	string DESC;
protected:
};

#endif
