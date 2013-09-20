#define _REENTRANT
#include "settings.h"

Settings::Settings()
{
  NextID = 0;
  NextRID = 0;
  NextCID = 0;
  setPortNumber(15073);
  setCommunicationBufferSize(2048);
  NumberOfThreads = 0;
  setMaxGroup(50);
  setMaxClient(30);
  setMaxUser(10);
  setMaxMrBug(100);
  setMaxThread(140);
  setMaxSockTab(160);
  setMaxWait(5);
  setMaxRoom(100);
  SockTab = new int[MaxSockTab];
  NumUser = 0;
  NumSockTab = 0;
  NumMrBug = 0;
  NumClient = 0;
  ///resetowanie SockTab
  for(int i = 0; i < MaxSockTab; i++)
  {
    SockTab[i] = -1;
  }
  ///koniec resetowania SockTab
  ClientTab = new Client[MaxClient];
  GroupTab = new Group[MaxGroup];
  UserTab = new User[MaxUser];
  MrBugTab = new MrBug[MaxMrBug];
  RoomTab = new Room[MaxRoom];
  //ustaw odpowiednio ilosc userow i clientow, aby nie bylo potem wyciekow
  for(int i = 0; i < MaxRoom; i++)
  {
    RoomTab[i].setMaxUser(MaxUser);
    RoomTab[i].setMaxClient(MaxClient);
  }
  setCurrentServerVersion("lc-server-dev-4");
  setDBPrefix("no_client_");
  setDbID("no_db_id");
  setGeoRange(3);
}

void Settings::init()
{
  NextID = 0;
  NextRID = 0;
  delete [] ClientTab;
  ClientTab = NULL;
  delete [] GroupTab;
  GroupTab = NULL;
  delete [] UserTab;
  UserTab = NULL;
  delete [] MrBugTab;
  MrBugTab = NULL;
  ///usuwanie, tworzenie i resetowanie SockTab
  delete [] SockTab;
  SockTab = NULL;
  delete [] RoomTab;
  RoomTab = NULL;
  MaxSockTab = MaxClient + MaxUser + MaxMrBug + 20;
  MaxThread = MaxClient + MaxUser + MaxMrBug + 10;
  SockTab = new int[MaxSockTab];
  for(int i = 0; i < MaxSockTab; i++)
  {
    SockTab[i] = -1;
  }
  ///koniec usuwania, tworzenia, resetowania SockTab
  ClientTab = new Client[MaxClient];
  GroupTab = new Group[MaxGroup];
  UserTab = new User[MaxUser];
  MrBugTab = new MrBug[MaxMrBug];
  RoomTab = new Room[MaxRoom];
  //ustaw odpowiednio ilosc userow i clientow, aby nie bylo potem wyciekow
  for(int i = 0; i < MaxRoom; i++)
  {
    RoomTab[i].setMaxUser(MaxUser);
    RoomTab[i].setMaxClient(MaxClient);
  }
  NumUser = 0;
  NumSockTab = 0;
  NumMrBug = 0;
  NumClient = 0;
  NumRoom = 0;
}

int Settings::getNumClient()
{
  return NumClient;
}

void Settings::addNumClient()
{
  NumClient++;
  return;
}

void Settings::subNumClient()
{
  NumClient--;
  return;
}

int Settings::getNumMrBug()
{
  return NumMrBug;
}

void Settings::addNumMrBug()
{
  NumMrBug++;
  return;
}

void Settings::subNumMrBug()
{
  NumMrBug--;
  return;
}

int Settings::getNumSockTab()
{
  return NumSockTab;
}

void Settings::addNumSockTab()
{
  NumSockTab++;
  return;
}

void Settings::subNumSockTab()
{
  NumSockTab--;
  return;
}

int Settings::getNumUser()
{
  return NumUser;
}

void Settings::addNumUser()
{
  NumUser++;
  return;
}

void Settings::subNumUser()
{
  NumUser--;
  return;
}

void Settings::setMaxWait(int max)
{
  MaxWait = max;
}

int Settings::getMaxWait()
{
  return MaxWait;
}

int Settings::getMaxSockTab()
{
  return MaxSockTab;
}

void Settings::setMaxSockTab(int max)
{
  MaxSockTab = max;
}

