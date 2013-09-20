
#include <fstream>


#include <pthread.h>

#include "client.h"
#include "thread_connect.h"

using namespace std;

//extern pthread_mutex_t intToStr_mutex;                                        USUNIETO
//extern pthread_mutex_t ipStrToInt_mutex;                                      USUNIETO
//extern pthread_mutex_t strToInt_mutex;                                        USUNIETO
extern pthread_mutex_t client_mutex;

Client::Client()
{
  ID = -1;
  FD = -1;
  ABOUT = "NO_INFO";
  NICK = "NOT_NAMED";
  InRoom = -1;
  KICK = false;
  IP = "0.0.0.0";

  UserTalkWith = -1;

  CLIENTDATA.clear();
  COMPLETEDATA.clear();
  GEODATA.clear();
  RIDSTR = "NOT_SET";
  TimeIn = "0";
  ROOMHASH.clear();
  clearHistory();
  DepID = -1;
}

Client::Client(const Client& x)//pomijamy ID ,FD
{
  this->ABOUT = x.ABOUT;
  this->NICK = x.NICK;
  this->InRoom = x.InRoom;
  this->KICK = x.KICK;
  this->IP = x.IP;
  this->CLIENTDATA = x.CLIENTDATA;
  this->COMPLETEDATA = x.COMPLETEDATA;
  this->GEODATA = x.GEODATA;
  this->RIDSTR = x.RIDSTR;
  this->UserTalkWith = x.UserTalkWith;
  this->HISTORY = x.HISTORY;
  this->TimeIn = x.TimeIn;
  this->ROOMHASH = x.ROOMHASH;
  this->SendQueue = x.SendQueue;
  this->DepID = x.DepID;
}

Client::~Client()
{

}

//funkcje
//-------------------------------UWAGA-------------------------------
//poprawnosc ID nie jest sprawdzana!

void Client::free()
{
  ID = -1;
  FD = -1;
  ABOUT = "NO_INFO";
  NICK = "NOT_NAMED";
  KICK = false;
  IP = "0.0.0.0";
  InRoom = -1;

  CLIENTDATA.clear();
  COMPLETEDATA.clear();
  GEODATA.clear();
  RIDSTR = "NOT_SET";
  UserTalkWith = -1;
  clearHistory();
  TimeIn = "0";
  ROOMHASH.clear();
  while(!SendQueue.empty())
  {
    SendQueue.pop();
  }
  DepID = -1;
  return;
}

void Client::setID(long long int id)
{
  ID = id;
  return;
}

//--------------------------------------------------------------------------

void Client::setFD(int fd)
{
  FD = fd;
  return;
}

void Client::setNick(string nick)
{
  NICK = nick;
  return;
}

void Client::setAbout(string about)
{
  ABOUT = about;
}

int Client::getFD() const
{
  return FD;
}

void Client::getFD(int& fd)
{
  fd = FD;
  return;
}

long long int Client::getID() const
{
  return ID;
}

void Client::getID(long long int &id)
{
  id = ID;
  return;
}

string Client::getAbout() const
{
  return ABOUT;
}

void Client::getAbout(string& about)
{
  about = ABOUT;
  return;
}

string Client::getNick() const
{
  return NICK;
}

void Client::getNick(string &nick)
{
  nick = NICK;
  return;
}

void Client::kick()
{
  KICK = true;
  return;
}

bool Client::getKickState() const
{
  return KICK;
}

void Client::getKickState(bool& kstate)
{
  kstate = KICK;
  return;
}

void Client::addToHistory(string text)
{
  HISTORY += text;
  return;
}

string Client::getHistory()
{
  return HISTORY;
}

void Client::getHistory(string& history)
{
  history = HISTORY;
  return;
}

void Client::setIP(string ip)
{
  IP = ip;
  return;
}

string Client::getIP()
{
  return IP;
}

void Client::getIP(string& ip)
{
  ip = IP;
  return;
}

void Client::getGeoFromVariable(string& geodata)
{
  geodata = GEODATA;
  return;
}

void Client::setClientData(string ClientData)
{
  CLIENTDATA = ClientData;
  //zamien pierwsyz znak na & aby potem ladnie dokleilo
  if(CLIENTDATA[0] == '+')
  {
    CLIENTDATA[0] = '&';
  }
  return;
}

