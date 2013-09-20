#include "thread_time_events.h"


extern pthread_mutex_t mutex;
extern pthread_mutex_t db_mutex;
extern pthread_mutex_t time_mutex;
extern pthread_mutex_t local_bans_mutex;
extern pthread_mutex_t rand_mutex;

extern Settings Configuration; //stad bierzemy m.in. DbID itp.

extern string IPLocalBans;
extern string InvitationData;
extern bool stats_on;
extern bool allow_subdomains;

void* threadTimeEvents(void *arg)
{
  pthread_mutex_lock(&time_mutex);
  pthread_mutex_lock(&rand_mutex);
  srand((int) time(NULL));
  pthread_mutex_unlock(&rand_mutex);
  pthread_mutex_unlock(&time_mutex);

  //spij 240s+rand(60)s po wlaczeniu - dane sa aktualne, wiec nie ma sensu walczyc
  //losuj czas sleepa z przedzialu 240 a 300 s
  pthread_mutex_lock(&rand_mutex);
  int TTL = 240 + rand() % 60; //TTL ==  TimeToSleep
  pthread_mutex_unlock(&rand_mutex);
  sleep(TTL);

  MYSQL Database;
  MYSQL_RES *Result;
  MYSQL_ROW Row;
  long res;
  while(1)
  {
    //dane dot. polaczen z baza danych
    pthread_mutex_lock(&mutex);
    string DBUsername = "";
    string DBDataBase = Configuration.getDbID();
    string DBPassword = "";
    string DBHost = "";
    string DBQuery = "";
    string DBPrefix = Configuration.getDBPrefix();
    pthread_mutex_unlock(&mutex);
    //zmienne lokalne - przypisz dane ze starych zmiennych, na wypadek niemożliwości polaczenia z baza daych
    pthread_mutex_lock(&mutex);
    string InvData = InvitationData;
    bool StatsON = stats_on;
    bool AllowSubs = allow_subdomains;
    pthread_mutex_unlock(&mutex);

    pthread_mutex_lock(&local_bans_mutex);
    string IPLB = IPLocalBans;
    pthread_mutex_unlock(&local_bans_mutex);

    //pobierz aktualne dane z bazy
    bool dbConnectSuccess = false;
    pthread_mutex_lock(&db_mutex);
    if(!mysql_init(&Database))
    {
      dbConnectSuccess = false;
    }
    else
    {
      if(mysql_real_connect(&Database, DBHost.c_str(), DBUsername.c_str(), DBPassword.c_str(), DBDataBase.c_str(), 0, NULL, 0))
      {
        //pobierz swieza liste banów z bazy danych
        DBQuery = "SELECT ip FROM " + DBPrefix + "bans";
        res = mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));
        pthread_mutex_lock(&mutex);
        cout << "MYSQL_QUERY #17: " << DBQuery.c_str() << " return " << intToStr(res) << endl;
        pthread_mutex_unlock(&mutex);
        if(res != 0)
        {
          //disconnect from database
          mysql_close(&Database);
          dbConnectSuccess = false;
        }
        else
        {
          dbConnectSuccess = true;
        }

        if(dbConnectSuccess)
        {
          IPLB.clear();
          Result = mysql_store_result(&Database);
          if(Result)
          {
            while(Row = mysql_fetch_row(Result))
            {
              unsigned long *lengths;
              lengths = mysql_fetch_lengths(Result);
              string iplocalbans = Row[0];
              iplocalbans = ":" + iplocalbans + ":";
              IPLB += iplocalbans;
            }
            mysql_free_result(Result);
            dbConnectSuccess = true;
          }
          else
          {
            //disconnect from database
            mysql_close(&Database);
            dbConnectSuccess = false;
          }
          //pobierz swieza liste inviteData z serwera
          if(dbConnectSuccess)
          {
            //pobierz dane
            DBQuery = "SELECT value FROM " + Configuration.getDBPrefix() + "config WHERE parm=\'invitation\'";
            res = mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));
            pthread_mutex_lock(&mutex);
            cout << "MYSQL_QUERY #18: " << DBQuery.c_str() << " return " << intToStr(res) << endl;
            pthread_mutex_unlock(&mutex);
            if(res != 0)
            {
              //disconnect from database
              mysql_close(&Database);
              dbConnectSuccess = false;
            }
            else
            {
              Result = mysql_store_result(&Database);
              InvData.clear();
              if(Result)
              {
                Row = mysql_fetch_row(Result);

                unsigned long *lengths;
                lengths = mysql_fetch_lengths(Result);
                string value = Row[0];
                InvData = value;
                mysql_free_result(Result);
                mysql_close(&Database);
                dbConnectSuccess = true;
              }
              else
              {
                //disconnect from database
                mysql_close(&Database);
                dbConnectSuccess = false;
              }
            }
          }
        }
      }
      else
      {
        dbConnectSuccess = false;
        mysql_close(&Database);
      }
    }
    //pobierz dane z glownej bazy
    if(dbConnectSuccess)
    {
      DBDataBase = "eConsultantdb1"; //teraz ma brac dane z bazy glownej, gdzie sa wpisani klienci

      if(!mysql_init(&Database))
      {
        dbConnectSuccess = false;
      }
      else
      {
        if(mysql_real_connect(&Database, DBHost.c_str(), DBUsername.c_str(), DBPassword.c_str(), DBDataBase.c_str(), 0, NULL, 0))
        {
          //pobierz dane
          DBQuery = "SELECT edition,allow_subdomain FROM _ec_clients WHERE db_prefix=\'" + DBPrefix + "\'";
          res = mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));
          pthread_mutex_lock(&mutex);
          cout << "MYSQL_QUERY #19: " << DBQuery.c_str() << " return " << intToStr(res) << endl;
          pthread_mutex_unlock(&mutex);
          if(res != 0)
          {
            //disconnect from database
            mysql_close(&Database);
            dbConnectSuccess = false;
          }
          else
          {
            Result = mysql_store_result(&Database);
            if(Result)
            {
              Row = mysql_fetch_row(Result);

              unsigned long *lengths;
              lengths = mysql_fetch_lengths(Result);
              string SStatsON = Row[0];
              string SAllowSubs = Row[1];

              if(SStatsON.find("statistics") != string::npos)
              {
                StatsON = true;
              }
              else
              {
                StatsON = false;
              }
              if(SAllowSubs == "1")
              {
                AllowSubs = true;
              }
              else
              {
                AllowSubs = false;
              }

              mysql_free_result(Result);
              dbConnectSuccess = true;
              mysql_close(&Database);
            }
            else
            {
              //disconnect from database
              mysql_close(&Database);
              dbConnectSuccess = false;
            }
          }
        }
        else
        {
          mysql_close(&Database);
          dbConnectSuccess = false;
        }
      }
    }
    pthread_mutex_unlock(&db_mutex);

    if(dbConnectSuccess) //udalo sie polaczyc z baza
    {
      //nadpisz aktualnymi danymi
      pthread_mutex_lock(&mutex);
      InvitationData = InvData;
      stats_on = StatsON;
      allow_subdomains = AllowSubs;
      pthread_mutex_unlock(&mutex);

      pthread_mutex_lock(&local_bans_mutex);
      IPLocalBans = IPLB;
      pthread_mutex_unlock(&local_bans_mutex);

      //losuj czas sleepa z przedzialu 240 a 300 s
      pthread_mutex_lock(&rand_mutex);
      TTL = 240 + rand() % 60; //TTL ==  TimeToSleep
      pthread_mutex_unlock(&rand_mutex);
      sleep(TTL);
    }
    else //nie udalo sie polaczyz z baza
    {
      sleep(5); //sproboj ponownie w przeciagu 5 s
    }
  }
  pthread_exit(0);
}