int Settings::getMaxMrBug()
{
  return MaxMrBug;
}

void Settings::setMaxMrBug(int max)
{
  MaxMrBug = max;
  return;
}

int Settings::getMaxThread()
{
  return MaxThread;
}

void Settings::setMaxThread(int max)
{
  if(max > 0)
  {
    MaxThread = max;
  }
  return;
}

void Settings::setMaxUser(int max)
{
  if(max > 0)
  {
    MaxUser = max;
  }
  return;
}

int Settings::getMaxClient()
{
  return MaxClient;
}

void Settings::setMaxClient(int max)
{
  if(max > 0)
  {
    MaxClient = max;
  }
  return;
}

int Settings::getMaxGroup()
{
  return MaxGroup;
}

void Settings::setMaxGroup(int max)
{
  if(max > 0)
  {
    MaxGroup = max;
  }
  return;
}

Settings::~Settings()
{
  delete [] ClientTab;
  ClientTab = NULL;
  delete [] GroupTab;
  GroupTab = NULL;
  delete [] UserTab;
  UserTab = NULL;
  delete [] MrBugTab;
  MrBugTab = NULL;
  ///usuwanie, tworzenie i resetowanie SockTab
  delete [] SockTab;
  SockTab = NULL;
}

int Settings::getMaxUser()
{
  return MaxUser;
}

int Settings::getPortNumber() const
{
  return PortNumber;
}

void Settings::setPortNumber(int port_no)
{
  PortNumber = port_no;
  return;
}

int Settings::getCommunicationBufferSize() const
{
  return CommunicationBufferSize;
}

void Settings::setCommunicationBufferSize(int communication_buffer_size)
{
  CommunicationBufferSize = communication_buffer_size;
  return;
}

int Settings::getNumberOfThreads() const
{
  return NumberOfThreads;
}

void Settings::addThreadNumber()
{
  NumberOfThreads++;
  return;
}

bool Settings::subThreadNumber()
{
  if(NumberOfThreads > 0)
  {
    NumberOfThreads--;
    return true;
  }
  else
  {
    return false;
  }
}

ostream & operator <<(ostream &out, Settings const &my_settings)
{
  out << "\nCurrent Settings:\n" << endl;
  out << "Port number: " << my_settings.getPortNumber() << endl;
  out << "Communication Buffer Size: " << my_settings.getCommunicationBufferSize() << endl;
  out << "Number Of Threads: " << my_settings.getNumberOfThreads() << endl;
  return out;
}

bool Settings::addFdToSockTab(int fd)
{
  for(int i = 0; i < MaxSockTab; i++)
  {
    if(SockTab[i] == -1)
    {
      //podwojne, tym razem zamutexowane sprawdzenie, aby nie bylo jazd ze juz cos zmienilo a to zas nadpisalo...
      bool ok = false;
      if(SockTab[i] == -1)
      {
        SockTab[i] = fd;
        ok = true;
        addNumSockTab();
      }
      if(ok == true)
      {
        return true;
      }
      else if(i == MaxSockTab - 1)
      {
        return false;
      }
    }
  }
  return false;
}

void Settings::deleteFdFromSockTab(int fd)
{
  for(int i = 0; i < MaxSockTab; i++)
  {
    if(SockTab[i] == fd)
    {
      //zakladamy ze inne watki tego nie usuwaja, wiec mozemy bezpiecznie zalozyc , ze skoro bylo w momencie znalezienia, to jest nadal
      SockTab[i] = -1;
      subNumSockTab();
      break;
    }
  }
  return;
}

bool Settings::checkSockTabForFd(int fd)
{
  ///Narazie wystarcza 99% pewnosci ze fd istnieje/nie w bazie (w miare mozliwosci inne watki nie usuwaja nie swoich fd
  for(int i = 0; i < MaxSockTab; i++)
  {
    if(SockTab[i] == fd)
    {
      return true;
    }
  }
  return false;
}

bool Settings::addUserToUserTab(int fd, long long int id)
{
  //mutex10
  if(id > MaxUser - 1)
  {
    return false;
  }
  if(UserTab[id].getID() == -1)
  {
    bool ok = false;
    if(UserTab[id].getID() == -1)
    {
      UserTab[id].setID(id);
      UserTab[id].setFD(fd);
      addNumUser();
      ok = true;
    }
    if(ok == true)
    {

      return true;
    }
  }
  return false;
}