void Client::setCompleteData()
{
  COMPLETEDATA = "id=" + intToStr(ID) + "&" + GEODATA + "&rid=" + RIDSTR + "&h=" + ROOMHASH + "&ip=" + IP + "&t=" + TimeIn + "&dep=" + intToStr(DepID) + CLIENTDATA; //CLIENTDATA ma postac &...
  return;
}

void Client::getCompleteData(string& CData)
{
  CData = COMPLETEDATA;
  return;
}

void Client::getClientData(string& ClientData)
{
  ClientData = CLIENTDATA;
  return;
}

void Client::setRidStr(string RidStr)
{
  RIDSTR = RidStr;
  return;
}

void Client::getGeoData(int geo_port) ///ZABEZPIECZYC !!!!!!!!!!!!!
{
  int fd;
  int port = geo_port;
  struct sockaddr_in servaddr;
  char buffer[1024];

  //USTAWIENIA TL
  timespec TL;
  TL.tv_sec = 1; //seconds
  TL.tv_nsec = 0; //nano seconds

  timespec *timeout = &TL;

  fd_set readmask;
  ///KONIEC USTAWIANIA TL

  //cout << "GEO_START" << endl;
  if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    //cout << "GEO_STOP_FAIL" << endl << endl;
    pthread_mutex_lock(&client_mutex);
    GEODATA = "geo=false";
    pthread_mutex_unlock(&client_mutex);
    return;
  }
  FD_ZERO(&readmask);
  FD_SET(fd, &readmask);

  memset(&servaddr, 0, sizeof (servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);

  if(inet_aton("127.0.0.1", &servaddr.sin_addr) == 0)
  {
    pthread_mutex_lock(&client_mutex);
    GEODATA = "geo=false";
    pthread_mutex_unlock(&client_mutex);
    close(fd);
    return;
  }
  /*  connect() to the geo-server  */

  if(connect(fd, (struct sockaddr *) & servaddr, sizeof (servaddr)) != 0)
  {
    pthread_mutex_lock(&client_mutex);
    GEODATA = "geo=false";
    pthread_mutex_unlock(&client_mutex);
    close(fd);
    return;
  }

  //konwertuj IP
  string IPNumStr;
  long long int IPNum;

  ipStrToInt(IP, IPNum, IPNumStr);

  //wyslij IP
  if(write(fd, IPNumStr.c_str(), IPNumStr.length()) > 0)
  {
    //odbierz geo-date
    int nfound = 0;
    FD_ZERO(&readmask);
    FD_SET(fd, &readmask);
    memset(buffer, NULL, 1024);
    nfound = pselect(sizeof (readmask)*8, &readmask, NULL, &readmask, timeout, NULL);

    if(nfound == 0)//jezeli TL przekroczony
    {
      //exit now!
      close(fd);
      return;
    }
    else
    {
      int n;
      n = read(fd, buffer, 1024);
      if(n <= 0)
      {
        pthread_mutex_lock(&client_mutex);
        GEODATA = "geo=false";
        pthread_mutex_unlock(&client_mutex);
        close(fd);
        return;
      }
      pthread_mutex_lock(&client_mutex);
      GEODATA.assign(buffer);
      if(GEODATA.empty() == true)
      {
        GEODATA = "geo=false";
      }
      pthread_mutex_unlock(&client_mutex);
      close(fd);
      return;
    }
  }
  else
  {
    pthread_mutex_lock(&client_mutex);
    GEODATA = "geo=false";
    pthread_mutex_unlock(&client_mutex);
    close(fd);
    return;
  }
}

void Client::setUserTalkWith(long long int id)
{
  if(UserTalkWith == -1)
  {
    this->UserTalkWith = id;
  }
}

long long int Client::getUserTalkWith()
{
  return this->UserTalkWith;
}

void Client::clearHistory()
{
  HISTORY.clear();
  return;
}

void Client::setTimeIn(string TimeIn)
{
  this->TimeIn = TimeIn;
  return;
}

void Client::setRoomHash(string RoomHash)
{
  this->ROOMHASH = RoomHash;
  return;
}

void Client::setDepartmentID(long long int DepID)
{
  if(DepID >= -1)
  {
    this->DepID = DepID;
  }
  return;
}

long long int Client::getDepartmentID()
{
  return this->DepID;
}

void Client::modifyStopParameterInsideCLIENTDATA(char value)
{
  int pos = CLIENTDATA.find("&stop=");
  if(pos != string::npos)
  {
    CLIENTDATA[pos + 6] = value;
  }
  return;
}

