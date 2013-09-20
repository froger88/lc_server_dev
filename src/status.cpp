#include "status.h"

using namespace std;

Status::Status()
{
	reset();
}

Status::~Status()
{

}

void Status::setType(string value)
{
	TYPE = value;
}

void Status::setStatus(string type, string desc)
{
	TYPE = type;
	DESC = desc;
}

void Status::setDesc(string desc)
{
	DESC = desc;
}

void Status::reset()
{
	TYPE="0000";
	DESC="";
}

string Status::getType()
{
	return TYPE;
}

string Status::getDesc()
{
	return DESC;
}
