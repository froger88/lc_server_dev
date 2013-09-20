#include "level.h"

//--------------------------------------------------------------------------------
//--------------------------STRUMIENIE-------------------------------------
//--------------------------------------------------------------------------------

inline ostream & operator <<(ostream& out, const Level &x)
{
  out << "<" << x.getMin() << " ; " << x.getMax() << ">" << endl;
  return out;
}

//--------------------------------------------------------------------------------
//--------------------------OPERATORY-----------------------------------
//--------------------------------------------------------------------------------

bool operator ==(Level &left, Level &right)
{
  if(left.getMin() == right.getMin() && left.getMax() == right.getMax())
  {
    return true;
  }
  return false;
}

bool operator !=(Level &left, Level &right)
{
  if(left == right)
  {
    return false;
  }
  return true;
}

inline bool operator ==(const Level &left, const int &right)
{
  if(left.getMin() <= right && left.getMax() >= right)
  {
    return true;
  }
  return false;
}

inline bool operator !=(const Level &left, const int &right)
{
  if(left == right)
  {
    return false;
  }
  return true;
}
