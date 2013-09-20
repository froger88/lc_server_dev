#ifndef SETTINGS_H
#define SETTINGS_H

#include <ostream>

#include "user.h"
#include "client.h"
#include "group.h"
#include "level.h"
#include "mrbug.h"
#include "room.h"

using namespace std;

class Settings
{
public:
	//constructor and destructor
	Settings();
	~Settings();

	//other functions
	int getPortNumber() const;
	void setPortNumber(int port_no);

	int getCommunicationBufferSize() const;
	void setCommunicationBufferSize(int communication_buffer_size);

  void setDBPrefix(string prefix);
  string getDBPrefix();

	int getNumberOfThreads() const;
	void addThreadNumber();
	bool subThreadNumber();

	int getMaxGroup();
	void setMaxGroup(int max);

	int getMaxClient();
	void setMaxClient(int max);

	int getMaxUser();
	void setMaxUser(int max);

	int getMaxThread();
	void setMaxThread(int max);

	int getMaxMrBug();
	void setMaxMrBug(int max);

	int getMaxSockTab();
	void setMaxSockTab(int max);

	int getMaxWait();
	void setMaxWait(int max);

	int getMaxRoom();
	void setMaxRoom(int max);

	bool addFdToSockTab(int fd);//return true if ok
	void deleteFdFromSockTab(int fd);
	bool checkSockTabForFd(int fd);//return true if exists

	bool addUserToUserTab(int fd,  long long int id);
	void deleteUserFromUserTab(int fd);
	void deleteUserFromUserTabById( long long int id);
	bool checkUserTabForUser(int fd);
	bool checkUserTabForUserById( long long int id);
	int getUserFDbyID( long long int id);

	bool addClientToClientTab(int fd, long long  int id);
	void deleteClientFromClientTab(int fd);
	void deleteClientFromClientTabById( long long int id);
	bool checkClientTabForClient(int fd);
	bool checkClientTabForClientById( long long int id);
	int getClientFDbyID(long long int id);

	bool addMrBugToMrBugTab(int fd,  long long int id);
	void deleteMrBugFromMrBugTab(int fd);
	void deleteMrBugFromMrBugTabById( long long int id);
	bool checkMrBugTabForMrBug(int fd);
	bool checkMrBugTabForMrBugById( long long int id);

	int getNumUser();
	void addNumUser();
	void subNumUser();

	int getNumClient();
	void addNumClient();
	void subNumClient();

	int getNumMrBug();
	void addNumMrBug();
	void subNumMrBug();

	int getNumSockTab();
	void addNumSockTab();
	void subNumSockTab();

	int getNumRoomTab();
	void addNumRoomTab();
	void subNumRoomTab();

	void openRoom( long long int &id);
	void closeRoom( long long int id);
	//operators
	friend ostream &operator << (ostream &out, Settings const &my_settings);
	//arrays and variables
	int* SockTab;

	void nextID( long long int &id);//return next ID as reference to id
	void nextUID( long long int &uid);//return next UserID as reference to uid
	//void nextRID( long long int &rid); //return next RoomID as reference to rid
	void nextCID( long long int &cid);//return next ClientID as reference to cid
	void nextMID( long long int &mid);//return next MrBugID as reference to mid
	void init();

	void setCurrentServerVersion(string csv);
//	void setMaxClientVersion(int mcv);
//	void setMinClientVersion(int mcv);

	void getCurrentServerVersion(string &csv);
//	void getMaxClientVersion(int &mcv);
//	void getMinClientVersion(int &mcv);

	void delClientFromRoomTab(long long int &rid, long long int &cid);
	void delUserFromRoomTab(long long int &rid, long long int &uid);

	void addUserToRoomTab(long long int rid, long long int uid, bool &error);
	void addClientToRoomTab(long long int rid, long long int cid, bool &error);


  void setDbID(string dbid);
  string getDbID();

  void setGeoRange(int geo_range=3);
  int getGeoRange();

	Client *ClientTab;//default : 300
	Group *GroupTab;//default : 50
	User *UserTab;//default :100
	MrBug *MrBugTab;//default : 10
	Room *RoomTab;//default 100;
private:
	int PortNumber; //default : 15073
	int CommunicationBufferSize;//default : 2048
	int NumberOfThreads;// 0 at start
	int MaxGroup;//default : 50
	int MaxClient;//default : 300
	int MaxUser; //default : 100
	int MaxRoom;//default : 100
	int MaxThread;//default : 200
	int MaxMrBug;//default : 10
	int MaxSockTab;//default : 310 (MaxMrBug + MaxClient)
	int MaxWait;//default : 5
	int NumUser;//number of actually logged-in users
	int NumClient;//number of actually logged-in Clients (guests)
	int NumMrBug;//number of actually logged-in
	int NumSockTab;//number of actually reserved slots in SocTab
	int NumRoom;//number of actually open rooms
	long long int NextID;//next ID
	long long int NextRID;//next RoomID
	long long int NextCID;//next ClientID

	string CurrentServerVersion;//lc-server-dev-4
	//float MaxClientVersion;//default CurrentServerVersion
	//float MinClientVersion;//default 1.000
  int GEO_RANGE;//default 3
  string DBPREFIX;
  string DB_ID;
protected:
};

#endif