void Settings::deleteUserFromUserTab(int fd)
{
  for(int i = 0; i < MaxUser; i++)
  {
    if(UserTab[i].getFD() == fd)
    {

      UserTab[i].free();
      subNumUser();

      break;
    }
  }
  return;
}

void Settings::deleteUserFromUserTabById(long long int id)
{
  if(id >= 0 && id < MaxUser)
  {
    if(UserTab[id].getID() == id)
    {
      UserTab[id].free();
      subNumUser();
    }
  }
  return;
}

bool Settings::checkUserTabForUser(int fd)
{
  for(int i = 0; i < MaxUser; i++)
  {
    if(UserTab[i].getFD() == fd)
    {
      return true;
    }
  }
  return false;
}

bool Settings::checkUserTabForUserById(long long int id)
{
  if(id >= 0 && id < MaxUser)
  {
    if(UserTab[id].getID() == id)
    {
      return true;
    }
  }
  return false;
}

int Settings::getUserFDbyID(long long int id)
{
  //mutex 10
  if(id >= 0 && id < MaxUser)
  {
    if(UserTab[id].getID() == id)
    {
      return UserTab[id].getFD();
    }
  }
  return -1;
}

bool Settings::addClientToClientTab(int fd, long long int id)
{
  //mutex11
  if(id > MaxClient - 1)
  {
    return false;
  }
  if(ClientTab[id].getID() == -1)
  {
    bool ok = false;
    if(ClientTab[id].getID() == -1)
    {
      ClientTab[id].setID(id);
      ClientTab[id].setFD(fd);
      addNumClient();
      ok = true;
    }
    if(ok == true)
    {
      return true;
    }
  }
  return false;
}

void Settings::deleteClientFromClientTab(int fd)
{
  for(int i = 0; i < MaxClient; i++)
  {
    if(ClientTab[i].getFD() == fd)
    {
      ClientTab[i].free();
      subNumClient();
      break;
    }
  }
  return;
}

void Settings::deleteClientFromClientTabById(long long int id)
{
  if(ClientTab[id].getID() == id)
  {
    ClientTab[id].free();
    subNumClient();
  }
  return;
}

bool Settings::checkClientTabForClient(int fd)
{
  for(int i = 0; i < MaxClient; i++)
  {
    if(ClientTab[i].getFD() == fd)
    {
      return true;
    }
  }
  return false;
}

bool Settings::checkClientTabForClientById(long long int id)
{
  if(ClientTab[id].getID() == id)
  {
    return true;
  }
  return false;
}

int Settings::getClientFDbyID(long long int id)
{
  if(id >= 0 && id < MaxClient)
  {
    if(ClientTab[id].getID() == id)
    {
      return ClientTab[id].getFD();
    }
  }
}

bool Settings::addMrBugToMrBugTab(int fd, long long int id)
{
  if(id > MaxMrBug - 1)
  {
    return false;
  }
  if(MrBugTab[id].getID() == -1)
  {
    bool ok = false;
    if(MrBugTab[id].getID() == -1)
    {
      MrBugTab[id].setFD(fd);
      MrBugTab[id].setID(id);
      addNumMrBug();
      ok = true;
    }
    if(ok == true)
    {
      return true;
    }
  }
  return false;
}

void Settings::deleteMrBugFromMrBugTab(int fd)
{
  for(int i = 0; i < MaxMrBug; i++)
  {
    if(MrBugTab[i].getFD() == fd)
    {
      MrBugTab[i].free();
      subNumMrBug();
    }
  }
  return;
}

void Settings::deleteMrBugFromMrBugTabById(long long int id)
{
  if(id >= 0 && id < MaxMrBug)
  {
    if(MrBugTab[id].getID() == id)
    {
      MrBugTab[id].free();
      subNumMrBug();
    }
  }
  return;
}

bool Settings::checkMrBugTabForMrBug(int fd)
{
  for(int i = 0; i < MaxMrBug; i++)
  {
    if(MrBugTab[i].getFD() == fd)
    {
      return true;
    }
  }
  return false;
}

