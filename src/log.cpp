#include "log.h"
using namespace std;

void log(int ERR_ID, string description)
{
	fstream __LOG;
	__LOG.open("qq.log", ios::out | ios::app);
	if(__LOG.good() == false)
	{
		cout << "[ERROR #4] - cannot open and create log file" << endl;
		__LOG.close();
		return;
	}
	if(ERR_ID >= 0)
	{
		cout << "[ERROR #" << ERR_ID << "] " << description.c_str() << endl;
		__LOG << "[ERROR #" << ERR_ID << "] " << description.c_str() <<endl;
		__LOG.close();
		return;
	}
	else
	{
		cout << "[OK #"<< -(ERR_ID) << "] " << description.c_str() <<endl;
		__LOG << "[OK #"<< -(ERR_ID) << "] " << description.c_str() <<endl;
		__LOG.close();
		return;
	}
}
