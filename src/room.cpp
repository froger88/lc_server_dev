
#include "room.h"

Room::Room(int maxu, int maxc) //maxu = max user, maxc = max client (guest)
{
	MaxUser = maxu;
	MaxClient = maxc;
	NumUser = 0;
	NumClient = 0;
	ID = -1;
	UserMembersTab = new  long long int[MaxUser];
	ClientMembersTab = new  long long int[MaxClient];
  HASH.clear();

	//resetowanie tablicy
	for(int i=0;i<MaxUser;i++)
	{
		UserMembersTab[i] = -1;
	}
	for(int i=0;i<MaxClient;i++)
	{
		ClientMembersTab[i] = -1;
	}
}

Room::Room(const Room &x)
{
	MaxUser = x.MaxUser;
	MaxClient = x.MaxClient;
}

Room::~Room()
{
	delete [] ClientMembersTab;
	ClientMembersTab = NULL;
	delete [] UserMembersTab;
	ClientMembersTab = NULL;
}

void Room::setID(long long int id)
{
	ID = id;
	return;
}

long long int Room::getID()
{
	return ID;
}

void Room::free()
{
	for(int i=0;i<MaxUser;i++)
	{
		UserMembersTab[i] = -1;
	}
	for(int i=0;i<MaxClient;i++)
	{
		ClientMembersTab[i] = -1;
	}

	NumUser = 0;
	NumClient = 0;
	ID = -1;
  HASH.clear();
}

void Room::setMaxClient(int max)
{
	MaxClient = max;
}

void Room::setMaxUser(int max)
{
	MaxUser = max;
	return;
}

int Room::getMaxClient()
{
	return MaxClient;
}

int Room::getMaxUser()
{
	return MaxUser;
}

void Room::Init()
{
	UserMembersTab = new long long int[MaxUser];
	ClientMembersTab = new long long int[MaxClient];
	free();
}

bool Room::addUser(long long int id)
{
	if(NumUser >= MaxUser)
	{
		return false;
	}
	for(int i=0;i<MaxUser;i++)
	{
		if(UserMembersTab[i] == -1)
		{
			UserMembersTab[i] = id;
			NumUser++;
			return true;
		}
	}
	return false;
}

bool Room::delUser( long long int id)
{
	for(int i=0;i<MaxUser;i++)
	{
		if(UserMembersTab[i] == id)
		{
			UserMembersTab[i] = -1;
			NumUser--;
		}
	}
	return true;
}

int Room::getNumUser()
{
	return NumUser;
}

bool Room::addClient( long long int id)
{
	if(NumClient >= MaxClient)
	{
		return false;
	}
	for(int i=0;i<MaxClient;i++)
	{
		if(ClientMembersTab[i] == -1)
		{
			ClientMembersTab[i] = id;
			NumClient++;
			return true;
		}
	}
	return false;
}

bool Room::delClient( long long int id)
{
	for(int i=0; i < MaxClient;i++)
	{
		if(ClientMembersTab[i] == id)
		{
			ClientMembersTab[i] = -1;
			NumClient--;
		}
	}
	return true;
}

int Room::getNumClient()
{
	return NumClient;
}

string Room::getHistory()
{
  return History;
}

void Room::clearHistory()
{
  History.clear();
  return;
}

void Room::addToHistory(string text)
{
  History += text;
}

  void Room::setHash(string hash)
  {
    HASH = hash;
    return;
  }

  string Room::getHash()
  {
    return HASH;
  }

  void Room::clearHash()
  {
    HASH.clear();
    return;
  }
