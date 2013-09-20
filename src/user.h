#ifndef USER_H
#define USER_H

#include <iostream>
#include <string>

#include "level.h"
#include "group.h"
#include "status.h"
#include <queue>

using namespace std;

class User
{
public:
  //konstruktory
  User();
  User(const User &x);
  ~User();
  //funkcje
  //on me operations
  void free();
  void setID(long long int new_id);
  void setFD(int new_sockfd);
  void setNick(string nick);
  void setLevel(int min = 1, int max = 1);
  Level& getLevel(); //deprecated
  void getLevel(Level &lvl);
  int getMaxLevel() const; //deprecated
  void getMaxLevel(int &lvl);
  int getMinLevel() const; //deprecated
  void getMinLevel(int& lvl);

  long long int getID() const; //deprecated
  void getID(long long int &id);
  int getFD() const; //deprecated
  void getFD(long long int &fd);
  string getNick() const; //deprecated
  void getNick(string &nick);
  //on group operations
  void addToGroup(Group& g);
  void delFromGroup(Group& g);
  bool checkInGroup(Group& g) const; //deprecated
  void checkInGroup(Group& g, bool &inside);

  User & operator =(const User &x);
  friend bool operator ==(User &left, User &right);
  friend bool operator !=(User &left, User &right);

  void kick();
  bool getKickState() const; //deprecated
  void getKickState(bool &kstate);

  void setLoginData(string LD);
  string getLoginData();

  void setLoginDataForClient(string LD);
  string getLoginDataForClient();



  ///-----------------UWAGA-----------------------------
  //-----------TABLICA InRoom nie jest kontrolowana z poziomu klasy! (jedynie destruktor)
  long long int *InRoom;
  Status status;

    //jeszcze nie uzywane!
    queue<string> SendQueue;
  ///-----------------UWAGA-----------------------------
private:
  //my ID and my FD
  long long int ID;
  int FD;
  bool KICK;
  string LOGIN_DATA;
  string LOGIN_DATA_FOR_CLIENT;

  Level LVL;
  string NICK;

protected:
};

#endif
