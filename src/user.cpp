
#include <queue>

#include "user.h"
//konstruktory

User::User()
{
  NICK = "NO_NAME";
  LVL.setMin(1);
  LVL.setMax(1); //default min=1,max=1
  ID = -1;
  FD = -1;
  KICK = false;
  LOGIN_DATA.clear();
}

User::User(const User &x)
{
  *this = x;
  ID = -1;
  FD = -1;
  KICK = x.KICK;
}

User::~User()
{
  delete [] InRoom;
  InRoom = NULL;
}
//funkcje

void User::setID(long long int new_id)
{
  ID = new_id;
  return;
}

void User::setFD(int new_sockfd)
{
  FD = new_sockfd;
  return;
}

void User::setNick(string nick)
{
  NICK = nick;
  return;
}

void User::setLevel(int min, int max)
{
  LVL.setMin(min);
  LVL.setMax(max);
  return;
}

Level& User::getLevel()
{
  Level& ret = LVL;
  return ret;
}

void User::getLevel(Level &lvl)
{
  lvl = LVL;
  return;
}

int User::getMaxLevel() const
{
  return LVL.getMax();
}

void User::getMaxLevel(int &lvl)
{
  lvl = LVL.getMax();
  return;
}

int User::getMinLevel() const
{
  return LVL.getMin();
}

void User::getMinLevel(int &lvl)
{
  lvl = LVL.getMin();
  return;
}

long long int User::getID() const
{
  return ID;
}

void User::getID(long long int &id)
{
  id = ID;
  return;
}

int User::getFD() const
{
  return FD;
}

void User::getFD(long long int &fd)
{
  fd = FD;
  return;
}

string User::getNick() const
{
  return NICK;
}

void User::getNick(string &nick)
{
  nick = NICK;
  return;
}
//on group operations

void User::addToGroup(Group& g)
{
  g += ID;
  return;
}

void User::delFromGroup(Group& g)
{
  g -= ID;
  return;
}

bool User::checkInGroup(Group& g) const
{
  if(g.checkMember(ID) == true)
  {
    return true;
  }
  return false;
}

void User::checkInGroup(Group& g, bool &inside)
{
  inside = g.checkMember(ID);
  return;
}

User& User::operator =(const User &x)
{
  ID = -1;
  FD = -1;
  KICK = x.KICK;
  LVL = x.LVL;
  NICK = x.NICK;
  SendQueue = x.SendQueue;
  return *this;
}

void User::kick()
{
  KICK = true;
  return;
}

bool User::getKickState() const
{
  return KICK;
}

void User::getKickState(bool& kstate)
{
  kstate = KICK;
  return;
}

//UWAGA operatory == oraz != nie sprawdzaja zgodnosci ID,FD oraz NICK - poniewaz zakladamy ze kazdy user ma inne

bool operator ==(User &left, User &right)
{
  if(left.LVL != right.LVL)
  {
    return false;
  }
  return true;
}

bool operator !=(User &left, User &right)
{
  if(left == right)
  {
    return false;
  }
  return true;
}

void User::free()
{
  ID = -1;
  FD = -1;

  LVL.setMin(1);
  LVL.setMax(1);

  NICK = "NO_NAME";
  status.reset();

  KICK = false;
  LOGIN_DATA.clear();
  while(!SendQueue.empty())
  {
    SendQueue.pop();
  }
  return;
}

void User::setLoginData(string LD)
{
  LOGIN_DATA = LD;
  return;
}

string User::getLoginData()
{
  return LOGIN_DATA;
}

void User::setLoginDataForClient(string LD)
{
  LOGIN_DATA_FOR_CLIENT = LD;
  return;
}

string User::getLoginDataForClient()
{
  return LOGIN_DATA_FOR_CLIENT;
}
