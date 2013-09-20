#ifndef MRBUG_H
#define MRBUG_H

#include <string>
#include <iostream>

#include "inttostr.h"
#include "ipstrtoint.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <queue>

using namespace std;

class MrBug
{
public:
  MrBug();
  MrBug(const MrBug &x);
  ~MrBug();

  long long int getID() const; //deprecated
  void getID(long long int &id);
  void setID(long long int id);

  int getFD() const; //deprecated
  void getFD(long long int &fd);
  void setFD(int fd);
  void setIP(string IP);
  string getIP();

  void free();

  void kick();
  bool getKickState() const; //deprecated
  void getKickState(bool &kstate);

  void getGeoData(int geo_port = 15001);
  void getGeoFromVariable(string &geodata);

  void setBugData(string Bugdata);
  void setCompleteData();

  void getCompleteData(string &CData);
  void getBugData(string &BData);

  void setTimeIn(string TimeIn);

  //jeszcze nie uzywane
  queue<string> SendQueue;

private:
  long long int ID;
  string IP;
  int FD;
  string TimeIn;
  string GEODATA;
  string BUGDATA;
  string COMPLETEDATA; //geodata+ip+id+bugdata
  bool KICK; //default - false;

  //jeszcze nie uzywane!
protected:
};

#endif