bool Settings::checkMrBugTabForMrBugById(long long int id)
{
  if(id >= 0 && id < MaxMrBug)
  {
    if(MrBugTab[id].getID() == id)
    {
      return true;
    }
  }
  return false;
}

int Settings::getMaxRoom()
{
  return MaxRoom;
}

void Settings::setMaxRoom(int max)
{
  MaxRoom = max;
  return;
}

int Settings::getNumRoomTab()
{
  return NumRoom;
}

void Settings::addNumRoomTab()
{
  NumRoom++;
  return;
}

void Settings::subNumRoomTab()
{
  NumRoom--;
  return;
}

void Settings::openRoom(long long int &id)
{
  id = -1;
  for(long long int i = 0; i < MaxRoom; i++)
  {
    if(RoomTab[i].getID() == -1)
    {
      id = i;
      break;
    }
  }
  if(RoomTab[id].getID() == -1)
  {
    RoomTab[id].setID(id);
    addNumRoomTab();
  }
  return;
}

void Settings::closeRoom(long long int id)
{
  //zamyka pokoj o podanym id
  if(RoomTab[id].getID() == id)
  {
    RoomTab[id].free();
    subNumRoomTab();
  }
  return;
}

void Settings::nextID(long long int &id)
{
  id = NextID;
  NextID++;
  return;
}

void Settings::nextUID(long long int &uid)
{
  if(UserTab[uid].getID() != -1)
  {
    uid = -1;
  }
  return;
}

void Settings::nextCID(long long int &cid)
{
  cid = -1;
  for(long long int i = 0; i < MaxClient; i++)
  {
    if(ClientTab[i].getID() == -1)
    {
      cid = i;
      break;
    }
  }
  return;
}

void Settings::nextMID(long long int &mid)
{
  mid = -1;
  for(long long int i = 0; i < MaxMrBug; i++)
  {
    if(MrBugTab[i].getID() == -1)
    {
      mid = i;
      break;
    }
  }
  return;
}

void Settings::setCurrentServerVersion(string csv)
{
  CurrentServerVersion = csv;
}

//void Settings::setMaxClientVersion(int mcv)
//{
//  MaxClientVersion = mcv;
//}
//
//void Settings::setMinClientVersion(int mcv)
//{
//  MinClientVersion = mcv;
//}

void Settings::getCurrentServerVersion(string &csv)
{
  csv = CurrentServerVersion;
}

/*void Settings::getMaxClientVersion(int &mcv)
{
  mcv = CurrentServerVersion;
}*/

/*void Settings::getMinClientVersion(int &mcv)
{
  mcv = MinClientVersion;
}*/

void Settings::delClientFromRoomTab(long long int &rid, long long int &cid)
{
  if(rid < MaxRoom && cid < MaxClient && rid >-1 && cid >-1)
  {
    RoomTab[rid].delClient(cid);
  }
  return;
}

void Settings::delUserFromRoomTab(long long int &rid, long long int &uid)
{
  if(rid < MaxRoom && uid < MaxUser && rid >-1 && uid >-1)
  {
    RoomTab[rid].delUser(uid);
  }
  return;
}

void Settings::addUserToRoomTab(long long int rid, long long int uid, bool &error)
{
  error = true;
  if(rid < MaxRoom && rid >= 0 && uid >= 0 && uid < MaxUser)
  {
    if(RoomTab[rid].addUser(uid))
    {
      error = false;
    }
  }
  return;
}

void Settings::addClientToRoomTab(long long int rid, long long int cid, bool &error)
{
  error = true;
  if(rid < MaxRoom && cid < MaxClient)
  {
    if(RoomTab[rid].addClient(cid))
    {
      error = false;
    }
  }
  return;
}

void Settings::setDBPrefix(string prefix)
{
  DBPREFIX = prefix;
  return;
}

string Settings::getDBPrefix()
{
  return DBPREFIX;
}

void Settings::setDbID(string dbid)
{
  DB_ID = dbid;
  return;
}

string Settings::getDbID()
{
  return this->DB_ID;
}

void Settings::setGeoRange(int geo_range)
{
  this->GEO_RANGE = geo_range;
}

int Settings::getGeoRange()
{
  return this->GEO_RANGE;
}
