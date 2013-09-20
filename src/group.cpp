#include "group.h"

using namespace std;

//---------------------------------------------------------------------
//---------------KONSTRUKTORY i DESTRUKTORY----
//---------------------------------------------------------------------

Group::Group()
{
	ID = -1;
	MAX_MEMBERS = 50;
	MEMBERS = new long long int [MAX_MEMBERS];
	NUM_MEMBERS = 0;
	NAME = "NOT_NAMED";
	for(int i=0;i<MAX_MEMBERS;i++)
	{
		MEMBERS[i] = -1;
	}
}

//---------------------------------------------------------------------
//---------------FUNKCJE----------------------------------------
//---------------------------------------------------------------------

bool Group::addMember(long long int memberID)
{
	//sprawdza czy taki wpis juz istnieje, jezeli tak to zwraca true ale nie dodaje
	if(checkMember(memberID))
	{
		return true;
	}
	//jezeli nie istnieje, to znajduje wolne miejsce i dopisuje do bazy. jezeli brak wolnych miejsc to zwraca false
	if(NUM_MEMBERS < MAX_MEMBERS)
	{
		NUM_MEMBERS++;
		for(int i=0;i<MAX_MEMBERS;i++)
		{
			if(MEMBERS[i] != -1)
			{
				MEMBERS[i] = memberID;
				break;
			}
		}
	}
	else
	{
		return false;
	}
	return true;
}

bool Group::delMember(long long int memberID)//return false if member doesn't exists
{
	if(checkMember(memberID) == false || NUM_MEMBERS == 0)
	{
		return false;
	}
	else
	{
		for(int i=0;i<MAX_MEMBERS;i++)
		{
			if(MEMBERS[i] == memberID)
			{
				NUM_MEMBERS--;
				MEMBERS[i] = -1;
				return true;
			}
		}
	}
	return false;
}

bool Group::checkMember(long long int memberID)//return true if member exists
{
	for(int i=0;i<MAX_MEMBERS;i++)
	{
		if(MEMBERS[i] == memberID)
		{
			return true;
		}
	}
	return false;
}

long long int* Group::getMembers() const
{
	long long int *tabMembers = new long long int[NUM_MEMBERS];
	int count = 0;
	for(int i=0;i<MAX_MEMBERS;i++)
	{
		if(MEMBERS[i] != -1)
		{
			tabMembers[count] = MEMBERS[i];
			count++;
		}
	}
	return tabMembers;
}

Group& Group::operator = (const Group &x)
{
	NAME = x.NAME;
	NUM_MEMBERS = x.NUM_MEMBERS;
	MAX_MEMBERS = x.MAX_MEMBERS;
	delete [] MEMBERS;
	MEMBERS = NULL;
	MEMBERS = new long long int[MAX_MEMBERS];
	for(int i=0;i<MAX_MEMBERS;i++)
	{
		MEMBERS[i] = x.MEMBERS[i];
	}

	ID = -1;
	return *this;
}
