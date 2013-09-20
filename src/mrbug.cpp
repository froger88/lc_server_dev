
#include <pthread.h>
#include <string>
#include <netinet/tcp.h>

#include "mrbug.h"

//extern pthread_mutex_t intToStr_mutex;                                        USUNIETO
//extern pthread_mutex_t ipStrToInt_mutex;                                      USUNIETO
//extern pthread_mutex_t strToInt_mutex;                                        USUNIETO
extern pthread_mutex_t mrbug_mutex;

MrBug::MrBug()
{
  FD = -1;
  ID = -1;
  KICK = false;
  IP.clear();
  GEODATA.clear();
  BUGDATA.clear();
  COMPLETEDATA.clear();
  TimeIn = "0";
}

MrBug::MrBug(const MrBug &x)
{
  FD = FD;
  ID = ID;
  KICK = x.KICK;
  IP = x.IP;
  GEODATA = x.GEODATA;
  BUGDATA = x.BUGDATA;
  COMPLETEDATA = x.COMPLETEDATA;
  TimeIn = x.TimeIn;
  SendQueue = x.SendQueue;
}

MrBug::~MrBug()
{

}

void MrBug::setID(long long int id)
{
  ID = id;
  return;
}

long long int MrBug::getID() const
{
  return ID;
}

void MrBug::getID(long long int& id)
{
  id = ID;
  return;
}

int MrBug::getFD() const
{
  return FD;
}

void MrBug::getFD(long long int& fd)
{
  fd = FD;
  return;
}

void MrBug::setFD(int fd)
{
  FD = fd;
  return;
}

void MrBug::free()
{
  FD = -1;
  ID = -1;
  IP.clear();
  GEODATA.clear();
  BUGDATA.clear();
  COMPLETEDATA.clear();
  KICK = false;
  TimeIn = "0";
  while(!SendQueue.empty())
  {
    SendQueue.pop();
  }
  return;
}

void MrBug::kick()
{
  KICK = true;
  return;
}

bool MrBug::getKickState() const
{
  return KICK;
}

void MrBug::getKickState(bool& kstate)
{
  kstate = KICK;
  return;
}

void MrBug::getGeoData(int geo_port) ///ZABEZPIECZYC !!!!!!!!!!!!!
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
  if((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
  {
    //cout << "GEO_STOP_FAIL" << endl << endl;
    pthread_mutex_lock(&mrbug_mutex);
    GEODATA = "value=FAIL";
    pthread_mutex_unlock(&mrbug_mutex);
    return;
  }
  FD_ZERO(&readmask);
  FD_SET(fd, &readmask);

  memset(&servaddr, 0, sizeof (servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);

  if(inet_aton("127.0.0.1", &servaddr.sin_addr) == 0)
  {
    //
    ////cout << "  GEO_STOP_FAIL" << endl << endl;
    pthread_mutex_lock(&mrbug_mutex);
    GEODATA = "geo=false";
    pthread_mutex_unlock(&mrbug_mutex);
    close(fd);
    return;
  }
  /*  connect() to the geo-server  */

  if(connect(fd, (struct sockaddr *) & servaddr, sizeof (servaddr)) != 0)
  {
    //cout << "  GEO_STOP_FAIL" << endl << endl;
    pthread_mutex_lock(&mrbug_mutex);
    GEODATA = "geo=false";
    pthread_mutex_unlock(&mrbug_mutex);
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
        pthread_mutex_lock(&mrbug_mutex);
        GEODATA = "geo=false";
        pthread_mutex_unlock(&mrbug_mutex);
        close(fd);
        return;
      }
      pthread_mutex_lock(&mrbug_mutex);
      GEODATA.assign(buffer);
      if(GEODATA.empty() == true)
      {
        GEODATA = "geo=false";
      }
      pthread_mutex_unlock(&mrbug_mutex);
      close(fd);
      return;
    }
  }
  else
  {
    pthread_mutex_lock(&mrbug_mutex);
    GEODATA = "geo=false";
    pthread_mutex_unlock(&mrbug_mutex);
    close(fd);
    return;
  }
}

void MrBug::setIP(string IP)
{
  this->IP = IP;
  return;
}

void MrBug::getGeoFromVariable(string& geodata)
{
  geodata = this->GEODATA;
  return;
}

void MrBug::setBugData(string Bugdata)
{
  BUGDATA = Bugdata;
  //zamien pierwsyz znak na & aby potem ladnie dokleilo
  if(BUGDATA[0] == '*')
  {
    BUGDATA[0] = '&';
  }
  return;
}

void MrBug::setCompleteData()
{
  COMPLETEDATA = "id=" + intToStr(ID) + "&" + GEODATA + "&ip=" + IP + "&t=" + TimeIn + BUGDATA; //bugdata ma postac &...
  return;
}

void MrBug::getCompleteData(string& CData)
{
  CData = COMPLETEDATA;
  return;
}

void MrBug::getBugData(string& BData)
{
  BData = BUGDATA;
  return;
}

string MrBug::getIP()
{
  return IP;
}

void MrBug::setTimeIn(string TimeIn)
{
  this->TimeIn = TimeIn;
  return;
}

