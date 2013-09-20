#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <queue>

#include "inttostr.h"
#include "ipstrtoint.h"

using namespace std;

using namespace std;

///--------------------------------------------------
///-----UWAGA: poprawnosc ID nie jest sprawdzana
///-----DOMYSLNE WARTOSCI
///-----ID = -1
///-----FD = -1
///-------------------------------------------------

class Client
{
public:
  //konstruktory i destruktory
  Client();
  Client(const Client& x);
  ~Client();
  //funkcje
  //-------------------------------UWAGA-------------------------------
  //poprawnosc ID nie jest sprawdzana!
  void setID(long long int id);
  //--------------------------------------------------------------------------
  void setFD(int fd);
  void setNick(string nick);
  void setAbout(string about);

  int getFD() const; //deprecated
  void getFD(int &fd);
  long long int getID() const; //deprecated
  void getID(long long int &id);
  string getAbout() const; //deprecated
  void getAbout(string &about);
  string getNick() const; //deprecated
  void getNick(string &nick);

  void free();
  long long int InRoom; //default = -1

  void kick();
  bool getKickState() const; //deprecated
  void getKickState(bool &kstate);

  void addToHistory(string text);
  string getHistory(); //deprecated
  void getHistory(string &history);
  void clearHistory();

  void setIP(string ip);
  string getIP(); //deprecated
  void getIP(string &ip);

  void getGeoData(int geo_port = 15001);
  void getGeoFromVariable(string &geodata);

  void setClientData(string ClientData);
  void setCompleteData();

  void getCompleteData(string &CData);
  void getClientData(string &ClientData);

  void setRidStr(string RidStr);

  void setUserTalkWith(long long int id);
  long long int getUserTalkWith();

  void setTimeIn(string TimeIn);

  void setRoomHash(string RoomHash);

  void setDepartmentID(long long int DepID);
  long long int getDepartmentID();

  void modifyStopParameterInsideCLIENTDATA(char value);
  //jeszcze nie uzywane
  queue<string> SendQueue;

private:
  long long int ID; //client ID
  int FD; //client FileDescriptor
  string ABOUT;
  string NICK;
  bool KICK; //default false
  string HISTORY;
  string IP;

  int UserTalkWith; //default -1

  string GEODATA;
  string CLIENTDATA;
  string COMPLETEDATA; //geodata+ip+id+clientdata+rid
  string RIDSTR;
  string TimeIn;
  string ROOMHASH;
  long long int DepID;
protected:
};

#endif
