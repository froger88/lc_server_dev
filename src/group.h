#ifndef GROUP_H
#define GROUP_H

#include <iostream>
#include <string>

using namespace std;

/**
  UWAGA: wartosci domyslne:
 * id = -1  oznacza ze id nie zostalo przypisane
 * MEMBERS[_Z_] = -1 oznacza wolne miejsce w tablicy uzytkownikow (jest to tez wartosc domyslna)
 * MAX_MEMBERS jest zmienna przechowujaca maksymalna ilosc osob przypisanych do grupy. domyslnie 50
 **/

class Group
{
public:
	//konstruktory i destruktory
	Group();
	inline Group(const Group& x);
	inline ~Group();
	//funkcje
	inline void setID(int id);
	inline void setName(string name);
	inline void setMaxMembers(long long int max);
	//--------------------------------------------------------------------------
	bool addMember(long long int memberID);
	bool delMember(long long int memberID);//return false if member doesn't exists
	bool checkMember(long long int memberID);//return true if member exists
	inline string getName() const;
	long long int* getMembers() const;
	inline long long int getNumMembers() const;
	inline long long int getMaxMembers() const;
	inline int getID() const;
	//operatory
	Group& operator = (const Group &x);
	inline Group& operator += (long long int x);
	inline Group& operator -= (long long int x);

private:
	string NAME;//nazwa grupy
	long long int* MEMBERS; //id'ki osob przypissanych do grupy
	long long int NUM_MEMBERS;
	long long int MAX_MEMBERS;
	int ID;
protected:
};

//---------------------------------------------------------------------
//---------------KONSTRUKTORY I DESTRUKTORY---
//---------------------------------------------------------------------

inline Group::Group(const Group& x)
{
	*this = x;
	ID = -1;
}

inline Group::~Group()
{
	delete [] MEMBERS;
	MEMBERS = NULL;
}

//---------------------------------------------------------------------
//---------------FUNKCJE----------------------------------------
//---------------------------------------------------------------------

inline void Group::setID(int id)
{
	ID = id;
	return;
}

inline int Group::getID() const
{
	return ID;
}

inline void Group::setName(string name)
{
	NAME = name;
	return;
}

inline void Group::setMaxMembers(long long int max)
{
	MAX_MEMBERS = max;
	return;
}

inline string Group::getName() const
{
	return NAME;
}

inline long long int Group::getNumMembers() const
{
	return NUM_MEMBERS;
}

inline long long int Group::getMaxMembers() const
{
	return MAX_MEMBERS;
}

//---------------------------------------------------------------------
//---------------OPERATORY-----------------------------------
//---------------------------------------------------------------------

inline Group& Group::operator += (long long int x)
{
	addMember(x);
	return *this;
}

inline Group& Group::operator -= (long long int x)
{
	delMember(x);
	return *this;
}

#endif
