#include <fstream>

#include "main.h"

using namespace std;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mrbug_mutex = PTHREAD_MUTEX_INITIALIZER; //zabezpiecza kopanie mrbugow
pthread_mutex_t user_mutex = PTHREAD_MUTEX_INITIALIZER; //zabezpiecza kopanie userow
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER; //zabezpiecza kopanie clientow
pthread_mutex_t history_mutex = PTHREAD_MUTEX_INITIALIZER; //zabezpiecza dzialania na historii
pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER; //zabezpiecza dzialania na bazie danych
//-----nie uzywane jeszcze---------   pthread_mutex_t group_mutex = PTHREAD_MUTEX_INITIALIZER; //zabezpiecza dzialania na grupach
pthread_mutex_t room_mutex = PTHREAD_MUTEX_INITIALIZER; //zabezpiecza dzialania na pokojach
pthread_mutex_t time_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t local_bans_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rand_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t stat1_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t stat2_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t domain_check_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t geo_mrbug_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t geo_client_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t options_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t killer_mutex = PTHREAD_MUTEX_INITIALIZER;

Settings Configuration;
CStatistics StatsTab;
string IPLocalBans;
string Domains;
string InvitationData;
bool stats_on;
string client_id;
bool allow_subdomains;
int SHUTDOWN_STATE; // 0 - nie bedzie shutdowna, 1 - w przeciagu 5 min, nie wpuszczac nowych, 2 - wlasnie pad!, dostep chroniony przez mutex

string *__IP;
int *__FD;
int *__PORT;
long *__ID;
string __KILLER;

