#ifndef LEVEL_H
#define LEVEL_H

#include <iostream>
using namespace std;

class Level
{
public:
	//kontrukotry i destruktory
	inline Level(int min=1,int max=1);
	inline Level(const Level& x);
	inline ~Level();
	//funkcje
	inline void setMin(int min);
	inline void setMax(int max);

	inline int getMin() const;
	inline int getMax() const;

	inline bool check(int lvl) const;//returns false if lvl is out of min-max range
	inline bool check(Level &x);
	//strumienie
	inline friend ostream& operator << (ostream& out, const Level &x);
	//operatory
	inline Level& operator = (const Level &x);
	friend bool operator == (Level &left, Level &right);
	friend bool operator == (const Level &left, const int &right); //UWAGA*
	friend bool operator != (Level &left, Level &right);
	friend bool operator != (const Level &left, const int &right);//UWAGA*

	//* - zwraca odpowiednio true/false, jezeli zmienna typu int jest w/poza zakresem levelu
private:
	int MIN;
	int MAX;
protected:
};

//-----------------------------------------------------------
//---------------------------INLINE----------------------
//----------------------------------------------------------

#include "level.h"

//--------------------------------------------------------------------------------
//--------------------------KONSTRUKTORY I DEStRUKTORY--------
//--------------------------------------------------------------------------------

//default - min = 1, max = 1
inline Level::Level(int min, int max)
{
	MIN = min;
	MAX = max;
}

inline Level::Level(const Level& x)
{
	*this = x;
}

inline Level::~Level()
{

}

//--------------------------------------------------------------------------------
//--------------------------FUNKCJE-----------------------------------------
//--------------------------------------------------------------------------------

inline void Level::setMin(int min)
{
	MIN = min;
	return;
}

inline void Level::setMax(int max)
{
	MAX = max;
	return;
}

inline int Level::getMin() const
{
	return MIN;
}

inline int Level::getMax() const
{
	return MAX;
}

inline bool Level::check(int lvl) const
{
	if(lvl <= MAX && lvl >= MIN)
	{
		return true;
	}
	return false;
}

inline bool Level::check(Level &x)
{
	if(x.getMax() <= MAX && x.getMin() >= MIN)
	{
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------------
//--------------------------OPERATORY-----------------------------------
//--------------------------------------------------------------------------------

inline Level& Level::operator = (const Level &x)
{
	MIN = x.getMin();
	MAX = x.getMax();
	return *this;
}

#endif
