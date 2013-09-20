#ifndef ROOM_H
#define ROOM_H

#include <string>
using namespace std;

class Room
{
public:
	Room(int maxu = 100, int maxc = 300);
	Room(const Room &x);
	~Room();

	void setID( long long int id);
	long long int getID();

	void free();
	void setMaxClient(int max);
	void setMaxUser(int max);
	int getMaxClient();
	int getMaxUser();
	void Init();

	bool addUser( long long int id);
	bool delUser( long long int id);
	int getNumUser();

	bool addClient( long long int id);
	bool delClient( long long int id);
	int getNumClient();

	void addToHistory(string text);
  string getHistory();
  void clearHistory();

  void setHash(string hash);
  string getHash();
  void clearHash();

	long long int *UserMembersTab;
	long long int *ClientMembersTab;
private:
  string HASH;
	long long int ID;
	int MaxUser;//default 100
	int MaxClient;//default 300
	int NumUser;//number of logged-in user-members
	int NumClient;//number of logged-in client-members
	string History;//default : empty
protected:

};

#endif