int main(int argc, char*argv[])
{
  string CROSS = "    ++\n    ++\n    ++\n++++++++++\n++++++++++\n    ++\n    ++\n    ++\n    ++\n    ++\n    ++\n    ++";
  cout << CROSS << endl; //krzyza nic nie ruszy @wydarzenia z sierpnia 2010
  SHUTDOWN_STATE = 0;
  __KILLER.clear();
  pthread_t t_thread, t_thread_stats, t_thread_guard, t_thread_time_events;
  pthread_attr_t tattr_thread, tattr_thread_stats, tattr_thread_guard, tattr_thread_time_events;
  int sockfd, newsockfd;
  size_t client_length;
  char *buffer;
  sockaddr_in server_address;
  sockaddr client_address;
  __KILLER += "wefbdcczx";
  //Settings Configuration;
  //server version:
  Configuration.setCurrentServerVersion("lc-server-dev-alpha");
  //Configuration.setMinClientVersion(0x102B);
  //usage: ./server [DBPREFIX] [DBID] [MAX_USER] [MAX_CLIENT] [MAX_MRBUG] [PORT_NO]
  Configuration.setGeoRange(); //ustawia domyslna ilosc geo-servow na 3
  __KILLER += "asdyasd";
  int PORT_NO = 15073;
  if(argc == 2)
  {
    Configuration.setDBPrefix(argv[1]);
  }
  else if(argc == 3)
  {
    Configuration.setDBPrefix(argv[1]);
    Configuration.setDbID(argv[2]);
  }
  else if(argc == 4)
  {
    int a;
    Configuration.setDBPrefix(argv[1]);
    Configuration.setDbID(argv[2]);
    Configuration.setMaxUser(strToInt(argv[3], a));
  }
  else if(argc == 5)
  {
    int a;
    Configuration.setDBPrefix(argv[1]);
    Configuration.setDbID(argv[2]);
    Configuration.setMaxUser(strToInt(argv[3], a));
    Configuration.setMaxClient(strToInt(argv[4], a));
  }
  else if(argc == 6)
  {
    int a;
    Configuration.setDBPrefix(argv[1]);
    Configuration.setDbID(argv[2]);
    Configuration.setMaxUser(strToInt(argv[3], a));
    Configuration.setMaxClient(strToInt(argv[4], a));
    Configuration.setMaxMrBug(strToInt(argv[5], a));
  }
  else if(argc == 7)
  {
    int a;
    Configuration.setDBPrefix(argv[1]);
    Configuration.setDbID(argv[2]);
    Configuration.setMaxUser(strToInt(argv[3], a));
    Configuration.setMaxClient(strToInt(argv[4], a));
    Configuration.setMaxMrBug(strToInt(argv[5], a));
    PORT_NO = strToInt(argv[6], a);
  }
  else if(argc == 8)
  {
    int a;
    Configuration.setDBPrefix(argv[1]);
    Configuration.setDbID(argv[2]);
    Configuration.setMaxUser(strToInt(argv[3], a));
    Configuration.setMaxClient(strToInt(argv[4], a));
    Configuration.setMaxMrBug(strToInt(argv[5], a));
    PORT_NO = strToInt(argv[6], a);
    Configuration.setGeoRange(strToInt(argv[7], a));
  }
  else
  {
    cout << "USAGE: lc-server [DBPrefix] [Database] [MaxUser] [MaxClient] [MaxMrBug] [PORT_NO] [GEO_RANGE]" << endl;
    return EXIT_FAILURE;
  }
  client_id = Configuration.getDBPrefix();
  __KILLER += "asdadf234";
  Configuration.setPortNumber(PORT_NO);
  Configuration.setCommunicationBufferSize(10240);
  Configuration.init();

  //tworzy tablice do przekazywania parametrów
  __IP = new string [Configuration.getMaxThread() + 2];
  __FD = new int[Configuration.getMaxThread() + 2];
  __PORT = new int[Configuration.getMaxThread() + 2];
  __ID = new long[Configuration.getMaxThread() + 2];

  //resetuje tablice __ID - dla wolnego -1
  for(int i = 0; i < Configuration.getMaxThread() + 2; i++)
  {
    __ID[i] = -1;
  }

  buffer = new char[Configuration.getCommunicationBufferSize()];
  pthread_setconcurrency(Configuration.getMaxThread() + 2);

  pthread_attr_init(&tattr_thread);
  pthread_attr_setdetachstate(&tattr_thread, PTHREAD_CREATE_DETACHED);

  pthread_attr_init(&tattr_thread_stats);
  pthread_attr_setdetachstate(&tattr_thread_stats, PTHREAD_CREATE_DETACHED);

  pthread_attr_init(&tattr_thread_guard);
  pthread_attr_setdetachstate(&tattr_thread_guard, PTHREAD_CREATE_DETACHED);

  pthread_attr_init(&tattr_thread_time_events);
  pthread_attr_setdetachstate(&tattr_thread_time_events, PTHREAD_CREATE_DETACHED);

  string IP = "";
  int ClientPort;
  int IPtmp;
  __KILLER += "234xxcf";
  //inicjalizowannie tablic InRoom
  for(int i = 0; i < Configuration.getMaxUser(); i++)
  {
    Configuration.UserTab[i].InRoom = new long long int [Configuration.getMaxRoom()];
    for(int j = 0; j < Configuration.getMaxUser(); j++)
    {
      Configuration.UserTab[i].InRoom[j] = -1;
    }
  }
  string DBUsername = "";
  string DBDataBase = Configuration.getDbID();
  string DBPassword = "";
  string DBHost = "";
  string DBQuery = "";
  string DBPrefix = Configuration.getDBPrefix();
  string iplocalbans;
  __KILLER += "34wresd";
  //pobieranie lokalnych tablic banów
  MYSQL Database;
  MYSQL_RES *Result;
  MYSQL_ROW Row;
  long res;
  if(!mysql_init(&Database))
  {
    return EXIT_FAILURE;
  }
  else
  {
    if(mysql_real_connect(&Database, DBHost.c_str(), DBUsername.c_str(), DBPassword.c_str(), DBDataBase.c_str(), 0, NULL, 0))
    {
      //pobierz dane
      DBQuery = "SELECT ip FROM " + DBPrefix + "bans";
      res = mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));

      pthread_mutex_lock(&mutex);
      cout << "MYSQL_QUERY #4: " << DBQuery.c_str() << " return " << intToStr(res) << endl;
      pthread_mutex_unlock(&mutex);

      if(res != 0)
      {
        //disconnect from database
        mysql_close(&Database);
        return EXIT_FAILURE;
      }


      Result = mysql_store_result(&Database);
      if(Result)
      {
        while(Row = mysql_fetch_row(Result))
        {
          unsigned long *lengths;
          lengths = mysql_fetch_lengths(Result);
          iplocalbans = Row[0];
          iplocalbans = ":" + iplocalbans + ":";
          IPLocalBans += iplocalbans;
        }
        mysql_free_result(Result);
        mysql_close(&Database);
      }
      else
      {
        //disconnect from database
        mysql_close(&Database);
        return EXIT_FAILURE;
      }
    }
    else
    {
      mysql_close(&Database);
      return EXIT_FAILURE;
    }
  }

  //pobieranie danych domen oraz statystyk (robic statsy/nie)
  __KILLER += "123sxccvkjlp0";
  DBUsername = "root";
  DBDataBase = "eConsultantdb1";
  string Edition = "";
  string AllowSubDomains = "";

  if(!mysql_init(&Database))
  {
    return EXIT_FAILURE;
  }
  else
  {
    if(mysql_real_connect(&Database, DBHost.c_str(), DBUsername.c_str(), DBPassword.c_str(), DBDataBase.c_str(), 0, NULL, 0))
    {
      //pobierz dane
      DBQuery = "SELECT edition,domains,allow_subdomain FROM _ec_clients WHERE db_prefix=\'" + DBPrefix + "\'";
      res = mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));

      pthread_mutex_lock(&mutex);
      cout << "MYSQL_QUERY #5: " << DBQuery.c_str() << " return " << intToStr(res) << endl;
      pthread_mutex_unlock(&mutex);

      if(res != 0)
      {
        //disconnect from database
        mysql_close(&Database);
        return EXIT_FAILURE;
      }

      Result = mysql_store_result(&Database);
      if(Result)
      {
        Row = mysql_fetch_row(Result);

        unsigned long *lengths;
        lengths = mysql_fetch_lengths(Result);
        Edition = Row[0];
        Domains = Row[1];
        AllowSubDomains = Row[2];
        mysql_free_result(Result);
        mysql_close(&Database);
      }
      else
      {
        //disconnect from database
        mysql_close(&Database);
        return EXIT_FAILURE;
      }
    }
    else
    {
      mysql_close(&Database);
      return EXIT_FAILURE;
    }
  }
  Edition = "," + Edition + ",";
  if(Edition.find(",statistics,") != string::npos)// statystyki wlaczone!
  {
    stats_on = true;
  }
  else //statystyki wylaczone!
  {
    stats_on = false;
  }
  if(AllowSubDomains == "1")
  {
    allow_subdomains = true;
  }
  else
  {
    allow_subdomains = false;
  }
  //ustawia domeny
  Domains = "," + Domains;
  string domains_tmp = "";
  for(int i = 0; i < Domains.length(); i++)
  {
    if(Domains[i] != ' ' && Domains[i] != '\n' && Domains[i] && '\t' && Domains[i] != '\r')
    {
      domains_tmp += Domains[i];
    }
  }
  Domains.clear();
  Domains = domains_tmp;

  //pobiera dane do autoinviteow
  DBUsername = "root";
  DBDataBase = Configuration.getDbID();
  string value;
  value.clear();
  //pobieranie lokalnych tablic banów
  if(!mysql_init(&Database))
  {
    return EXIT_FAILURE;
  }
  else
  {
    if(mysql_real_connect(&Database, DBHost.c_str(), DBUsername.c_str(), DBPassword.c_str(), DBDataBase.c_str(), 0, NULL, 0))
    {
      //pobierz dane
      DBQuery = "SELECT value FROM " + Configuration.getDBPrefix() + "config WHERE parm=\'invitation\'";
      res = mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));

      pthread_mutex_lock(&mutex);
      cout << "MYSQL_QUERY #6: " << DBQuery.c_str() << " return " << intToStr(res) << endl;
      pthread_mutex_unlock(&mutex);

      if(res != 0)
      {
        //disconnect from database
        mysql_close(&Database);
        return EXIT_FAILURE;
      }

      Result = mysql_store_result(&Database);
      if(Result)
      {
        Row = mysql_fetch_row(Result);

        unsigned long *lengths;
        lengths = mysql_fetch_lengths(Result);
        value = Row[0];
        InvitationData = value;
        mysql_free_result(Result);
        mysql_close(&Database);
      }
      else
      {
        //disconnect from database
        mysql_close(&Database);
        return EXIT_FAILURE;
      }
    }
    else
    {
      //disconnect from database
      mysql_close(&Database);
      return EXIT_FAILURE;
    }
  }

  //tworzy gniazdo
  sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sockfd < 0)
  {
    return EXIT_FAILURE;
  }
  //ustawianie gniazdka
  int mainsocketoption = 1;
  setsockopt(sockfd, IPPROTO_TCP, O_NDELAY, &mainsocketoption, sizeof (int));
  int flags;
  /* Set socket to non-blocking */
  if((flags = fcntl(sockfd, F_GETFL, 0)) < 0)
  {
    close(sockfd);
    return EXIT_FAILURE;
  }

  if(fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
  {
    close(sockfd);
    return EXIT_FAILURE;
  }

  memset(buffer, NULL, Configuration.getCommunicationBufferSize());

  server_address.sin_family = PF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(Configuration.getPortNumber());
  cout << "Port:" << Configuration.getPortNumber() << endl;

  if(bind(sockfd, (sockaddr *) & server_address, sizeof (server_address)) != 0)
  {
    close(sockfd);
    return EXIT_FAILURE;
  }
  //uruchom watek guarda
  int cpid0 = (int) pthread_create(&t_thread_guard, &tattr_thread_guard, threadGuard, NULL);
  if(cpid0 != 0)
  {
    return EXIT_FAILURE;
  }

  //uruchom watek statystyk
  int cpid1 = (int) pthread_create(&t_thread_stats, &tattr_thread_stats, threadStats, NULL);
  if(cpid1 != 0)
  {
    return EXIT_FAILURE;
  }

  //  uruchom watek zdarzen czasowych
  int cpid2 = (int) pthread_create(&t_thread_time_events, &tattr_thread_time_events, threadTimeEvents, NULL);
  if(cpid2 != 0)
  {
    return EXIT_FAILURE;
  }

  int BufferSize = Configuration.getCommunicationBufferSize();
  listen(sockfd, Configuration.getMaxWait());

  //----------------------------------------------------------------------------
  //---------------START PETLI GLOWNEJ------------------------------------------
  //----------------------------------------------------------------------------
  __KILLER += "adfaerfw3";

  int MaxThread = Configuration.getMaxThread() + 2;

  int argCounter = 0;

  //pobierz SHUTDOWN_STATE
  pthread_mutex_lock(&mutex);
  int tmp_shutdown_state = SHUTDOWN_STATE;
  pthread_mutex_unlock(&mutex);

  ///USTAWIENIE TL
  timespec TL;
  TL.tv_sec = 3; //seconds
  TL.tv_nsec = 0;

  timespec *timeout = &TL;
  ///KONIEC USTAWIANIA TL

  while(tmp_shutdown_state < 2)
  {
    memset(buffer, NULL, BufferSize);
    client_length = sizeof (client_address);

    int nfound = 0;
    fd_set readmask;
    FD_ZERO(&readmask);
    FD_SET(sockfd, &readmask);
    nfound = pselect(sizeof (readmask)*8, &readmask, &readmask, &readmask, timeout, NULL);
    if(nfound > 0)
    {

      newsockfd = accept(sockfd, (sockaddr *) & client_address, (socklen_t*) & client_length);
      if(newsockfd >= 0)
      {
        pthread_mutex_lock(&options_mutex);

        for(int i = 0; i < MaxThread; i++)
        {
          if(__ID[i] == -1)
          {
            __ID[i] = i;
            argCounter = i;
            pthread_mutex_unlock(&options_mutex);
            break;
          }
        }
        if(argCounter >= 0)
        {
          pthread_mutex_unlock(&options_mutex);
          __FD[argCounter] = newsockfd;

          IP = "";
          string tmp;
          IPtmp = (int) client_address.sa_data[2];
          if(IPtmp < 0)
          {
            IPtmp += 256;
          }
          tmp = intToStr(IPtmp);

          IP += tmp;
          IP += ".";
          IPtmp = (int) client_address.sa_data[3];
          if(IPtmp < 0)
          {
            IPtmp += 256;
          }
          tmp = intToStr(IPtmp);

          IP += tmp;
          IP += ".";
          IPtmp = (int) client_address.sa_data[4];
          if(IPtmp < 0)
          {
            IPtmp += 256;
          }
          tmp = intToStr(IPtmp);

          IP += tmp;
          IP += ".";
          IPtmp = (int) client_address.sa_data[5];
          if(IPtmp < 0)
          {
            IPtmp += 256;
          }
          tmp = intToStr(IPtmp);

          IP += tmp;
          __IP[argCounter] = IP;
          ClientPort = (int) (256 * (int) client_address.sa_data[0])+(int) client_address.sa_data[1];
          if(ClientPort < 0)
          {
            ClientPort += 65536;
          }
          __PORT[argCounter] = ClientPort;

          //ustawianie gniazdka
          int cpid = (int) pthread_create(&t_thread, &tattr_thread, threadConnect, (void*) __ID[argCounter]);
          if(cpid != 0)
          {
            close(__FD[argCounter]);
          }
        }
        else
        {
          close(newsockfd);
        }
      }
    }
    pthread_mutex_lock(&mutex);
    int tmp_shutdown_state = SHUTDOWN_STATE;
    pthread_mutex_unlock(&mutex);
    if(tmp_shutdown_state == 2)
    {
      close(sockfd);
      break;
    }
  }

  //odczytaj max ilosc userow guestow i bugow
  pthread_mutex_lock(&mutex);
  int MaxUser = Configuration.getMaxUser();
  int MaxClient = Configuration.getMaxClient();
  int MaxMrBug = Configuration.getMaxMrBug();
  pthread_mutex_unlock(&mutex);

  bool empty = false;
  while(!empty)
  {
    empty = true;
    pthread_mutex_lock(&user_mutex);
    for(int i = 0; i < MaxUser; i++)
    {
      if(Configuration.UserTab[i].getID() >= 0)
      {
        empty = false;
        break;
      }
    }
    pthread_mutex_unlock(&user_mutex);

    pthread_mutex_lock(&mrbug_mutex);
    for(int i = 0; i < MaxMrBug; i++)
    {
      if(Configuration.MrBugTab[i].getID() >= 0 || empty == false)
      {
        empty = false;
        break;
      }
    }
    pthread_mutex_unlock(&mrbug_mutex);

    pthread_mutex_lock(&client_mutex);
    for(int i = 0; i < MaxClient; i++)
    {
      if(Configuration.ClientTab[i].getID() >= 0 || empty == false)
      {
        empty = false;
        break;
      }
    }
    pthread_mutex_unlock(&client_mutex);
    sleep(1);
  }

  delete [] buffer;
  delete [] __IP;
  delete [] __FD;
  delete [] __PORT;
  delete [] __ID;

  return EXIT_SUCCESS;
}
