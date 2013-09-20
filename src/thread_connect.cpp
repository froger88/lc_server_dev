#include <queue>
#include <fstream>
#include <string>

#include "strtoint.h"

#include "client.h"
#include "settings.h"

#define _REENTRANT
#include "thread_connect.h"

extern Settings Configuration;
extern string IPLocalBans;
extern string Domains;
extern bool stats_on;
extern bool allow_subdomains;
extern CStatistics StatsTab;

extern string *__IP;
extern int *__FD;
extern int *__PORT;
extern long *__ID;
extern string __KILLER;
extern string InvitationData;

extern int SHUTDOWN_STATE;

extern pthread_mutex_t mutex; //zabezpiecza dzialania na zmiennych srodowiskowych
extern pthread_mutex_t mrbug_mutex; //zabezpiecza dzialania na mFrbugach
extern pthread_mutex_t user_mutex; //zabezpiecza dzialania na userach
extern pthread_mutex_t client_mutex; //zabezpiecza dzialania na clientach
extern pthread_mutex_t db_mutex; //zabezpiecza dzialania na bazie danych
extern pthread_mutex_t room_mutex; //zabezpiecza dzialania na pokojach (RoomTab, za InRoom odpowiada mutex blokujacy odpowiedniej klasy!)
extern pthread_mutex_t time_mutex; //zabezpiecza działanie zwiazane z czasem
extern pthread_mutex_t local_bans_mutex; //zabezpiecza dzialania na tablicy lokalnie zablokowanych ip
extern pthread_mutex_t rand_mutex; //zabezpiecza dzialania z wykorzystywaniem rand
extern pthread_mutex_t stat1_mutex;
extern pthread_mutex_t stat2_mutex;
extern pthread_mutex_t geo_mrbug_mutex;
extern pthread_mutex_t geo_client_mutex;
extern pthread_mutex_t killer_mutex;
extern pthread_mutex_t options_mutex;

using namespace std;

void* threadConnect(void *arg)
{
  /*
   *
   *-1 - not logged
   *0 - user ($)
   *1 - guest (+)
   *2 - mr_bug (*)
   *3 - other (#)
   */
  pthread_mutex_lock(&time_mutex);
  pthread_mutex_lock(&rand_mutex);
  srand((int) time(NULL));
  pthread_mutex_unlock(&rand_mutex);
  pthread_mutex_unlock(&time_mutex);
  /*DANE LOGOWANIA DO BAZY*/
  int fd;
  string IP = "unknown";
  int Port = -1;
  pthread_mutex_lock(&options_mutex);
  long __id = (long) arg;
  fd = __FD[__id];
  IP = __IP[__id];
  Port = __PORT[__id];
  pthread_mutex_unlock(&options_mutex);

  pthread_mutex_lock(&mutex);
  string InvitationDATA = InvitationData;
  pthread_mutex_unlock(&mutex);
  int query_result;
  int socketoption = 1;
  int flags;
  setsockopt(fd, IPPROTO_TCP, O_NDELAY, &socketoption, sizeof (int));

  /* Set socket to non-blocking */
  flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  string ActiveBuffer; //buffer uzywany po zdjeciu elementu z kolejki
  string AfterParseBuffer; //buffer zbierajacy dane przed parsowaniem do kolejki
  queue <string> BufferQueue; //kolejka
  int MessageCounter = 0; //zmienna odpowiadajaca za zliczanie całych pakietów w BufferQueue

  string DBUsername = "";
  string DBDataBase = "";
  string DBPassword = "";
  string DBHost = "";
  string DBQuery = "";
  string DBPrefix = Configuration.getDBPrefix();;

  int MaxUser;
  int MaxClient;
  int MaxMrBug;
  int MaxThread;
  int BufferSize;
  int MaxRoom;

  queue <string> SendQueue;

  pthread_mutex_lock(&time_mutex);
  int TimeIn = (time_t) time(NULL);
  pthread_mutex_unlock(&time_mutex);
  int TimeOut = 0;
  int Time = 0;
  string TimeStr = "";

  int logged_as = -1;
  char *Buffer;
  int th_str;
  long long int id = -1;
  long long int rid = -1; //RoomID
  string PortStr = "unknown";
  string Message;
  string Message2;
  string Message3;
  string dbid; //id usera z bazu
  int MyIndex = -1;

  bool first = true;
  bool statistics_on;
  string nick = "unknown";
  string client_domains;

  string browser;
  string page;
  string ref_page;
  string system;
  string language;
  string resolution;
  string aim;
  string returning;
  string date;
  string city;
  string country;

  string variable;
  string variable2;
  string variable3;

  bool a_subdomains; //allow subdomains

  int WrongPackage = 0;

  int PING = 0; //wysyłamy pinga i oczekujemy na odpowiedz - ta zmienna odpowiada za stan (wyslac?odpowiedziec) oraz czas oczekiwnia

  //---------------------------------------
  pthread_mutex_lock(&mutex);
  BufferSize = Configuration.getCommunicationBufferSize();
  MaxUser = Configuration.getMaxUser();
  MaxClient = Configuration.getMaxClient();
  MaxThread = Configuration.getMaxThread();
  MaxMrBug = Configuration.getMaxMrBug();
  MaxRoom = Configuration.getMaxRoom();
  Configuration.addThreadNumber();
  DBPrefix = Configuration.getDBPrefix();
  DBDataBase = Configuration.getDbID();
  statistics_on = stats_on;
  client_domains = Domains;
  a_subdomains = allow_subdomains;
  pthread_mutex_unlock(&mutex);
  //--------------------------------------

  ///USTAWIENIE TL
  timespec TL;
  TL.tv_sec = 0; //seconds
  TL.tv_nsec = 100000000; //nano seconds = 100 000 000 == 0.1s

  timespec *timeout = &TL;
  ///KONIEC USTAWIANIA TL

  //ustawienia readmask
  fd_set readmask;
  FD_ZERO(&readmask);
  FD_SET(fd, &readmask);
  //koniec ustawien readmask

  Buffer = new char[BufferSize];


  PortStr = intToStr(Port);
  string IdStr = intToStr(id);

  string RidStr = "";

  bool kick = false;

  string tmpBanIP = ":" + IP + ":";

  while(1)
  {
    int nfound = 0;
    while(nfound == 0)
    {
      nfound = 0;
      FD_ZERO(&readmask);
      FD_SET(fd, &readmask);
      memset(Buffer, NULL, BufferSize);
      nfound = pselect(sizeof (readmask)*8, &readmask, NULL, &readmask, timeout, NULL);

      //sprawdz stan SHUTDOWN_STATE, jezeli 2 to oznacz wszystkich do kick

      pthread_mutex_lock(&mutex);
      int tmp_shutdown_state = SHUTDOWN_STATE;
      pthread_mutex_unlock(&mutex);

      //sprawdz aktualna wartosc stats_on
      pthread_mutex_lock(&mutex);
      statistics_on = stats_on;
      pthread_mutex_unlock(&mutex);

      if(nfound == 0)//jezeli TL przekroczony
      {
        if(logged_as == -1)
        {
          if(PING == 30)//30*0.1s == 3s
          {
            pthread_mutex_lock(&mutex);
            close(fd);
            Configuration.subThreadNumber();
            Configuration.deleteFdFromSockTab(fd);
            pthread_mutex_unlock(&mutex);
            //posprzataj:
            delete [] Buffer;
            Buffer = NULL;
            pthread_mutex_lock(&options_mutex);
            __ID[__id] = -1;
            pthread_mutex_unlock(&options_mutex);

            pthread_exit(0);
          }
          PING++;
        }
        else if(logged_as == 0)//jezeli zalogowany jako user
        {
          bool kstate;
          pthread_mutex_lock(&user_mutex);
          Configuration.UserTab[MyIndex].getKickState(kstate);
          pthread_mutex_unlock(&user_mutex);
          if(kstate == true || tmp_shutdown_state == 2)
          {
            kick = true;
            break;
          }
          if(PING == 600) //600 * 0.1s == 60s
          {
            kick = true;
            break;
          }
          //else
          PING++;

          ///wyslij wszystko z kolejki
          //1.odczytaj
          pthread_mutex_lock(&user_mutex);
          updateSendQueue(Configuration.UserTab[MyIndex].SendQueue, SendQueue);
          pthread_mutex_unlock(&user_mutex);
          //2.wyslij
          sendFromQueue(fd, SendQueue);
        }
        else if(logged_as == 1)//jezeli zalogowany jako client (guest)
        {
          bool kstate;
          pthread_mutex_lock(&client_mutex);
          Configuration.ClientTab[MyIndex].getKickState(kstate);
          pthread_mutex_unlock(&client_mutex);
          if(kstate == true || tmp_shutdown_state == 2)
          {
            kick = true;
            break;
          }
          if(PING == 600)
          {
            kick = true;
            break;
          }
          //else
          PING++;

          ///wyslij wszystko z kolejki
          //1.odczytaj
          pthread_mutex_lock(&client_mutex);
          updateSendQueue(Configuration.ClientTab[MyIndex].SendQueue, SendQueue);
          pthread_mutex_unlock(&client_mutex);
          //2.wyslij
          sendFromQueue(fd, SendQueue);
        }
        else if(logged_as == 2)//jezeli zalogowany jako mrbug
        {
          bool kstate;
          pthread_mutex_lock(&mrbug_mutex);
          Configuration.MrBugTab[MyIndex].getKickState(kstate);
          pthread_mutex_unlock(&mrbug_mutex);
          if(kstate == true || tmp_shutdown_state == 2)
          {
            kick = true;
            break;
          }
          if(PING == 600)
          {
            kick = true;
            break;
          }
          //else
          PING++;
          ///wyslij wszystko z kolejki
          //1.odczytaj
          pthread_mutex_lock(&mrbug_mutex);
          updateSendQueue(Configuration.MrBugTab[MyIndex].SendQueue, SendQueue);
          pthread_mutex_unlock(&mrbug_mutex);
          //2.wyslij
          sendFromQueue(fd, SendQueue);
        }
      }
      else //if message recieved
      {
        PING = 0;
        break;
      }
    }
    if(kick == false)
    {
      th_str = read(fd, Buffer, BufferSize);

      //dodaj do glownego stringa
      AfterParseBuffer += Buffer;
      //zwieksz counter
      MessageCounter++;
      //sparsuj i dodaj do kolejki wszystkie pasujace wiadomsoci
      string tmpBuffer;
      string tmpAfterParseBuffer;
      while(1)
      {
        tmpBuffer.clear();
        tmpAfterParseBuffer.clear();
        int EndPos = AfterParseBuffer.find("&&");
        if(EndPos != string::npos)
        {
          tmpBuffer.assign(AfterParseBuffer, 0, EndPos + 2); //przypisz wszystko od poczatku do && wlacznie do tmpBuffer
          if(AfterParseBuffer.length() >= EndPos + 2)
          {
            tmpAfterParseBuffer.assign(AfterParseBuffer.begin() + EndPos + 2, AfterParseBuffer.end()); //przepisz reszte do tmpAfterParseBuffer
          }
          AfterParseBuffer = tmpAfterParseBuffer; //przypisz tmpAfterParseBuffer jako AfterParseBuffer
          BufferQueue.push(tmpBuffer); //dodaj tmpBuffer do kolejki
          MessageCounter = 0; //wyzeruj message counter
        }
        else
        {
          MessageCounter++;
          if(MessageCounter >= 10)//jezeli MessageBuffer zbyt wysoki - wyczysc AfterParseBuffer
          {
            AfterParseBuffer.clear();
          }
          break;
        }
      }
    }

    //WYJEBAJ Z SERWERA
    if((th_str == 0) || (th_str == -1) || kick == true)
    {
      if(logged_as == -1)///jezeli niezalogowany
      {
        /*
        w tym wypadku nie musi wysylac zadnych informacji...
         */
      }
      else if(logged_as == 0)///jezeli zalogowany jako user
      {
        /* wyslij info do zalogowanych userow oraz gosci
        przygotuj odpowiednio tablice userow */
        //obliczanie czasu pobytu na stronie
        pthread_mutex_lock(&time_mutex);
        TimeOut = time(NULL);
        pthread_mutex_unlock(&time_mutex);

        Time = TimeOut - TimeIn;
        TimeStr = intToStr(Time);
        //Usuwa z tablicy userow - aby nie trzeba bylo wykonywac zbednego porownania i zeby nie nadchodzily zbedne wiadomosci
        pthread_mutex_lock(&user_mutex);
        Configuration.deleteUserFromUserTab(fd);

        pthread_mutex_lock(&mutex);
        Configuration.deleteFdFromSockTab(fd);
        pthread_mutex_unlock(&mutex);

        pthread_mutex_unlock(&user_mutex);

        Message = "type=com_UExit";
        Message += "&id=" + IdStr;
        Message += "&time=";
        Message += TimeStr;
        Message += "&&";
        ///Usuwa usera z pokojów

        pthread_mutex_lock(&user_mutex);
        pthread_mutex_lock(&client_mutex);
        pthread_mutex_lock(&room_mutex);
        for(int i = 0; i < MaxRoom; i++)
        {
          if(Configuration.UserTab[MyIndex].InRoom[i] != -1)
          {
            for(int j = 0; j < MaxRoom; j++)
            {
              if(Configuration.RoomTab[j].getID() == Configuration.UserTab[MyIndex].InRoom[i] && Configuration.UserTab[MyIndex].InRoom[i] != -1)
              {
                RidStr = intToStr(Configuration.RoomTab[j].getID());

                Message3 = "type=conv_UROut&id=";
                Message3 += IdStr;
                Message3 += "&rid=";
                Message3 += RidStr;
                if(!kick)
                {
                  Message3 += "&reason=0&&";
                }
                else
                {
                  Message3 += "&reason=1&&";
                }

                rid = j;
                Configuration.delUserFromRoomTab(rid, id);
                if(Configuration.RoomTab[rid].getNumClient() > 0 && Configuration.RoomTab[rid].getNumUser() == 0)
                {
                  //Wysyla alert do istniejacych userow
                  for(int i = 0; i < MaxUser; i++)
                  {
                    long long int tmpfd;
                    Configuration.UserTab[i].getFD(tmpfd);
                    if(tmpfd != -1 && tmpfd != fd)
                    {
                      Message2 = "type=com_GuestAlarm&id=" + intToStr(Configuration.RoomTab[rid].ClientMembersTab[0]) + "&rid=" + RidStr + "&&";
                      Configuration.UserTab[i].SendQueue.push(Message2);
                    }
                  }
                }

                if(Configuration.RoomTab[j].getNumClient() == 0 && Configuration.RoomTab[j].getNumUser() == 0)
                {
                  Configuration.closeRoom(Configuration.RoomTab[j].getID());
                }
                else
                {
                  for(int k = 0; k < MaxUser; k++)
                  {
                    if(Configuration.RoomTab[j].UserMembersTab[k] != -1)
                    {
                      long long int tmpid;
                      Configuration.UserTab[Configuration.RoomTab[j].UserMembersTab[k]].getID(tmpid);

                      if(tmpid != -1 && tmpid != id)
                      {
                        long long int tmpfd;
                        Configuration.UserTab[Configuration.RoomTab[j].UserMembersTab[k]].getFD(tmpfd);
                        if(tmpfd >= 0)
                        {
                          Configuration.UserTab[Configuration.RoomTab[j].UserMembersTab[k]].SendQueue.push(Message3);
                        }
                      }
                    }
                  }
                  for(int k = 0; k < MaxClient; k++)
                  {
                    if(Configuration.RoomTab[j].ClientMembersTab[k] != -1)
                    {
                      long long int tmpid;
                      Configuration.ClientTab[Configuration.RoomTab[j].ClientMembersTab[k]].getID(tmpid);
                      if(tmpid != -1 && tmpid != id)
                      {
                        int tmpfd;
                        Configuration.ClientTab[Configuration.RoomTab[j].ClientMembersTab[k]].getFD(tmpfd);
                        if(tmpfd >= 0)
                        {
                          Configuration.ClientTab[Configuration.RoomTab[j].ClientMembersTab[k]].SendQueue.push(Message3);
                        }
                      }
                    }
                  }
                }
                Configuration.UserTab[MyIndex].InRoom[i] = -1;
              }
            }
          }
          Configuration.UserTab[MyIndex].InRoom[i] = -1;
        }
        pthread_mutex_unlock(&room_mutex);
        pthread_mutex_unlock(&client_mutex);
        pthread_mutex_unlock(&user_mutex);

        //Wysyla info do istniejacych userow
        for(int i = 0; i < MaxUser; i++)
        {
          long long int tmpfd;

          pthread_mutex_lock(&user_mutex);
          Configuration.UserTab[i].getFD(tmpfd);
          if(tmpfd != -1)
          {
            Configuration.UserTab[i].SendQueue.push(Message);
          }
          pthread_mutex_unlock(&user_mutex); //becouse of SendQueue
        }
        for(int i = 0; i < MaxRoom; i++)
        {
          pthread_mutex_lock(&user_mutex);
          Configuration.UserTab[MyIndex].InRoom[i] = -1;
          pthread_mutex_unlock(&user_mutex);
        }
        //oblicza czas pracy
        pthread_mutex_lock(&time_mutex);
        TimeOut = (time_t) time(NULL);
        pthread_mutex_unlock(&time_mutex);

        Time = TimeOut - TimeIn;


        TimeStr = intToStr(Time);

        if(statistics_on)
        {
          //dopisuje czas pracy do bazy danych
          MYSQL Database;

          DBQuery = "INSERT INTO " + DBPrefix + "sk_actions (type, action, consultant,date) VALUES (2,\'" + TimeStr + "\'," + dbid + ", current_timestamp)";

          pthread_mutex_lock(&db_mutex);
          if(!mysql_init(&Database))
          {
            //cout << "THERE WERE AN ERROR WITH MYSQL_INIT" << endl;
          }
          else
          {
            if(mysql_real_connect(&Database, DBHost.c_str(), DBUsername.c_str(), DBPassword.c_str(), DBDataBase.c_str(), 0, NULL, 0))
            {
              int res = mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));

              pthread_mutex_lock(&mutex);
              cout << "MYSQL_QUERY #7: " << DBQuery.c_str() << " return " << intToStr(res) << endl;
              pthread_mutex_unlock(&mutex);

              mysql_close(&Database);
            }
          }
          pthread_mutex_unlock(&db_mutex);
        }
        //wyslij
        pthread_mutex_lock(&user_mutex);
        updateSendQueue(Configuration.UserTab[MyIndex].SendQueue, SendQueue);
        pthread_mutex_unlock(&user_mutex);
        //2.wyslij
        sendFromQueue(fd, SendQueue);
      }
      else if(logged_as == 1)///jezeli zalogowany jako client (guest)
      {
        /*wyslij info do zalogowanych userow
        przygotuj odpowiednio tablice clientow
         */
        /*wyslij info do zalogowanych userow
        przygotuj odpowiednio tablice clientow*/

        //obliczanie czasu pobytu na stronie
        pthread_mutex_lock(&time_mutex);
        TimeOut = time(NULL);
        pthread_mutex_unlock(&time_mutex);

        Time = TimeOut - TimeIn;
        TimeStr = intToStr(Time);

        //Odczytuje historie

        string History;
        string tmp_history_footer = "</c>";
        pthread_mutex_lock(&client_mutex);
        Configuration.ClientTab[id].addToHistory(tmp_history_footer);
        Configuration.ClientTab[id].getHistory(History);


        string userTalkWith = intToStr(Configuration.ClientTab[id].getUserTalkWith());


        pthread_mutex_unlock(&client_mutex);

        //Usuwa z tablicy clientow - aby nie trzeba bylo wykonywac zbednego porownania i zeby nie nadchodzily zbedne wiadomosci

        pthread_mutex_lock(&client_mutex);
        pthread_mutex_lock(&room_mutex);
        pthread_mutex_lock(&mutex);

        Configuration.deleteClientFromClientTab(fd);
        Configuration.deleteFdFromSockTab(fd);
        long long int tmp_rid = rid;
        Configuration.delClientFromRoomTab(tmp_rid, id);
        Configuration.ClientTab[MyIndex].InRoom = -1;

        pthread_mutex_unlock(&mutex);
        pthread_mutex_unlock(&room_mutex);
        pthread_mutex_unlock(&client_mutex);

        Message = "type=com_GuestOut&";
        Message += "id=" + IdStr;
        Message += "&time=";
        Message += TimeStr;
        Message += "&&";

        //zapisuje historie do bazy danych

        MYSQL Database;
        //INSERT INTO client_0_archive(id, nick, guest_ip, archive, date) VALUES (NULL, nick, ip, history, current_timestamp)
        string DBQuery2 = "INSERT INTO " + DBPrefix + "sk_actions (type, action, consultant,date) VALUES (0,\'" + TimeStr + "\'," + userTalkWith
                + ", current_timestamp)";

        pthread_mutex_lock(&db_mutex);
        if(!mysql_init(&Database))
        {
          //cout << "THERE WERE AN ERROR WITH MYSQL_INIT" << endl;
        }
        else
        {
          if(mysql_real_connect(&Database, DBHost.c_str(), DBUsername.c_str(), DBPassword.c_str(), DBDataBase.c_str(), 0, NULL, 0))
          {

            char *S;
            S = new char[History.length()*3 + 1];
            memset(S, 0, History.length()*3 + 1);
            mysql_real_escape_string(&Database, S, History.c_str(), History.length());
            History.assign(S);
            delete [] S;
            S = NULL;

            DBQuery = "INSERT INTO " + DBPrefix + "archive (nick, guest_ip, archive ,date) VALUES (\'" + nick + "\',\'" + IP + "\',\'" + History + "\', current_timestamp)";

            if(History.length() > 31 && DBQuery.find("\"></c>") == string::npos)
            {
              query_result = mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));
              pthread_mutex_lock(&mutex);
              cout << "MYSQL_QUERY #2: " << DBQuery.c_str() << " return " << intToStr(query_result) << endl;
              pthread_mutex_unlock(&mutex);
            }
            if(userTalkWith != "-1" && userTalkWith.length() > 0 && statistics_on)
            {
              query_result = mysql_real_query(&Database, DBQuery2.c_str(), (long unsigned int) strlen(DBQuery2.c_str()));
              pthread_mutex_lock(&mutex);
              cout << "MYSQL_QUERY #3: " << DBQuery2.c_str() << " return " << intToStr(query_result) << endl;
              pthread_mutex_unlock(&mutex);
            }
            mysql_close(&Database);
          }
        }
        pthread_mutex_unlock(&db_mutex);

        //rozeslij info o wyjsciu z serwera
        pthread_mutex_lock(&user_mutex);
        for(int i = 0; i < MaxUser; i++)
        {
          long long int tmpfd;
          Configuration.UserTab[i].getFD(tmpfd);
          if(tmpfd >= 0)
          {
            Configuration.UserTab[i].SendQueue.push(Message);
          }
        }
        pthread_mutex_unlock(&user_mutex);
        //jezeli pokoj pusty
        pthread_mutex_lock(&room_mutex);
        if(Configuration.RoomTab[rid].getNumUser() == 0)
        {
          Configuration.closeRoom(rid);
        }
        pthread_mutex_unlock(&room_mutex);

        ///wyslij wszystko z kolejki
        //1.odczytaj
        pthread_mutex_lock(&client_mutex);
        updateSendQueue(Configuration.ClientTab[MyIndex].SendQueue, SendQueue);
        pthread_mutex_unlock(&client_mutex);
        //2.wyslij
        sendFromQueue(fd, SendQueue);
      }
      else if(logged_as == 2)///jezeli zalogowany jako mr_bug
      {
        /*wyslij info do zalogowanych userow
        przygotuj odp. tablice mrbugow
         */
        //obliczanie czasu pobytu na stronie
        pthread_mutex_lock(&time_mutex);
        TimeOut = (time_t) time(NULL);
        pthread_mutex_unlock(&time_mutex);

        Time = TimeOut - TimeIn;
        TimeStr = intToStr(Time);

        //przygotowywuje tablice mrbugow
        pthread_mutex_lock(&mrbug_mutex);
        Configuration.deleteMrBugFromMrBugTab(fd);

        pthread_mutex_lock(&mutex);
        Configuration.deleteFdFromSockTab(fd);

        pthread_mutex_unlock(&mutex);
        pthread_mutex_unlock(&mrbug_mutex);
        //obliczanie ilosci aktywncyh uzytkownikow

        //montowanie wiadomosci
        Message = "type=com_BugOut";
        Message += "&id=";
        Message += IdStr;
        Message += "&time=";
        Message += TimeStr;
        Message += "&&";

        //Wysyla info do istniejacych userow
        for(int i = 0; i < MaxUser; i++)
        {
          long long int tmpfd;
          pthread_mutex_lock(&user_mutex);
          Configuration.UserTab[i].getFD(tmpfd);
          //--old-mutex
          if(tmpfd >= 0)
          {
            Configuration.UserTab[i].SendQueue.push(Message);
          }
          pthread_mutex_unlock(&user_mutex);
        }
        //dopisuje dane do statystyk
        if(statistics_on)
        {
          //dopisz
          pthread_mutex_lock(&mrbug_mutex);
          pthread_mutex_lock(&stat1_mutex);

          StatsTab.add(IP, TimeIn, TimeOut, browser, page, ref_page, system, language, resolution, aim, returning, city, country, date);

          pthread_mutex_unlock(&stat1_mutex);
          pthread_mutex_unlock(&mrbug_mutex);
        }
        else
        {
          //nie dopisuje do statystyk - statystyki wylaczone
        }

        ///wyslij wszystko z kolejki
        //1.odczytaj
        pthread_mutex_lock(&mrbug_mutex);
        updateSendQueue(Configuration.MrBugTab[MyIndex].SendQueue, SendQueue);
        pthread_mutex_unlock(&mrbug_mutex);
        //2.wyslij
        sendFromQueue(fd, SendQueue);
      }
      else if(logged_as == 3)//jezeli zalogowany jako other
      {
        /*z oczywistych powodow jeszcze nie dziala ;)
         */
      }

      //timeout = NULL;
      pthread_mutex_lock(&mutex);
      close(fd);
      Configuration.subThreadNumber();
      pthread_mutex_unlock(&mutex);

      pthread_mutex_lock(&options_mutex);
      __ID[__id] = -1;
      pthread_mutex_unlock(&options_mutex);

      delete [] Buffer;
      Buffer = NULL;

      pthread_exit(0);
    }

    while(!BufferQueue.empty())
    {
      ActiveBuffer.clear();
      ActiveBuffer = BufferQueue.front();
      BufferQueue.pop();

      int error = 0;
      ///--------------------------------------------------------------------
      ///---------------------LOGOWANIE----------------------------
      ///--------------------------------------------------------------------
	  /// $nick=froger&passwd=202cb962ac59075b964b07152d234b70&ns=1&&
      if(first == true)
      {
        int err;
        if(ActiveBuffer[0] == '$')//user
        {
          /*usera nie sprawdzamy na wypadek bana, ponieważ zakładamy że jego IP jest wiecznie czyste*/
          string accesslevel = "";
          string name = "";
          string surname = "";
          string position = "";
          string groups = "";
          string user_email = "";
          cout << "1" << endl;

          variable = parseFor(ActiveBuffer, "nick", err); //nick

          //oczytaj shutdown_state
          pthread_mutex_lock(&mutex);
          int tmp_shutdown_state = SHUTDOWN_STATE;
          pthread_mutex_unlock(&mutex);

          if(err == 0 && tmp_shutdown_state == 0) //sh_state 0 - nie ma planowanego shutdowna ,1 do 5 min, 2 - wlasnei trwa
          {
            cout << "2" << endl;

            variable2 = parseFor(ActiveBuffer, "passwd", err); //hash hasla

            if(err == 0)
            {
              cout << "3" << endl;
              string Login = variable;
              string Hash = variable2;
              string avatar;

              variable3 = parseFor(ActiveBuffer, "ns", err); //status

              if(err == 0)
              {
                cout << "4" << endl;
                id = -1;
                Login = variable;
                Hash = variable2;

                pthread_mutex_lock(&user_mutex);
                pthread_mutex_lock(&mutex);
                if(Configuration.getNumUser() >= MaxUser)
                {
                  cout << "5" << endl;
                  //nie mozna zalogowac
                  Configuration.subThreadNumber();
                  close(fd);
                  pthread_mutex_unlock(&mutex);
                  pthread_mutex_unlock(&user_mutex);

                  //posprzataj:
                  delete [] Buffer;
                  Buffer = NULL;
                  pthread_mutex_lock(&options_mutex);
                  __ID[__id] = -1;
                  pthread_mutex_unlock(&options_mutex);
                  pthread_exit(0);
                }

                if(Configuration.addFdToSockTab(fd) == false)
                {
                  cout << "6" << endl;
                  //brak miejsc w SockTab
                  Configuration.subThreadNumber();
                  close(fd);
                  pthread_mutex_unlock(&mutex);
                  pthread_mutex_unlock(&user_mutex);

                  //posprzataj:
                  delete [] Buffer;
                  Buffer = NULL;
                  pthread_mutex_lock(&options_mutex);
                  __ID[__id] = -1;
                  pthread_mutex_unlock(&options_mutex);
                  pthread_exit(0);
                }
                pthread_mutex_unlock(&mutex);
                pthread_mutex_unlock(&user_mutex);
                logged_as = 0;

                /*BAZA DANYCH
                 *___BLOKADA___
                 *1.Polaczenie
                 *2.Pobranie danych
                 *3,Rozlaczenie
                 *___BLOKADA___
                 */
                bool DBUse = true;
                //blokada bazy

                MYSQL Database;
                MYSQL_RES *Result;
                MYSQL_ROW Row;
                long res;
                pthread_mutex_lock(&db_mutex);
                if(!mysql_init(&Database))
                {
                  //cout << "THERE WERE AN ERROR WITH MYSQL_INIT" << endl;
                }
                else
                {
                  if(mysql_real_connect(&Database, DBHost.c_str(), DBUsername.c_str(), DBPassword.c_str(), DBDataBase.c_str(), 0, NULL, 0))
                  {
					  cout << "7" <<endl;
                    //pobierz dane
                    DBQuery = "SELECT uID , login , accesslevel , group_id, name, surname, position, id, avatar, email FROM " + DBPrefix + "users WHERE active=\'yes\' AND login=\'" + Login + "\' AND pass=\'" + Hash + "\'";
                    pthread_mutex_lock(&mutex);
                    cout << "MYSQL_QUERY #8: " << DBQuery.c_str() <<endl;
                    pthread_mutex_unlock(&mutex);
					res = mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));

                    pthread_mutex_lock(&mutex);
                    cout << "MYSQL_QUERY #8: " << DBQuery.c_str() << " return " << intToStr(res) << endl;
                    pthread_mutex_unlock(&mutex);

                    if(res != 0)
                    {
                      mysql_close(&Database);
                      DBUse = false;
                      //cout << "no res" << endl;
                    }
                    if(DBUse)
                    {
                      Result = mysql_store_result(&Database);
                      if(Result)
                      {
                        if(mysql_num_rows(Result) == 1)
                        {
                          while(Row = mysql_fetch_row(Result))
                          {
                            unsigned long *lengths;
                            lengths = mysql_fetch_lengths(Result);
                            accesslevel = Row[2];
                            id = strToInt(Row[0], error);
                            nick = Row[1];
                            name = Row[4];
                            surname = Row[5];
                            position = Row[6];
                            groups = Row[3];
                            dbid = Row[7];
                            avatar = Row[8];
                            user_email = Row[9];
                          }
                          mysql_free_result(Result);
                          DBQuery = "UPDATE " + DBPrefix + "users SET last_login=current_timestamp WHERE uID=" + intToStr(id);
                          res = mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));

                          pthread_mutex_lock(&mutex);
                          cout << "MYSQL_QUERY #9: " << DBQuery.c_str() << " return " << intToStr(res) << endl;
                          pthread_mutex_unlock(&mutex);

                          if(res != 0)
                          {
                            mysql_close(&Database);
                            DBUse = false;
                          }
                        }
                        else
                        {
                          id = -1;
                        }
                      }
                      else
                      {

                      }
                    }
                    //odlacz od bazy
                    mysql_close(&Database);
                  }
                  else
                  {
					  cout << "dupa" <<endl;
                    DBUse = false;
                    //odlacz od bazy
                    mysql_close(&Database);
                  }
                }
                //zdjecie blokady bazy
                pthread_mutex_unlock(&db_mutex);

                //cout << "7" << endl;
                if(id >= 0 && id < MaxUser)
                {

                  IdStr = intToStr(id);

                  first = false;
                  long long int tmpid;
                  pthread_mutex_lock(&user_mutex);
                  Configuration.UserTab[id].getID(tmpid);

                  if(tmpid == id)
                  {
                    Configuration.UserTab[id].kick();

                    pthread_mutex_lock(&mutex);
                    close(fd);
                    Configuration.deleteFdFromSockTab(fd);
                    Configuration.subThreadNumber();
                    pthread_mutex_unlock(&mutex);
                    pthread_mutex_unlock(&user_mutex);

                    //posprzataj:
                    delete [] Buffer;
                    Buffer = NULL;
                    pthread_mutex_lock(&options_mutex);
                    __ID[__id] = -1;
                    pthread_mutex_unlock(&options_mutex);

                    pthread_exit(0);
                  }

                  if(Configuration.addUserToUserTab(fd, id) == false)
                  {
                    //cout << "FAIL TO LOG_IN: " << nick << "(" << IdStr << ")" << endl;
                    pthread_mutex_lock(&mutex);
                    close(fd);
                    Configuration.deleteFdFromSockTab(fd);
                    Configuration.subThreadNumber();
                    pthread_mutex_unlock(&mutex);
                    pthread_mutex_unlock(&user_mutex);

                    //posprzataj:
                    delete [] Buffer;
                    Buffer = NULL;
                    pthread_mutex_lock(&options_mutex);
                    __ID[__id] = -1;
                    pthread_mutex_unlock(&options_mutex);

                    pthread_exit(0);
                  }
                  pthread_mutex_unlock(&user_mutex);

                  pthread_mutex_lock(&user_mutex);
                  Configuration.UserTab[id].status.setType(variable3);

                  for(int i = 0; i < MaxRoom; i++)
                  {
                    Configuration.UserTab[id].InRoom[i] = -1;
                  }
                  pthread_mutex_unlock(&user_mutex);

                  //oblicz MyIndex
                  MyIndex = id;
                  //cout << "B" << endl;
                  Message = "type=com_NewUser";
                  Message += "&id=";
                  Message += IdStr;
                  Message += "&dbid=";
                  Message += dbid;

                  Message += "&nick=";
                  Message += nick;
                  Message += "&name=";
                  Message += name;
                  Message += "&surname=";
                  Message += surname;
                  Message += "&position=";
                  Message += position;
                  Message += "&groups=";
                  Message += groups;
                  Message += "&email=";
                  Message += user_email;
                  Message += "&t=";
                  Message += intToStr(TimeIn);

                  pthread_mutex_lock(&user_mutex);
                  Configuration.UserTab[id].setLoginData(Message);
                  pthread_mutex_unlock(&user_mutex);
                  //ns -> może sie zmieniac,dlatego nie jest dopisywany do LoginData
                  Message += "&ns=";
                  pthread_mutex_lock(&user_mutex);
                  Message += Configuration.UserTab[id].status.getType();
                  pthread_mutex_unlock(&user_mutex);
                  Message += "&&";

                  //wiadomosc do LoginDataForClient (chat)

                  string LDFC;

                  LDFC = "type=com_NewUser";
                  LDFC += "&id=";
                  LDFC += IdStr;

                  LDFC += "&nick=";
                  LDFC += nick;
                  LDFC += "&name=";
                  LDFC += name;
                  LDFC += "&surname=";
                  LDFC += surname;
                  LDFC += "&position=";
                  LDFC += position;
                  LDFC += "&groups=";
                  LDFC += groups;
                  LDFC += "&email=";
                  LDFC += user_email;
                  LDFC += "&avtr=";
                  LDFC += avatar;
                  pthread_mutex_lock(&user_mutex);
                  Configuration.UserTab[id].setLoginDataForClient(LDFC);
                  pthread_mutex_unlock(&user_mutex);
                  //nie konczy sie na &&, poniewaz jest wykorzystywana tylko w user_renew dla clienta (chata)
                  //a chat sobie sam dopisuje '&ns=...&&'

                  //wiadomosc tylko do usera ktory sie loguje
                  Message2 = "type=event_YourID";
                  Message2 += "&id=";
                  Message2 += IdStr;
                  Message2 += "&dbid=";
                  Message2 += dbid;
                  Message2 += "&ip=";
                  Message2 += IP;
                  Message2 += "&ns=";

                  pthread_mutex_lock(&user_mutex);
                  Message2 += Configuration.UserTab[id].status.getType();
                  pthread_mutex_unlock(&user_mutex);

                  Message2 += "&nick=";
                  Message2 += nick;
                  Message2 += "&name=";
                  Message2 += name;
                  Message2 += "&surname=";
                  Message2 += surname;
                  Message2 += "&position=";
                  Message2 += position;
                  Message2 += "&groups=";
                  Message2 += groups;
                  Message2 += "&email=";
                  Message2 += user_email;
                  Message2 += "&priv=";
                  Message2 += accesslevel;
                  Message2 += "&t=";
                  Message2 += intToStr(TimeIn);
                  Message2 += "&&";
                  //cout << "C" << endl;
                  //rozeslij

                  //dorzuć do SendQueue
                  pthread_mutex_lock(&user_mutex);
                  Configuration.UserTab[MyIndex].SendQueue.push(Message2);
                  pthread_mutex_unlock(&user_mutex);

                  for(int i = 0; i < MaxUser; i++)
                  {
                    long long int tmpfd;

                    pthread_mutex_lock(&user_mutex);
                    Configuration.UserTab[i].getFD(tmpfd);
                    if(tmpfd != -1 && tmpfd != fd)
                    {
                      Configuration.UserTab[i].SendQueue.push(Message);
                    }
                    pthread_mutex_unlock(&user_mutex);
                  }
                  //cout << "D" << endl;
                }
                else
                {
                  //cout << "E" << endl;
                  //user not exist or bad data
                  pthread_mutex_lock(&mutex);
                  close(fd);
                  Configuration.subThreadNumber();
                  Configuration.deleteFdFromSockTab(fd);
                  pthread_mutex_unlock(&mutex);

                  //posprzataj:
                  delete [] Buffer;
                  Buffer = NULL;
                  pthread_mutex_lock(&options_mutex);
                  __ID[__id] = -1;
                  pthread_mutex_unlock(&options_mutex);

                  pthread_exit(0);
                }
              }
              else
              {
                //nie mozna zalogowac !!
                pthread_mutex_lock(&mutex);
                close(fd);
                Configuration.subThreadNumber();
                Configuration.deleteFdFromSockTab(fd);
                pthread_mutex_unlock(&mutex);

                //posprzataj:
                delete [] Buffer;
                Buffer = NULL;
                pthread_mutex_lock(&options_mutex);
                __ID[__id] = -1;
                pthread_mutex_unlock(&options_mutex);

                pthread_exit(0);
              }
            }
            else
            {
              //nie mozna zalogowac !!
              pthread_mutex_lock(&mutex);
              close(fd);
              Configuration.subThreadNumber();
              Configuration.deleteFdFromSockTab(fd);
              pthread_mutex_unlock(&mutex);

              //posprzataj:
              delete [] Buffer;
              Buffer = NULL;
              pthread_mutex_lock(&options_mutex);
              __ID[__id] = -1;
              pthread_mutex_unlock(&options_mutex);

              pthread_exit(0);
            }
          }
          else
          {
            //nie mozna zalogowac !!
            pthread_mutex_lock(&mutex);
            close(fd);
            Configuration.subThreadNumber();
            Configuration.deleteFdFromSockTab(fd);
            pthread_mutex_unlock(&mutex);

            //posprzataj:
            delete [] Buffer;
            Buffer = NULL;
            pthread_mutex_lock(&options_mutex);
            __ID[__id] = -1;
            pthread_mutex_unlock(&options_mutex);

            pthread_exit(0);
          }
          ///wyslij wszystko z kolejki
          //1.odczytaj
          pthread_mutex_lock(&user_mutex);
          updateSendQueue(Configuration.UserTab[MyIndex].SendQueue, SendQueue);
          pthread_mutex_unlock(&user_mutex);
          //2.wyslij
          sendFromQueue(fd, SendQueue);
        }
        else if(ActiveBuffer[0] == '+')///CLIENT (GUEST) CASE
        {
          //------------------- FILTROWANIE PO IP --------------------------------------
          pthread_mutex_lock(&local_bans_mutex);
          if(IPLocalBans.find(tmpBanIP) != string::npos)
          {
            pthread_mutex_lock(&mutex);
            Configuration.subThreadNumber();
            close(fd);
            pthread_mutex_unlock(&mutex);
            pthread_mutex_unlock(&local_bans_mutex);

            //posprzataj:
            delete [] Buffer;
            Buffer = NULL;
            pthread_mutex_lock(&options_mutex);
            __ID[__id] = -1;
            pthread_mutex_unlock(&options_mutex);
            pthread_exit(0);
          }
          pthread_mutex_unlock(&local_bans_mutex);

          //jezeli nie zbanowany to jaaazda dalej ;)

          string tmpDomain = parseFor(ActiveBuffer, "dom", error); //domena

          //odczytaj shutdown_state
          pthread_mutex_lock(&mutex);
          int tmp_shutdown = SHUTDOWN_STATE;
          pthread_mutex_unlock(&mutex);

          if(error == 0 && tmp_shutdown == 0)
          {
            tmpDomain = "," + tmpDomain + ",";

            //przeszukaj tmpDomain w poszukiwaniu dokładnie domeny
            if(allow_subdomains == false)
            {
              if(client_domains.find(tmpDomain) == string::npos)//jezeli nie znajdzie wpisu
              {
                pthread_mutex_lock(&mutex);
                close(fd);
                Configuration.subThreadNumber();
                pthread_mutex_unlock(&mutex);

                //posprzataj:
                delete [] Buffer;
                Buffer = NULL;
                pthread_mutex_lock(&options_mutex);
                __ID[__id] = -1;
                pthread_mutex_unlock(&options_mutex);
                pthread_exit(0);
              }
            }
            else //przeszukaj tmpDomain w poszukiwaniu domeny
            {
              bool found = false; //znalazl = true, else = false
              string tmp;
              //rozklada client_domains na domeny i odrazu porownuje z tmpDomain
              for(int i = 0; i < client_domains.length(); i++)
              {
                if(client_domains[i] == ',' || i == client_domains.length() - 1)
                {
                  //sprawdz dlugosc i przeszukaj
                  if(tmp.length() > 3)
                  {
                    if(tmpDomain.find(tmp) != string::npos)//jezeli znajdzie
                    {
                      found = true;
                      break;
                    }
                  }
                  else
                  {
                    tmp.clear();
                  }
                }
                else
                {
                  tmp += client_domains[i];
                }
              }
              if(!found)//jezeli nie znalazl
              {
                pthread_mutex_lock(&mutex);
                close(fd);
                Configuration.subThreadNumber();
                pthread_mutex_unlock(&mutex);

                //posprzataj:
                delete [] Buffer;
                Buffer = NULL;
                pthread_mutex_lock(&options_mutex);
                __ID[__id] = -1;
                pthread_mutex_unlock(&options_mutex);
                pthread_exit(0);
              }
            }
          }
          else
          {
            //nie mozna zalogowac
            pthread_mutex_lock(&mutex);
            close(fd);
            Configuration.subThreadNumber();
            pthread_mutex_unlock(&mutex);

            //posprzataj:
            delete [] Buffer;
            Buffer = NULL;
            pthread_mutex_lock(&options_mutex);
            __ID[__id] = -1;
            pthread_mutex_unlock(&options_mutex);
            pthread_exit(0);
          }

          pthread_mutex_lock(&client_mutex);
          Configuration.nextCID(id);

          IdStr = intToStr(id);

          if(Configuration.getNumClient() >= MaxClient)
          {
            //nie mozna zalogowac
            pthread_mutex_lock(&mutex);
            close(fd);
            Configuration.subThreadNumber();
            pthread_mutex_unlock(&mutex);
            pthread_mutex_unlock(&client_mutex);

            //posprzataj:
            delete [] Buffer;
            Buffer = NULL;
            pthread_mutex_lock(&options_mutex);
            __ID[__id] = -1;
            pthread_mutex_unlock(&options_mutex);
            pthread_exit(0);
          }
          pthread_mutex_unlock(&client_mutex);

          string ref_id = parseFor(ActiveBuffer, "ref_dbid", error); //reference_id => !bazowe!
          if(error == 0 && ref_id != "E")
          {
            int ref_id_int = strToInt(ref_id, error);
            if(ref_id_int >= 0 && statistics_on)
            {
              //dopisuje invite'a do bazy danych
              MYSQL Database;

              DBQuery = "INSERT INTO " + DBPrefix + "sk_actions (type, consultant,date) VALUES (3," + ref_id + ", current_timestamp)";

              pthread_mutex_lock(&db_mutex);
              if(!mysql_init(&Database))
              {
                //cout << "THERE WERE AN ERROR WITH MYSQL_INIT" << endl;
              }
              else
              {
                if(mysql_real_connect(&Database, DBHost.c_str(), DBUsername.c_str(), DBPassword.c_str(), DBDataBase.c_str(), 0, NULL, 0))
                {
                  int res = mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));

                  pthread_mutex_lock(&mutex);
                  cout << "MYSQL_QUERY #10: " << DBQuery.c_str() << " return " << intToStr(res) << endl;
                  pthread_mutex_unlock(&mutex);

                  mysql_close(&Database);
                }
              }
              pthread_mutex_unlock(&db_mutex);
            }
          }
          else
          {
            //nie mozna zalogowac
            pthread_mutex_lock(&mutex);
            close(fd);
            Configuration.subThreadNumber();
            pthread_mutex_unlock(&mutex);

            //posprzataj:
            delete [] Buffer;
            Buffer = NULL;
            pthread_mutex_lock(&options_mutex);
            __ID[__id] = -1;
            pthread_mutex_unlock(&options_mutex);
            pthread_exit(0);
          }
          pthread_mutex_lock(&client_mutex);
          pthread_mutex_lock(&mutex);
          if(Configuration.addFdToSockTab(fd) == false)
          {
            //brak miejsc w SockTab
            close(fd);
            Configuration.subThreadNumber();
            pthread_mutex_unlock(&mutex);
            pthread_mutex_unlock(&client_mutex);

            //posprzataj:
            delete [] Buffer;
            Buffer = NULL;
            pthread_mutex_lock(&options_mutex);
            __ID[__id] = -1;
            pthread_mutex_unlock(&options_mutex);
            pthread_exit(0);
          }

          if(Configuration.addClientToClientTab(fd, id) == false)
          {
            //cout << "FAIL TO LOG_IN: " << nick << "(" << IdStr << ")" << endl;
            close(fd);
            Configuration.deleteFdFromSockTab(fd);
            Configuration.subThreadNumber();
            pthread_mutex_unlock(&mutex);
            pthread_mutex_unlock(&client_mutex);

            //posprzataj:
            delete [] Buffer;
            Buffer = NULL;
            pthread_mutex_lock(&options_mutex);
            __ID[__id] = -1;
            pthread_mutex_unlock(&options_mutex);
            pthread_exit(0);
          }
          pthread_mutex_unlock(&mutex);
          pthread_mutex_unlock(&client_mutex);

          nick = parseFor(ActiveBuffer, "nick", error);
          if(error != 0)
          {
            nick = "unknown";
          }
          logged_as = 1;
          first = false;

          //obliczanie MyIndex
          MyIndex = id;

          //otwiera pokoj
          pthread_mutex_lock(&room_mutex);
          Configuration.openRoom(rid);
          pthread_mutex_unlock(&room_mutex);
          if(rid == -1)
          {
            pthread_mutex_lock(&client_mutex);

            RidStr = intToStr(rid);
            //cout << "FAIL CREATE ROOM" << endl;
            close(fd);
            Configuration.deleteClientFromClientTab(fd);
            pthread_mutex_lock(&mutex);
            Configuration.deleteFdFromSockTab(fd);
            Configuration.subThreadNumber();
            pthread_mutex_unlock(&mutex);
            pthread_mutex_unlock(&client_mutex);

            //posprzataj:
            delete [] Buffer;
            Buffer = NULL;
            pthread_mutex_lock(&options_mutex);
            __ID[__id] = -1;
            pthread_mutex_unlock(&options_mutex);
            pthread_exit(0);

            pthread_exit(0);
          }

          RidStr = intToStr(rid);

          //dodaje guesta do pokoj
          bool err = true;
          pthread_mutex_lock(&client_mutex);
          Configuration.addClientToRoomTab(rid, id, err);
          if(err)
          {
            //cout << "FAIL TO JOIN ROOM" << endl;
            Configuration.deleteClientFromClientTab(fd);

            pthread_mutex_lock(&mutex);
            close(fd);
            Configuration.deleteFdFromSockTab(fd);
            Configuration.subThreadNumber();
            pthread_mutex_unlock(&mutex);

            pthread_mutex_lock(&room_mutex);
            Configuration.closeRoom(rid);
            pthread_mutex_unlock(&room_mutex);
            pthread_mutex_unlock(&client_mutex);

            //posprzataj:
            delete [] Buffer;
            Buffer = NULL;
            pthread_mutex_lock(&options_mutex);
            __ID[__id] = -1;
            pthread_mutex_unlock(&options_mutex);
            pthread_exit(0);
          }
          pthread_mutex_unlock(&client_mutex);
          //Uzupelnia tablce ClientTab dodatkowymi informacjami
          pthread_mutex_lock(&rand_mutex);
          pthread_mutex_lock(&mutex);
          int geo_range = Configuration.getGeoRange();
          int geo_port = 15001 + (rand() % geo_range);
          pthread_mutex_unlock(&mutex);
          pthread_mutex_unlock(&rand_mutex);

          pthread_mutex_lock(&client_mutex);
          Configuration.ClientTab[id].setIP(IP);
          pthread_mutex_unlock(&client_mutex);

          pthread_mutex_lock(&geo_client_mutex);
          Configuration.ClientTab[id].getGeoData(geo_port);
          pthread_mutex_unlock(&geo_client_mutex);

          string local_TimeIn = intToStr(TimeIn);

          //HASH dla  pokoju (data utworzenia)

          string ROOMHASH = intToStr((int) ((time_t) time(NULL)));

          pthread_mutex_lock(&room_mutex);
          Configuration.RoomTab[rid].setHash(ROOMHASH);
          pthread_mutex_unlock(&room_mutex);

          //przygotuj tmpActiveBuffer
          string tmpActiveBuffer = ActiveBuffer;
          if(tmpActiveBuffer[0] == '+')
          {
            tmpActiveBuffer[0] = '&';
          }

          pthread_mutex_lock(&client_mutex);
          Configuration.ClientTab[MyIndex].InRoom = rid;
          Configuration.ClientTab[id].setTimeIn(local_TimeIn);
          Configuration.ClientTab[id].setRoomHash(ROOMHASH);
          Configuration.ClientTab[id].setClientData(tmpActiveBuffer);
          Configuration.ClientTab[id].setRidStr(RidStr);
          Configuration.ClientTab[id].setCompleteData();
          string Geo_data;
          Configuration.ClientTab[id].getGeoFromVariable(Geo_data);
          pthread_mutex_unlock(&client_mutex);

          //oblicza czas rozpoczecia i dopisuje do historii
          pthread_mutex_lock(&time_mutex);
          char timestamp[100];
          time_t mytime;
          struct tm *mytm;
          mytime = time(NULL);
          mytm = localtime(&mytime);
          strftime(timestamp, sizeof timestamp, "%F %T", mytm);
          date.assign(timestamp);
          pthread_mutex_unlock(&time_mutex);

          string tmp_history_header = "<c d=\"" + date + "\">";

          pthread_mutex_lock(&client_mutex);
          Configuration.ClientTab[id].addToHistory(tmp_history_header);
          pthread_mutex_unlock(&client_mutex);

          //wysyla informacje do wszystkich userow
          Message = "type=com_NewGuest";
          Message += "&id=";
          Message += IdStr;
          Message += "&";
          Message += Geo_data;
          Message += "&rid=";
          Message += RidStr;
          Message += "&h=";
          Message += ROOMHASH;
          Message += "&ip=";
          Message += IP;
          Message += "&t=";
          Message += intToStr(TimeIn);
          string tmpBuffer;
          tmpBuffer.assign(ActiveBuffer);
          tmpBuffer[0] = '&'; //zamienia pierwszy znak , czyli "+" na & aby dovbrze zlaczyc.
          if(tmpBuffer[tmpBuffer.length() - 2] == '&' || tmpBuffer[tmpBuffer.length() - 1] == '&')//odcina koncowe &&
          {
            string tmpBuffer2;
            for(int i = 0; i < tmpBuffer.length() - 2; i++)
            {
              tmpBuffer2 += tmpBuffer[i];
            }
            tmpBuffer = tmpBuffer2;
          }
          Message += tmpBuffer;
          Message += "&&";
          for(int i = 0; i < MaxUser; i++)
          {
            long long int tmpfd;
            pthread_mutex_lock(&user_mutex);
            Configuration.UserTab[i].getFD(tmpfd);

            if(tmpfd != fd && tmpfd != -1)
            {
              Configuration.UserTab[i].SendQueue.push(Message);
            }
            pthread_mutex_unlock(&user_mutex);

          }
          //wyslanie informacji o czasie wejscia
          Message = "type=data&t=";
          Message += intToStr(TimeIn);
          Message += "&&";
          pthread_mutex_lock(&client_mutex);
          Configuration.ClientTab[MyIndex].SendQueue.push(Message);
          pthread_mutex_unlock(&client_mutex);

          ///wyslij wszystko z kolejki
          //1.odczytaj
          pthread_mutex_lock(&client_mutex);
          updateSendQueue(Configuration.ClientTab[MyIndex].SendQueue, SendQueue);
          pthread_mutex_unlock(&client_mutex);
          //2.wyslij
          sendFromQueue(fd, SendQueue);
        }
        else if(ActiveBuffer[0] == '*')///MR BUG CASE
        {
          pthread_mutex_lock(&local_bans_mutex);
          if(IPLocalBans.find(tmpBanIP) != string::npos)
          {
            pthread_mutex_lock(&mutex);
            Configuration.subThreadNumber();
            close(fd);
            pthread_mutex_unlock(&mutex);
            pthread_mutex_unlock(&local_bans_mutex);

            //posprzataj:
            delete [] Buffer;
            Buffer = NULL;
            pthread_mutex_lock(&options_mutex);
            __ID[__id] = -1;
            pthread_mutex_unlock(&options_mutex);
            pthread_exit(0);
          }
          pthread_mutex_unlock(&local_bans_mutex);


          string tmpDomain = parseFor(ActiveBuffer, "dom", error);
          if(error == 0)
          {
            tmpDomain = "," + tmpDomain + ",";

            //przeszukaj tmpDomain w poszukiwaniu dokładnie domeny
            if(allow_subdomains == false)
            {
              if(client_domains.find(tmpDomain) == string::npos)//jezeli nie znajdzie wpisu
              {
                pthread_mutex_lock(&mutex);
                close(fd);
                Configuration.subThreadNumber();
                pthread_mutex_unlock(&mutex);

                //posprzataj:
                delete [] Buffer;
                Buffer = NULL;
                pthread_mutex_lock(&options_mutex);
                __ID[__id] = -1;
                pthread_mutex_unlock(&options_mutex);
                pthread_exit(0);
              }
            }
            else //przeszukaj tmpDomain w poszukiwaniu domeny
            {
              bool found = false; //znalazl = true, else = false
              string tmp;
              //rozklada client_domains na domeny i odrazu porownuje z tmpDomain
              for(int i = 0; i < client_domains.length(); i++)
              {
                if(client_domains[i] == ',' || i == client_domains.length() - 1)
                {
                  //sprawdz dlugosc i przeszukaj
                  if(tmp.length() > 3)
                  {
                    if(tmpDomain.find(tmp) != string::npos)//jezeli znajdzie
                    {
                      found = true;
                      break;
                    }
                  }
                  else
                  {
                    tmp.clear();
                  }
                }
                else
                {
                  tmp += client_domains[i];
                }
              }
              if(!found)//jezeli nie znalazl
              {
                pthread_mutex_lock(&mutex);
                close(fd);
                Configuration.subThreadNumber();
                pthread_mutex_unlock(&mutex);

                //posprzataj:
                delete [] Buffer;
                Buffer = NULL;
                pthread_mutex_lock(&options_mutex);
                __ID[__id] = -1;
                pthread_mutex_unlock(&options_mutex);
                pthread_exit(0);
              }
            }
          }
          else
          {
            pthread_mutex_lock(&mutex);
            Configuration.subThreadNumber();
            close(fd);
            pthread_mutex_unlock(&mutex);

            //posprzataj:
            delete [] Buffer;
            Buffer = NULL;
            pthread_mutex_lock(&options_mutex);
            __ID[__id] = -1;
            pthread_mutex_unlock(&options_mutex);
            pthread_exit(0);
          }

          pthread_mutex_lock(&mrbug_mutex);
          Configuration.nextMID(id);
          IdStr = intToStr(id);
          pthread_mutex_lock(&mutex);

          if(Configuration.getNumMrBug() >= MaxMrBug)
          {
            //nie mozna zalogowac

            close(fd);
            Configuration.subThreadNumber();
            pthread_mutex_unlock(&mutex);
            pthread_mutex_unlock(&mrbug_mutex);

            //posprzataj:
            delete [] Buffer;
            Buffer = NULL;
            pthread_mutex_lock(&options_mutex);
            __ID[__id] = -1;
            pthread_mutex_unlock(&options_mutex);
            pthread_exit(0);
          }
          if(Configuration.addFdToSockTab(fd) == false)
          {
            //brak miejsc w SockTab
            close(fd);
            Configuration.subThreadNumber();
            pthread_mutex_unlock(&mutex);
            pthread_mutex_unlock(&mrbug_mutex);
            //posprzataj:
            delete [] Buffer;
            Buffer = NULL;
            pthread_mutex_lock(&options_mutex);
            __ID[__id] = -1;
            pthread_mutex_unlock(&options_mutex);
            pthread_exit(0);
          }
          if(Configuration.addMrBugToMrBugTab(fd, id) == false)
          {
            close(fd);
            Configuration.subThreadNumber();
            Configuration.deleteFdFromSockTab(fd);

            pthread_mutex_unlock(&mutex);
            pthread_mutex_unlock(&mrbug_mutex);

            //posprzataj:
            delete [] Buffer;
            Buffer = NULL;
            pthread_mutex_lock(&options_mutex);
            __ID[__id] = -1;
            pthread_mutex_unlock(&options_mutex);
            pthread_exit(0);
          }
          pthread_mutex_unlock(&mutex);
          pthread_mutex_unlock(&mrbug_mutex);

          pthread_mutex_lock(&rand_mutex);
          pthread_mutex_lock(&mutex);
          int geo_range = Configuration.getGeoRange();
          int geo_port = 15001 + (rand() % geo_range);
          pthread_mutex_unlock(&mutex);
          pthread_mutex_unlock(&rand_mutex);

          pthread_mutex_lock(&mrbug_mutex);
          Configuration.MrBugTab[id].setIP(IP);
          pthread_mutex_unlock(&mrbug_mutex);

          pthread_mutex_lock(&geo_mrbug_mutex);
          Configuration.MrBugTab[id].getGeoData(geo_port);
          pthread_mutex_unlock(&geo_mrbug_mutex);

          string local_TimeIn = intToStr(TimeIn);

          //przygotuj tmpActiveBuffer
          string tmpActiveBuffer = ActiveBuffer;
          if(tmpActiveBuffer[0] == '+')
          {
            tmpActiveBuffer[0] = '&';
          }

          pthread_mutex_lock(&mrbug_mutex);
          Configuration.MrBugTab[id].setTimeIn(local_TimeIn);
          Configuration.MrBugTab[id].setBugData(tmpActiveBuffer);
          Configuration.MrBugTab[id].setCompleteData();
          string Geo_data;
          Configuration.MrBugTab[id].getGeoFromVariable(Geo_data);
          pthread_mutex_unlock(&mrbug_mutex);

          logged_as = 2;
          first = false;
          Message = "type=com_NewBug&";
          Message += "id=";
          Message += IdStr;
          Message += "&";
          Message += Geo_data;
          Message += "&ip=";
          Message += IP;
          Message += "&t=";
          Message += intToStr(TimeIn);
          string tmpBuffer;
          tmpBuffer.assign(ActiveBuffer);
          tmpBuffer[0] = '&'; //zamienia pierwszy znak, czyli "*" na &, aby dobrze zlaczyc.
          if(tmpBuffer[tmpBuffer.length() - 2] == '&' || tmpBuffer[tmpBuffer.length() - 1] == '&')//odcina koncowe "&&"
          {
            string tmpBuffer2;
            for(int i = 0; i < tmpBuffer.length() - 2; i++)
            {
              tmpBuffer2 += tmpBuffer[i];
            }
            tmpBuffer = tmpBuffer2;
          }
          Message += tmpBuffer;
          Message += "&&";

          //oblicz MyIndex
          for(int i = 0; i < MaxMrBug; i++)
          {
            long long int tmpfd;
            pthread_mutex_lock(&mrbug_mutex);
            Configuration.MrBugTab[i].getFD(tmpfd);
            pthread_mutex_unlock(&mrbug_mutex);
            if(tmpfd == fd)
            {
              MyIndex = i;
              break;
            }
          }
          int error;

          browser = parseFor(Message.c_str(), "brow", error);
          page = parseFor(Message.c_str(), "url", error);
          ref_page = parseFor(Message.c_str(), "ref", error);
          system = parseFor(Message.c_str(), "os", error);
          language = parseFor(Message.c_str(), "lang", error);
          resolution = parseFor(Message.c_str(), "res", error);
          aim = parseFor(Message.c_str(), "dom", error);
          returning = parseFor(Message.c_str(), "ret", error);
          country = parseFor(Message.c_str(), "country", error);
          city = parseFor(Message.c_str(), "city", error);

          pthread_mutex_lock(&time_mutex);
          char timestamp[100];
          time_t mytime;
          struct tm *mytm;
          mytime = time(NULL);
          mytm = localtime(&mytime);
          strftime(timestamp, sizeof timestamp, "%F %T", mytm);
          date.assign(timestamp);
          pthread_mutex_unlock(&time_mutex);

          //wyslij do wszystkich aktywnych userow informacje
          for(int i = 0; i < MaxUser; i++)
          {
            long long int tmpfd;
            pthread_mutex_lock(&user_mutex);
            Configuration.UserTab[i].getFD(tmpfd);
            if(tmpfd > 0 && tmpfd < MaxUser)
            {
              Configuration.UserTab[i].SendQueue.push(Message);
            }
            pthread_mutex_unlock(&user_mutex);
          }
          //wyslij do buga informacje wolnych userow
          //type=data&cons=5&invit=342&stime=1353453453&&

          //zlicz wolnych userkow:
          int free = 0;

          string freeUser = "type=data&cons=0&invit=" + InvitationDATA;
          freeUser += "&t=";
          freeUser += intToStr(TimeIn);
          freeUser += "&&";
          pthread_mutex_lock(&user_mutex);
          //          for(int i = 0; i < MaxUser; i++)
          //          {
          //            if(Configuration.UserTab[i].getID() >= 0)
          //            {
          //              string tmpStatus = Configuration.UserTab[i].status.getType();
          //              if(tmpStatus.length() > 0)
          //              {
          //                if(tmpStatus[0] == '1')
          //                {
          //                  freeUser = "type=data&cons=1&invit=" + InvitationDATA;
          //                  freeUser += "&t=";
          //
          //                  freeUser += intToStr(TimeIn);
          //                  freeUser += "&&";
          //                  break;
          //                }
          //              }
          //            }
          //          }

          for(int i = 0; i < MaxUser; i++)
          {
            if(Configuration.UserTab[i].getID() >= 0)
            {
              string tmpStatus = Configuration.UserTab[i].status.getType();
              if(tmpStatus.length() > 0)
              {
                if(tmpStatus[0] == '1')
                {
                  free++;
                }
              }
            }
          }
          freeUser = "type=data&cons=" + intToStr(free) + "&invit=" + InvitationDATA;
          freeUser += "&t=";

          freeUser += intToStr(TimeIn);
          freeUser += "&&";

          pthread_mutex_unlock(&user_mutex);

          pthread_mutex_lock(&mrbug_mutex);
          Configuration.MrBugTab[MyIndex].SendQueue.push(freeUser);
          pthread_mutex_unlock(&mrbug_mutex);

          ///wyslij wszystko z kolejki
          //1.odczytaj
          pthread_mutex_lock(&mrbug_mutex);
          updateSendQueue(Configuration.MrBugTab[MyIndex].SendQueue, SendQueue);
          pthread_mutex_unlock(&mrbug_mutex);
          //2.wyslij
          sendFromQueue(fd, SendQueue);
        }
        else if(ActiveBuffer[0] == '#')///OTHER CASE
        {
          //other
          logged_as = 3;
          first = false;
        }
        else///NOT KNOWN
        {
          logged_as = -1;
          string info = "";

          pthread_mutex_lock(&mutex);
          Configuration.subThreadNumber();
          close(fd);
          pthread_mutex_unlock(&mutex);

          delete [] Buffer;
          Buffer = NULL;

          Message.clear();
          Message2.clear();
          Message3.clear();

          PortStr.clear();

          variable.clear();
          variable2.clear();
          variable3.clear();
          info.clear();

          //posprzataj:
          pthread_mutex_lock(&options_mutex);
          __ID[__id] = -1;
          pthread_mutex_unlock(&options_mutex);
          pthread_exit(0);
        }
      }///----------------------------------------------------------
        ///------------------JEZELI ZALOGOWANY-------
        ///----------------------------------------------------------
      else
      {
        if(logged_as == 0)///USER CASE
        {
          variable = parseFor(ActiveBuffer, "type", error);

          if(error == 0)
          {
            if(variable == "rcreate")///tworzy i wchodzi do pokoju
            {
              //otwiera pokoj
              pthread_mutex_lock(&room_mutex);
              Configuration.openRoom(rid);
              pthread_mutex_unlock(&room_mutex);

              if(rid == -1)
              {
                RidStr = intToStr(rid);
                //cout << "FAIL TO CREATE ROOM" << endl;
                //wyslij odpowiednie info
              }
              else
              {
                RidStr = intToStr(rid);
                //dodaje usera do pokoj
                bool err = true;

                pthread_mutex_lock(&room_mutex);
                Configuration.addUserToRoomTab(rid, id, err);
                pthread_mutex_unlock(&room_mutex);

                //Uzupelnia tablce InRoom
                if(!err)
                {
                  pthread_mutex_lock(&user_mutex);
                  for(int i = 0; i < MaxRoom; i++)
                  {
                    if(Configuration.UserTab[MyIndex].InRoom[i] == -1)
                    {
                      Configuration.UserTab[MyIndex].InRoom[i] = rid;
                      break;
                    }
                  }
                  pthread_mutex_unlock(&user_mutex);
                }
                else
                {
                  pthread_mutex_lock(&room_mutex);
                  Configuration.closeRoom(rid);
                  pthread_mutex_unlock(&room_mutex);
                }

                //HASH dla  pokoju (data utworzenia)
                string ROOMHASH = intToStr((int) ((time_t) time(NULL)));

                pthread_mutex_lock(&room_mutex);
                Configuration.RoomTab[rid].setHash(ROOMHASH);
                pthread_mutex_unlock(&room_mutex);

                Message = "type=conv_NewRoom&rid=" + RidStr + "&h=" + ROOMHASH + "&id=" + IdStr + "&&";
                //wysyla info do tworzacego
                pthread_mutex_lock(&user_mutex);
                Configuration.UserTab[MyIndex].SendQueue.push(Message);
                pthread_mutex_unlock(&user_mutex);
              }
            }
            else if(variable == "getServerInfo")
            {
              MYSQL Database;
              MYSQL_RES *Result;
              MYSQL_ROW Row;
              long res;

              string payed_from;
              string payed_to;
              string payed_id;
              string DBDataBaseTmp = "eConsultantdb1";

              pthread_mutex_lock(&db_mutex);
              if(mysql_init(&Database))
              {
                if(mysql_real_connect(&Database, DBHost.c_str(), DBUsername.c_str(), DBPassword.c_str(), DBDataBaseTmp.c_str(), 0, NULL, 0))
                {
                  //pobierz dane
                  DBQuery = "SELECT payed_from, payed_to, id  FROM _ec_clients WHERE db_prefix=\"" + DBPrefix + "\" LIMIT 1";
                  //cout << DBQuery;
                  res = mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));

                  pthread_mutex_lock(&mutex);
                  cout << "MYSQL_QUERY #11: " << DBQuery.c_str() << " return " << intToStr(res) << endl;
                  pthread_mutex_unlock(&mutex);

                  if(res != 0)
                  {
                    mysql_close(&Database);
                  }
                  else
                  {
                    Result = mysql_store_result(&Database);
                    if(Result)
                    {
                      if(mysql_num_rows(Result) == 1)
                      {
                        while(Row = mysql_fetch_row(Result))
                        {
                          unsigned long *lengths;
                          lengths = mysql_fetch_lengths(Result);
                          payed_from = Row[0];
                          payed_to = Row[1];
                          payed_id = Row[2];
                        }
                        mysql_free_result(Result);
                        res = mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));

                        pthread_mutex_lock(&mutex);
                        cout << "MYSQL_QUERY #12: " << DBQuery.c_str() << " return " << intToStr(res) << endl;
                        pthread_mutex_unlock(&mutex);

                        if(res != 0)
                        {
                          mysql_close(&Database);
                        }

                      }
                    }
                  }
                  //odlacz od bazy
                  mysql_close(&Database);
                }
                else
                {
                  mysql_close(&Database);
                }
              }
              //zdjecie blokady bazy
              pthread_mutex_unlock(&db_mutex);

              Message.clear();
              Message = "type=event_serverInfo&clientID=" + payed_id + "&from=" + payed_from + "&to=" + payed_to + "&&";

              pthread_mutex_lock(&user_mutex);
              Configuration.UserTab[MyIndex].SendQueue.push(Message);
              pthread_mutex_unlock(&user_mutex);
            }
            else if(variable == "rout")///wychodzi z pokoju
            {
              variable = parseFor(ActiveBuffer, "rid", error); //id pokoju - aby bylo wiadomo z ktorego pokoju wyjsc
              if(error == 0)
              {
                long long int tmp = strToInt(variable, error);
                bool inside = false;
                pthread_mutex_lock(&user_mutex);
                for(int i = 0; i < MaxRoom; i++)
                {
                  if(Configuration.UserTab[MyIndex].InRoom[i] == tmp)
                  {
                    inside = true;
                    Configuration.UserTab[MyIndex].InRoom[i] = -1;
                  }
                }
                pthread_mutex_unlock(&user_mutex);

                int tmp2 = tmp; //index pokoju w tablicy RoomTab
                if(inside)
                {
                  pthread_mutex_lock(&room_mutex);
                  Configuration.delUserFromRoomTab(tmp, id);
                  pthread_mutex_unlock(&room_mutex);

                  RidStr = intToStr(tmp);

                  Message = "type=conv_UROut&id=" + IdStr + "&rid=" + RidStr + "&reason=0&&";

                  pthread_mutex_lock(&user_mutex);
                  pthread_mutex_lock(&client_mutex);
                  pthread_mutex_lock(&room_mutex);

                  int maxUserInRoom = Configuration.RoomTab[tmp2].getMaxUser();
                  int maxClientInRoom = Configuration.RoomTab[tmp2].getMaxClient();
                  for(int i = 0; i < maxUserInRoom; i++)
                  {
                    if(Configuration.RoomTab[tmp2].UserMembersTab[i] != -1 && Configuration.RoomTab[tmp2].UserMembersTab[i] != id)
                    {
                      Configuration.UserTab[Configuration.RoomTab[tmp2].UserMembersTab[i]].SendQueue.push(Message);
                    }
                  }
                  for(int i = 0; i < maxClientInRoom; i++)
                  {
                    if(Configuration.RoomTab[tmp2].ClientMembersTab[i] != -1)
                    {
                      Configuration.ClientTab[Configuration.RoomTab[tmp2].ClientMembersTab[i]].SendQueue.push(Message);
                    }
                  }
                  pthread_mutex_unlock(&client_mutex);

                  Message = "type=conv_RClose&rid=" + RidStr + "&reason=0&&";
                  Configuration.UserTab[MyIndex].SendQueue.push(Message);

                  if(Configuration.RoomTab[tmp2].getNumClient() > 0 && Configuration.RoomTab[tmp2].getNumUser() == 0)
                  {
                    //Wysyla alert do istniejacych userow
                    for(int i = 0; i < MaxUser; i++)
                    {
                      long long int tmpfd, tmpid;
                      Configuration.UserTab[i].getFD(tmpfd);
                      Configuration.UserTab[i].getID(tmpid);
                      if(tmpfd > 0 && tmpid >= 0 && tmpid < MaxUser)
                      {
                        Message = "type=com_GuestAlarm&id=" + intToStr(Configuration.RoomTab[tmp2].ClientMembersTab[0]) + "&rid=" + RidStr + "&&";
                        Configuration.UserTab[tmpid].SendQueue.push(Message);
                      }
                    }
                  }
                  else if(Configuration.RoomTab[tmp2].getNumClient() + Configuration.RoomTab[tmp2].getNumUser() == 0)
                  {
                    Configuration.closeRoom(tmp2);
                  }
                  pthread_mutex_unlock(&room_mutex);
                  pthread_mutex_unlock(&user_mutex);
                }
              }
            }
            else if(variable == "rin")///wchodzi do pokoju
            {
              variable = parseFor(ActiveBuffer, "rid", error); //id pokoju - aby bylo wiadomo do ktorego pokoju wejsc
              if(error == 0)
              {
                long long int tmp = strToInt(variable, error);

                //sprawdz czy pokoj istnieje
                bool room_exist = false;

                pthread_mutex_lock(&room_mutex);
                for(int i = 0; i < MaxRoom; i++)
                {
                  if(Configuration.RoomTab[i].getID() == tmp)
                  {
                    room_exist = true;
                    break;
                  }
                }
                pthread_mutex_unlock(&room_mutex);
                //sprawdz czy uzytkownik nie siedzi juz w tym pokoju
                if(room_exist)
                {
                  bool not_inside = true;
                  pthread_mutex_lock(&user_mutex);
                  for(int i = 0; i < MaxRoom; i++)
                  {
                    if(Configuration.UserTab[MyIndex].InRoom[i] == tmp)
                    {
                      //nie moze wejsc bo juz tam siedzi
                      not_inside = false;
                      break;
                    }
                  }
                  pthread_mutex_unlock(&user_mutex);
                  //sprawdza czy hash pokoju jest zgodny z przeslanym
                  bool room_hash_ok = false;
                  string tmpRoomHash = parseFor(ActiveBuffer, "h", error); //id pokoju - aby bylo wiadomo do ktorego pokoju wejsc

                  pthread_mutex_lock(&room_mutex);
                  if(Configuration.RoomTab[tmp].getHash() == tmpRoomHash)
                  {
                    room_hash_ok = true;
                  }
                  pthread_mutex_unlock(&room_mutex);

                  //dodaj uzytkownika do pokoju, jezeli brak miejsc/brak uprawnien wyslij odp wiadomosc
                  int tmp2 = tmp; //<-- index pokoju w tablicy Configuration.RoomTab[i]
                  if(not_inside && room_hash_ok)
                  {
                    pthread_mutex_lock(&room_mutex);
                    if(Configuration.RoomTab[tmp].getID() == tmp)
                    {
                      bool err = true;
                      Configuration.addUserToRoomTab(tmp, id, err);
                      pthread_mutex_unlock(&room_mutex);
                      if(!err)
                      {
                        pthread_mutex_lock(&user_mutex);
                        for(int i = 0; i < MaxRoom; i++)
                        {
                          if(Configuration.UserTab[MyIndex].InRoom[i] == -1)
                          {
                            Configuration.UserTab[MyIndex].InRoom[i] = tmp;
                            break;
                          }
                        }
                        pthread_mutex_unlock(&user_mutex);
                        //rozeslij informacje
                        Message = "type=conv_RoomIn&rid=" + variable + "&h=" + tmpRoomHash + "&&"; //type=conv_msg&value=<n>GodOfDarkness</n><m>nabu to informatyk.</m><style>#FF0000#800000#0#0#1</style>&author=0&rid="+variable+"&at=u&&";
                        Message2 = "type=conv_URCome&id=" + IdStr + "&rid=" + variable + "&&"; //type=conv_msg&value=<n>GodOfDarkness</n><m>nabu to informatyk.</m><style>#FF0000#800000#0#0#1</style>&author=0&rid="+variable+"&at=u&&";
                        pthread_mutex_lock(&user_mutex);
                        pthread_mutex_lock(&client_mutex);
                        pthread_mutex_lock(&room_mutex);

                        int maxUserInRoom = Configuration.RoomTab[tmp2].getMaxUser();
                        int maxClientInRoom = Configuration.RoomTab[tmp2].getMaxClient();
                        for(int i = 0; i < maxUserInRoom; i++)
                        {
                          if(Configuration.RoomTab[tmp2].UserMembersTab[i] != -1 && Configuration.RoomTab[tmp2].UserMembersTab[i] != id)
                          {
                            Configuration.UserTab[Configuration.RoomTab[tmp2].UserMembersTab[i]].SendQueue.push(Message2);
                          }
                        }

                        for(int i = 0; i < maxClientInRoom; i++)
                        {
                          if(Configuration.RoomTab[tmp2].ClientMembersTab[i] != -1)
                          {
                            int tmpclientid = Configuration.RoomTab[tmp2].ClientMembersTab[i];
                            Configuration.ClientTab[tmpclientid].setUserTalkWith(id);
                            Configuration.ClientTab[tmpclientid].SendQueue.push(Message2);
                          }
                        }

                        //wyslij do siebie
                        Configuration.UserTab[MyIndex].SendQueue.push(Message);

                        RidStr = intToStr(tmp2);
                        rid = tmp2;

                        if(Configuration.RoomTab[rid].getNumClient() > 0 && Configuration.RoomTab[rid].getNumUser() == 1)
                        {
                          //Wysyla alert do istniejacych userow
                          for(int i = 0; i < MaxUser; i++)
                          {
                            long long int tmpfd;
                            Configuration.UserTab[i].getFD(tmpfd);
                            if(tmpfd != -1)
                            {
                              Message = "type=com_EndAlarm&id=" + intToStr(Configuration.RoomTab[rid].ClientMembersTab[0]) + "&rid=" + RidStr + "&&";
                              Configuration.UserTab[i].SendQueue.push(Message);
                            }
                          }
                        }
                        pthread_mutex_unlock(&room_mutex);
                        pthread_mutex_unlock(&client_mutex);
                        pthread_mutex_unlock(&user_mutex);
                      }
                      else
                      {
                        //nie mozna dodac. powiadom, brak miejsc
                        Message = "type=room_full&add=false&&";
                        pthread_mutex_lock(&user_mutex);
                        Configuration.UserTab[MyIndex].SendQueue.push(Message);
                        pthread_mutex_unlock(&user_mutex);
                      }
                    }
                    else
                    {
                      pthread_mutex_unlock(&room_mutex);
                    }
                  }
                  else
                  {
                    //nie mozna dodac. powiadom
                    if(!room_hash_ok)
                    {
                      Message = "type=event_error&rin&&";
                    }
                    else
                    {
                      Message = "type=room_not_allow&add=false&&";
                    }
                    pthread_mutex_lock(&user_mutex);
                    Configuration.UserTab[MyIndex].SendQueue.push(Message);
                    pthread_mutex_unlock(&user_mutex);
                  }
                }
                else
                {
                  //nie mozna dodac. powiadom
                  Message = "type=room_not_exists&add=false&&";
                  pthread_mutex_lock(&user_mutex);
                  Configuration.UserTab[MyIndex].SendQueue.push(Message);
                  pthread_mutex_unlock(&user_mutex);
                }
              }
            }
            else if(variable == "kick_user_from_room")///wyrzuca usera o podanym id z pokoju o podanym rid
            {
              variable = parseFor(ActiveBuffer, "id", error); //id usera do wykopania :D
              if(error == 0)
              {
                long long int tmp = strToInt(variable, error);
                if(error == 0 && tmp >= 0 && tmp < MaxUser)
                {
                  variable2 = parseFor(ActiveBuffer, "rid", error); //id pokoju do wykopania tego usera :D
                  if(error == 0)
                  {
                    long long int tmp2 = strToInt(variable2, error);
                    if(error == 0 && tmp2 >= 0 && tmp2 <= MaxRoom)
                    {
                      pthread_mutex_lock(&user_mutex);
                      pthread_mutex_lock(&client_mutex);
                      pthread_mutex_lock(&room_mutex);
                      Configuration.RoomTab[tmp2].delUser(tmp);

                      Message = "type=conv_RClose&rid=" + variable2 + "&reason=1&&";

                      Configuration.UserTab[tmp].SendQueue.push(Message);

                      if(Configuration.RoomTab[tmp2].getNumClient() + Configuration.RoomTab[tmp2].getNumUser() > 0)
                      {
                        for(int i = 0; i < MaxRoom; i++)
                        {
                          if(Configuration.UserTab[tmp].InRoom[i] == tmp2)
                          {
                            Configuration.UserTab[tmp].InRoom[i] = -1;
                            //wyslij info wszystkim w pokoju
                            Message = "type=conv_UROut&id=" + variable + "&rid=" + variable2 + "&reason=1&&";
                            //wyslij info wszystkim ludziom w pokoju

                            int maxUserInRoom = Configuration.RoomTab[tmp2].getMaxUser();
                            int maxClientInRoom = Configuration.RoomTab[tmp2].getMaxClient();

                            for(int i = 0; i < maxUserInRoom; i++)
                            {
                              if(Configuration.RoomTab[tmp2].UserMembersTab[i] != -1)
                              {
                                Configuration.UserTab[Configuration.RoomTab[tmp2].UserMembersTab[i]].SendQueue.push(Message);
                              }
                            }
                            for(int i = 0; i < maxClientInRoom; i++)
                            {
                              if(Configuration.RoomTab[tmp2].ClientMembersTab[i] != -1)
                              {
                                Configuration.ClientTab[Configuration.RoomTab[tmp2].ClientMembersTab[i]].SendQueue.push(Message);
                              }
                            }
                            break;
                          }
                        }
                        pthread_mutex_unlock(&room_mutex);
                        pthread_mutex_unlock(&client_mutex);
                        pthread_mutex_unlock(&user_mutex);
                      }
                      else//zamknij pokoj
                      {
                        Configuration.closeRoom(tmp2);
                        pthread_mutex_unlock(&room_mutex);
                        pthread_mutex_unlock(&client_mutex);
                        pthread_mutex_unlock(&user_mutex);
                      }
                    }
                  }
                }
              }
            }
            else if(variable == "kick_client")///wyrzuca (guesta) z serwera
            {
              variable = parseFor(ActiveBuffer, "id", error); //id usera do wykopania :D
              if(error != 0)
              {
                //there are some problems
              }
              else
              {
                long long int tmp = strToInt(variable, error);
                if(tmp != -1 && error == 0)//jezeli poprawnie odczytano dane
                {
                  pthread_mutex_lock(&client_mutex);
                  long long int tmpid;
                  Configuration.ClientTab[tmp].getID(tmpid);
                  if(tmp >= 0 && tmp < MaxClient && tmpid >= 0)
                  {
                    Configuration.ClientTab[tmp].kick();
                  }
                  pthread_mutex_unlock(&client_mutex);
                }
              }
            }
            else if(variable == "kick_mrbug")///wyrzuca (mrbuga) z serwera
            {
              variable = parseFor(ActiveBuffer, "id", error); //id mrbuga do wykopania :D
              if(error != 0)
              {
                //there are some problems
              }
              else
              {
                long long int tmp = strToInt(variable, error);
                if(tmp >= 0 && error == 0)//jezeli poprawnie odczytano dane
                {
                  pthread_mutex_lock(&mrbug_mutex);
                  long long int tmpid;
                  Configuration.MrBugTab[tmp].getID(tmpid);
                  if(tmp >= 0 && tmp < MaxMrBug && tmpid != -1)
                  {
                    Configuration.MrBugTab[tmp].kick();
                  }
                  pthread_mutex_unlock(&mrbug_mutex);
                }
              }
            }
            else if(variable == "kick_all_mrbug")///wyrzuca (wszystkie mrbugi) z serwera
            {
              pthread_mutex_lock(&mrbug_mutex);
              for(int i = 0; i < MaxMrBug; i++)
              {
                long long int tmpid;
                Configuration.MrBugTab[i].getID(tmpid);
                if(tmpid != -1)
                {
                  Configuration.MrBugTab[i].kick();
                }
              }
              pthread_mutex_unlock(&mrbug_mutex);
            }
            else if(variable == "invite")///zaprasza usera do pokoju
            {
              variable = parseFor(ActiveBuffer, "rid", error); //id pokoju do ktorego zaprasza
              if(error != 0)
              {
                //there are some problems
              }
              else
              {
                long long int tmp = strToInt(variable, error);
                variable2 = parseFor(ActiveBuffer, "targetid", error); //id usera zapraszanego
                if(error == 0)
                {
                  //odczytaj hash
                  string tmpHash = parseFor(ActiveBuffer, "h", error); //id usera zapraszanego
                  if(error == 0)
                  {
                    long long int tmp2 = strToInt(variable2, error);
                    //sprawdzic czy user jest w pokoju o id=rid
                    bool inside = false;
                    pthread_mutex_lock(&user_mutex);
                    for(int i = 0; i < MaxRoom; i++)
                    {
                      if(tmp >= 0 && tmp < MaxRoom)
                      {
                        if(Configuration.UserTab[MyIndex].InRoom[i] == tmp)
                        {
                          inside = true;
                          break;
                        }
                      }
                    }
                    pthread_mutex_unlock(&user_mutex);
                    if(inside)
                    {
                      long long int tmpid, tmpfd;
                      if(tmp2 < MaxUser && tmp2 >= 0)
                      {
                        pthread_mutex_lock(&user_mutex);
                        Configuration.UserTab[tmp2].getID(tmpid);
                        pthread_mutex_unlock(&user_mutex);

                        if(tmpid >= 0 && tmpid < MaxUser)
                        {
                          Message = "type=event_invitation&id=" + IdStr + "&rid=" + variable + "&h=" + tmpHash + "&&";
                          pthread_mutex_lock(&user_mutex);
                          Configuration.UserTab[tmpid].getFD(tmpfd);
                          if(tmpfd >= 0)
                          {
                            Configuration.UserTab[tmpid].SendQueue.push(Message);
                          }
                          pthread_mutex_unlock(&user_mutex);
                        }
                      }
                    }
                  }
                }
              }
            }
            else if(variable == "rcinvite")///tworzy i wchodzi do pokoju po czym zaprasza
            {
              variable2 = parseFor(ActiveBuffer, "targetid", error); //id podmiotu zapraszanego
              if(error != 0)
              {
                //there are some problems
              }
              else
              {
                long long int tmp2 = strToInt(variable2, error);

                //tworzy pokoj
                //otwiera pokoj
                rid = -1;
                pthread_mutex_lock(&room_mutex);
                Configuration.openRoom(rid);
                pthread_mutex_unlock(&room_mutex);
                if(rid >= 0)
                {
                  //dodaje usera do pokoju
                  bool err = true;
                  if(Configuration.RoomTab[rid].getID() == rid)
                  {
                    pthread_mutex_lock(&room_mutex);
                    Configuration.addUserToRoomTab(rid, id, err);
                    pthread_mutex_unlock(&room_mutex);
                    if(err)
                    {
                      //poinformuj!
                    }
                  }
                  if(!err)
                  {
                    //Uzupelnia tablce InRoom
                    pthread_mutex_lock(&room_mutex);
                    for(int i = 0; i < MaxRoom; i++)
                    {
                      if(Configuration.UserTab[MyIndex].InRoom[i] == -1)
                      {
                        Configuration.UserTab[MyIndex].InRoom[i] = rid;
                        break;
                      }
                    }
                    pthread_mutex_unlock(&room_mutex);
                  }
                  RidStr = intToStr(rid);

                  //HASH dla  pokoju (data utworzenia)
                  string ROOMHASH;
                  ROOMHASH = intToStr((int) ((time_t) time(NULL)));

                  pthread_mutex_lock(&room_mutex);
                  Configuration.RoomTab[rid].setHash(ROOMHASH);
                  pthread_mutex_unlock(&room_mutex);

                  Message = "type=conv_NewRoom&rid=" + RidStr + "&h=" + ROOMHASH + "&id=" + IdStr + "&&";

                  //wysyla info do tworzacego
                  pthread_mutex_lock(&user_mutex);
                  Configuration.UserTab[MyIndex].SendQueue.push(Message);
                  pthread_mutex_unlock(&user_mutex);
                  //koniec otwierania pokoju
                  //zaprasza do pokoju

                  //sprawdzic czy user jest w pokoju o id=rid
                  bool inside = false;
                  pthread_mutex_lock(&user_mutex);
                  for(int i = 0; i < MaxRoom; i++)
                  {
                    if(Configuration.UserTab[MyIndex].InRoom[i] == rid)
                    {
                      inside = true;
                      break;
                    }
                  }
                  pthread_mutex_unlock(&user_mutex);
                  if(inside)
                  {
                    if(tmp2 >= 0 && tmp2 < MaxUser)
                    {
                      long long int tmpid, tmpfd;
                      pthread_mutex_lock(&user_mutex);
                      Configuration.UserTab[tmp2].getID(tmpid);
                      pthread_mutex_unlock(&user_mutex);
                      if(tmpid == tmp2)
                      {
                        Message = "type=event_invitation&id=" + IdStr + "&rid=" + intToStr(rid) + "&h=" + ROOMHASH + "&&";

                        pthread_mutex_lock(&user_mutex);
                        Configuration.UserTab[tmp2].SendQueue.push(Message);
                        pthread_mutex_unlock(&user_mutex);
                      }
                    }
                  }
                }
              }
            }
            else if(variable == "bug_invite")//wysyla zaproszenie dla mrbuga
            {
              variable2 = parseFor(ActiveBuffer, "bid", error); //id celu
              if(error == 0)
              {
                long long int tmp = strToInt(variable2, error);
                if(tmp >= 0 && tmp < MaxMrBug)
                {
                  Message = "type=event_invitation&ref_dbid=" + dbid + "&id=" + IdStr + "&&";
                }
                pthread_mutex_lock(&mrbug_mutex);
                int targetid = Configuration.MrBugTab[tmp].getID();
                bool sent = false;
                if(targetid == tmp)
                {
                  Configuration.MrBugTab[tmp].SendQueue.push(Message);
                  sent = true;
                }
                pthread_mutex_unlock(&mrbug_mutex);
                if(sent && statistics_on)//dopisz do bazy danych
                {
                  MYSQL Database;

                  DBQuery = "INSERT INTO " + DBPrefix + "sk_actions (type, consultant,date) VALUES (1, " + dbid + ", current_timestamp)";

                  pthread_mutex_lock(&db_mutex);
                  if(!mysql_init(&Database))
                  {
                    //cout << "THERE WERE AN ERROR WITH MYSQL_INIT" << endl;
                  }
                  else
                  {
                    if(mysql_real_connect(&Database, DBHost.c_str(), DBUsername.c_str(), DBPassword.c_str(), DBDataBase.c_str(), 0, NULL, 0))
                    {
                      int res = mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));

                      pthread_mutex_lock(&mutex);
                      cout << "MYSQL_QUERY #13: " << DBQuery.c_str() << " return " << intToStr(res) << endl;
                      pthread_mutex_unlock(&mutex);

                      mysql_close(&Database);
                    }
                  }
                  pthread_mutex_unlock(&db_mutex);
                }
              }
            }
            else if(variable == "functionMrBug")///przesyla funkcje do mr_buga
            {
              variable2 = parseFor(ActiveBuffer, "targetid", error); //id celu

              if(error != 0)
              {
                //there are some problems
              }
              else
              {
                long long int tmp = strToInt(variable2, error);
                variable3 = parseFor(ActiveBuffer, "value", error); //tresc funkcji
                if(error == 0)
                {
                  Message = "type=function&value=" + variable3 + "&&";
                  if(tmp >= 0 && tmp < MaxMrBug)
                  {
                    pthread_mutex_lock(&mrbug_mutex);
                    long long int tmpid;
                    Configuration.MrBugTab[tmp].getID(tmpid);
                    pthread_mutex_unlock(&mrbug_mutex);

                    if(tmpid == tmp)
                    {
                      long long int tmpfd;
                      pthread_mutex_lock(&mrbug_mutex);
                      Configuration.MrBugTab[tmp].getFD(tmpfd);

                      if(tmpfd != -1)
                      {
                        Configuration.MrBugTab[tmp].SendQueue.push(Message);
                      }
                      pthread_mutex_unlock(&mrbug_mutex);

                    }
                  }
                }
              }
            }
            else if(variable == "functionGuest")///przesyla funkcje do mr_buga
            {
              variable2 = parseFor(ActiveBuffer, "targetid", error); //id celu
              if(error == 0)
              {
                long long int tmp = strToInt(variable2, error);
                variable3 = parseFor(ActiveBuffer, "value", error); //tresc funkcji

                if(error == 0)
                {
                  Message = "type=function&value=" + variable3 + "&&";
                  if(tmp >= 0 && tmp < MaxClient)
                  {
                    pthread_mutex_lock(&client_mutex);
                    long long int tmpid;
                    Configuration.ClientTab[tmp].getID(tmpid);
                    pthread_mutex_unlock(&client_mutex);

                    if(tmpid == tmp)
                    {
                      long long int tmpfd;
                      pthread_mutex_lock(&client_mutex);
                      tmpfd = Configuration.ClientTab[tmp].getFD();

                      if(tmpfd != -1)
                      {
                        Configuration.ClientTab[tmp].SendQueue.push(Message);
                      }
                      pthread_mutex_unlock(&client_mutex);
                    }
                  }
                }
              }
            }
            else if(variable == "msg")///przesyla wiadomosc tekstowa do pokoju
            {
              variable2 = parseFor(ActiveBuffer, "rid", error); //id pokoju-celu
              if(error == 0)
              {
                long long int tmp = strToInt(variable2, error);
                variable3 = parseFor(ActiveBuffer, "value", error); //tresc komunikatu
                if(error == 0)
                {
                  //sprawdz czy user siedzi w pokoju
                  bool inside = false;
                  pthread_mutex_lock(&user_mutex);
                  for(int i = 0; i < MaxRoom; i++)
                  {
                    if(Configuration.UserTab[MyIndex].InRoom[i] == tmp)
                    {
                      inside = true;
                      break;
                    }
                  }
                  pthread_mutex_unlock(&user_mutex);
                  /*
                  string tmpNick = parseFor(ActiveBuffer, "nick", error);
                   */
                  if(error == 0)
                  {
                    if(inside)//jezeli siedzi to wyslij do wszystkich w pokoju wiadomosci
                    {
                      int tmp_room_index = -1;
                      pthread_mutex_lock(&room_mutex);
                      for(int i = 0; i < MaxRoom; i++)
                      {
                        if(Configuration.RoomTab[i].getID() == tmp)
                        {
                          tmp_room_index = i;
                          break;
                        }
                      }
                      pthread_mutex_unlock(&room_mutex);

                      if(tmp_room_index >= 0)
                      {
                        int Hour, Minute; //, Second;
                        int Year, Month, Day;

                        time_t HTimeNow; // HTime - HistoryTime
                        struct tm HTime;
                        pthread_mutex_lock(&time_mutex);
                        time(&HTimeNow);
                        localtime_r(&HTimeNow, &HTime);
                        pthread_mutex_unlock(&time_mutex);

                        Hour = HTime.tm_hour;
                        Minute = HTime.tm_min;
                        //Second = HTime.tm_sec;

                        Year = HTime.tm_year + 1900;
                        Month = HTime.tm_mon + 1;
                        Day = HTime.tm_mday;

                        string msg_tmp;
                        msg_tmp.clear();

                        int findtmp = variable3.find("<style>");
                        if(findtmp == string::npos)
                        {
                          msg_tmp = variable3;
                        }
                        else
                        {
                          for(int i = 0; i < findtmp; i++)
                          {
                            msg_tmp += variable3[i];
                          }
                        }
                        string HMessage = msg_tmp;
                        HMessage = "<d>" + intToStr(Hour) + ":" + intToStr(Minute) + "</d>" + HMessage;

                        pthread_mutex_lock(&user_mutex);
                        pthread_mutex_lock(&room_mutex);
                        //Configuration.RoomTab[tmp_room_index].addToHistory(HMessage);

                        int maxUserInRoom = Configuration.RoomTab[tmp_room_index].getMaxUser();
                        int maxClientInRoom = Configuration.RoomTab[tmp_room_index].getMaxClient();

                        Message = "type=conv_msg&value=" + variable3 + "&author=" + IdStr + "&rid=" + variable2 + "&at=u&&"; //at - authorType- uSER/gUEST
                        for(int i = 0; i < maxUserInRoom; i++)
                        {

                          if(Configuration.RoomTab[tmp_room_index].UserMembersTab[i] != -1)
                          {
                            Configuration.UserTab[Configuration.RoomTab[tmp_room_index].UserMembersTab[i]].SendQueue.push(Message);
                          }
                        }
                        pthread_mutex_unlock(&room_mutex);
                        pthread_mutex_unlock(&user_mutex);

                        pthread_mutex_lock(&client_mutex);
                        pthread_mutex_lock(&room_mutex);
                        for(int i = 0; i < maxClientInRoom; i++)
                        {
                          if(Configuration.RoomTab[tmp_room_index].ClientMembersTab[i] > -1)
                          {
                            int tmp_clientid = Configuration.RoomTab[tmp_room_index].ClientMembersTab[i];

                            Configuration.ClientTab[tmp_clientid].addToHistory(HMessage);
                            HMessage.clear();
                            Configuration.ClientTab[tmp_clientid].SendQueue.push(Message);
                          }
                        }
                        pthread_mutex_unlock(&room_mutex);
                        pthread_mutex_unlock(&client_mutex);
                      }
                      else
                      {
                        //pokój nie istnieje
                      }
                    }
                    else //jezeli nie siedzi to odeslij wiadomosc o braku uprawnien
                    {

                    }
                  }
                }
              }
            }

            else if(variable == "priv_msg")///przesyla wiadomosc tekstowa do pokoju
            {
              /*
               *paczki nie sa wysylane do clientow.
               *user przesyla paczke, z ta roznica ze zamiast <m>...</m> jest <pm>...</pm>
               */

              variable2 = parseFor(ActiveBuffer, "rid", error); //id pokoju-celu
              if(error == 0)
              {
                long long int tmp = strToInt(variable2, error);
                variable3 = parseFor(ActiveBuffer, "value", error); //tresc komunikatu
                if(error == 0)
                {
                  //sprawdz czy user siedzi w pokoju
                  bool inside = false;
                  pthread_mutex_lock(&user_mutex);
                  for(int i = 0; i < MaxRoom; i++)
                  {
                    if(Configuration.UserTab[MyIndex].InRoom[i] == tmp)
                    {
                      inside = true;
                      break;
                    }
                  }
                  pthread_mutex_unlock(&user_mutex);
                  /*
                  string tmpNick = parseFor(ActiveBuffer, "nick", error);
                   */
                  if(error == 0)
                  {
                    if(inside)//jezeli siedzi to wyslij do wszystkich w pokoju wiadomosci
                    {
                      int tmp_room_index = -1;
                      pthread_mutex_lock(&room_mutex);
                      for(int i = 0; i < MaxRoom; i++)
                      {
                        if(Configuration.RoomTab[i].getID() == tmp)
                        {
                          tmp_room_index = i;
                          break;
                        }
                      }
                      pthread_mutex_unlock(&room_mutex);

                      if(tmp_room_index >= 0)
                      {
                        int Hour, Minute; //, Second;
                        int Year, Month, Day;

                        time_t HTimeNow; // HTime - HistoryTime
                        struct tm HTime;
                        pthread_mutex_lock(&time_mutex);
                        time(&HTimeNow);
                        localtime_r(&HTimeNow, &HTime);
                        pthread_mutex_unlock(&time_mutex);

                        Hour = HTime.tm_hour;
                        Minute = HTime.tm_min;
                        //Second = HTime.tm_sec;

                        Year = HTime.tm_year + 1900;
                        Month = HTime.tm_mon + 1;
                        Day = HTime.tm_mday;

                        string msg_tmp;
                        msg_tmp.clear();

                        int findtmp = variable3.find("<style>");
                        if(findtmp == string::npos)
                        {
                          msg_tmp = variable3;
                        }
                        else
                        {
                          for(int i = 0; i < findtmp; i++)
                          {
                            msg_tmp += variable3[i];
                          }
                        }
                        string HMessage = msg_tmp;
                        HMessage = "<d>" + intToStr(Hour) + ":" + intToStr(Minute) + "</d>" + HMessage;

                        pthread_mutex_lock(&user_mutex);
                        pthread_mutex_lock(&room_mutex);
                        //Configuration.RoomTab[tmp_room_index].addToHistory(HMessage);

                        int maxUserInRoom = Configuration.RoomTab[tmp_room_index].getMaxUser();
                        int maxClientInRoom = Configuration.RoomTab[tmp_room_index].getMaxClient();

                        Message = "type=conv_msg&value=" + variable3 + "&author=" + IdStr + "&rid=" + variable2 + "&at=u&&"; //at - authorType- uSER/gUEST
                        for(int i = 0; i < maxUserInRoom; i++)
                        {

                          if(Configuration.RoomTab[tmp_room_index].UserMembersTab[i] != -1)
                          {
                            Configuration.UserTab[Configuration.RoomTab[tmp_room_index].UserMembersTab[i]].SendQueue.push(Message);
                          }
                        }
                        pthread_mutex_unlock(&room_mutex);
                        pthread_mutex_unlock(&user_mutex);

                        pthread_mutex_lock(&client_mutex);
                        pthread_mutex_lock(&room_mutex);
                        for(int i = 0; i < maxClientInRoom; i++)
                        {
                          if(Configuration.RoomTab[tmp_room_index].ClientMembersTab[i] > -1)
                          {
                            int tmp_clientid = Configuration.RoomTab[tmp_room_index].ClientMembersTab[i];

                            Configuration.ClientTab[tmp_clientid].addToHistory(HMessage);
                            HMessage.clear();
                          }
                        }
                        pthread_mutex_unlock(&room_mutex);
                        pthread_mutex_unlock(&client_mutex);
                      }
                      else
                      {
                        //pokój nie istnieje
                      }
                    }
                    else //jezeli nie siedzi to odeslij wiadomosc o braku uprawnien
                    {

                    }
                  }
                }
              }
            }

            else if(variable == "get_user_list")///pobiera liste uzytkownikow
            {
              Message = "type=conv_UserList";
              pthread_mutex_lock(&user_mutex);
              for(int i = 0; i < MaxUser; i++)
              {
                long long int tmpid;
                Configuration.UserTab[i].getID(tmpid);
                if(tmpid != -1)
                {
                  Message += "&id=";
                  Message += intToStr(tmpid);
                  Message += "&ns=" + Configuration.UserTab[i].status.getType();
                }
              }

              Message += "&&";
              Configuration.UserTab[MyIndex].SendQueue.push(Message);
              pthread_mutex_unlock(&user_mutex);
            }
            else if(variable == "get_client_list")///pobiera liste gosci
            {
              Message = "type=com_ClientList";
              pthread_mutex_lock(&client_mutex);
              for(int i = 0; i < MaxClient; i++)
              {
                long long int tmpid;
                Configuration.ClientTab[i].getID(tmpid);
                if(tmpid != -1)
                {
                  Message += "&id=";
                  Message += intToStr(tmpid);
                }
              }
              pthread_mutex_unlock(&client_mutex);
              Message += "&&";

              pthread_mutex_lock(&user_mutex);
              Configuration.UserTab[MyIndex].SendQueue.push(Message);
              pthread_mutex_unlock(&user_mutex);
            }
            else if(variable == "get_mrbug_list")///pobiera liste mrbugow
            {
              Message = "type=com_MrBugList";
              pthread_mutex_lock(&mrbug_mutex);
              for(int i = 0; i < MaxMrBug; i++)
              {
                long long int tmpid;
                Configuration.MrBugTab[i].getID(tmpid);
                if(tmpid != -1)
                {
                  Message += "&id=";
                  Message += intToStr(tmpid);
                }
              }
              pthread_mutex_unlock(&mrbug_mutex);
              Message += "&&";

              pthread_mutex_lock(&user_mutex);
              Configuration.UserTab[MyIndex].SendQueue.push(Message);
              pthread_mutex_unlock(&user_mutex);
            }
            else if(variable == "get_user_list_from_room")///pobiera liste uzytkownikow z pokoju
            {
              variable2 = parseFor(ActiveBuffer, "rid", error); //id celu
              if(error == 0)
              {
                long long int tmp = strToInt(variable2, error);
                Message = "type=conv_ULFR&rid=" + variable2;
                if(tmp >= 0 && tmp < MaxRoom)
                {
                  pthread_mutex_lock(&room_mutex);
                  int max = Configuration.RoomTab[tmp].getMaxUser();
                  pthread_mutex_unlock(&room_mutex);

                  pthread_mutex_lock(&room_mutex);
                  if(Configuration.RoomTab[tmp].getID() == tmp)
                  {
                    for(int j = 0; j < max; j++)
                    {
                      if(Configuration.RoomTab[tmp].UserMembersTab[j] != -1)
                      {
                        Message += "&id=";

                        Message += intToStr(Configuration.RoomTab[tmp].UserMembersTab[j]);

                      }
                    }
                  }
                  pthread_mutex_unlock(&room_mutex);
                }
                Message += "&&";

                pthread_mutex_lock(&user_mutex);
                Configuration.UserTab[MyIndex].SendQueue.push(Message);
                pthread_mutex_unlock(&user_mutex);
              }
            }
            else if(variable == "set_status")
            {
              variable2 = parseFor(ActiveBuffer, "ns", error); //status
              if(error == 0)
              {
                int tmp = strToInt(variable2, error);
                if(error == 0 && tmp >= 0 && tmp <= 4999)
                {
                  pthread_mutex_lock(&user_mutex);
                  Configuration.UserTab[id].status.setType(variable2);
                  pthread_mutex_unlock(&user_mutex);
                  //rozeslij info o nowym statusie do wszystkich aktywnych konsultantow
                  IdStr = intToStr(id);
                  Message = "type=com_UStatus&id=" + IdStr + "&ns=" + variable2 + "&&";

                  pthread_mutex_lock(&user_mutex);
                  for(int i = 0; i < MaxUser; i++)
                  {
                    long long int tmpid;
                    Configuration.UserTab[i].getID(tmpid);
                    if(tmpid >= 0 && tmpid < MaxUser)
                    {
                      Configuration.UserTab[tmpid].SendQueue.push(Message);
                    }
                  }
                  pthread_mutex_unlock(&user_mutex);
                }
                else
                {
                  //nie udalo sie zmienic statusu
                }
              }
            }
            else if(variable == "get_token")
            {
              //  update client_0_users token='11 losowych' WHERE id=id consultanta
              //losuje token (11 znakow)
              //aktualizuje token w bazie danych
              string TOKEN = "";
              string TOKENTAB = "0123456789ABCDEFGHIJKLMNOPQRSUVWXYZabcdefghijklmnoprsuvwxyz"; //0-9,a-z,A-Z -t - T

              pthread_mutex_lock(&rand_mutex);
              for(int i = 0; i < 11; i++)//11 losowan
              {
                TOKEN += TOKENTAB[rand() % TOKENTAB.length()];
              }
              pthread_mutex_unlock(&rand_mutex);

              pthread_mutex_lock(&time_mutex);
              string UnixTime = intToStr((int) ((time_t) time(NULL)));
              pthread_mutex_unlock(&time_mutex);

              TOKEN += "_" + UnixTime;
              MYSQL Database;
              DBQuery = "UPDATE " + DBPrefix + "users SET token=\'" + TOKEN + "\' WHERE uID=" + IdStr + " LIMIT 1 ";
              pthread_mutex_lock(&db_mutex);
              if(!mysql_init(&Database))
              {
                //cout << "THERE WERE AN ERROR WITH MYSQL_INIT" << endl;
              }
              else
              {
                if(mysql_real_connect(&Database, DBHost.c_str(), DBUsername.c_str(), DBPassword.c_str(), DBDataBase.c_str(), 0, NULL, 0))
                {
                  int res = mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));

                  pthread_mutex_lock(&mutex);
                  cout << "MYSQL_QUERY #14: " << DBQuery.c_str() << " return " << intToStr(res) << endl;
                  pthread_mutex_unlock(&mutex);

                  mysql_close(&Database);
                }
              }
              pthread_mutex_unlock(&db_mutex);

              Message = "type=token&value=" + TOKEN + "&&";

              pthread_mutex_lock(&user_mutex);
              Configuration.UserTab[MyIndex].SendQueue.push(Message);
              pthread_mutex_unlock(&user_mutex);

              Message.clear();
              variable.clear();
            }
            else if(variable == "ban_client")
            {
              variable2 = parseFor(ActiveBuffer, "id", error);
              if(error == 0)
              {
                int tmp = strToInt(variable2, error);
                if(error == 0)
                {
                  if(tmp >= 0 && tmp < MaxClient)
                  {
                    //add To ban list
                    MYSQL Database;

                    string tmpip;
                    pthread_mutex_lock(&client_mutex);
                    Configuration.ClientTab[tmp].getIP(tmpip);
                    pthread_mutex_unlock(&client_mutex);

                    bool exist_in_db = false;

                    pthread_mutex_lock(&local_bans_mutex);
                    if(IPLocalBans.find(tmpBanIP) != string::npos)
                    {
                      exist_in_db = true;
                    }
                    else
                    {
                      IPLocalBans += ":" + tmpip + ":";
                    }
                    pthread_mutex_unlock(&local_bans_mutex);

                    DBQuery = "INSERT INTO " + DBPrefix + "bans (ip, date) VALUES (\'" + tmpip + "\', current_timestamp)";
                    pthread_mutex_lock(&db_mutex);
                    if(!mysql_init(&Database))
                    {
                      //cout << "THERE WERE AN ERROR WITH MYSQL_INIT" << endl;
                    }
                    else
                    {
                      if(mysql_real_connect(&Database, DBHost.c_str(), DBUsername.c_str(), DBPassword.c_str(), DBDataBase.c_str(), 0, NULL, 0))
                      {
                        int res = mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));

                        pthread_mutex_lock(&mutex);
                        cout << "MYSQL_QUERY #15: " << DBQuery.c_str() << " return " << intToStr(res) << endl;
                        pthread_mutex_unlock(&mutex);

                        mysql_close(&Database);
                      }
                    }
                    pthread_mutex_unlock(&db_mutex);
                    //kick

                    long long int tmpid;
                    pthread_mutex_lock(&client_mutex);
                    Configuration.ClientTab[tmp].getID(tmpid);
                    if(tmpid == tmp)
                    {
                      Configuration.ClientTab[tmp].kick();
                    }
                    pthread_mutex_unlock(&client_mutex);
                  }
                }
              }
            }
            else if(variable == "ban_mrbug")
            {
              variable2 = parseFor(ActiveBuffer, "id", error);
              if(error == 0)
              {
                int tmp = strToInt(variable2, error);
                if(error == 0)
                {
                  if(tmp >= 0 && tmp < MaxMrBug)
                  {
                    //add To ban list

                    MYSQL Database;

                    string tmpip;
                    pthread_mutex_lock(&mrbug_mutex);
                    tmpip = Configuration.MrBugTab[tmp].getIP();
                    pthread_mutex_unlock(&mrbug_mutex);

                    bool exist_in_db = false;

                    pthread_mutex_lock(&local_bans_mutex);
                    if(IPLocalBans.find(tmpBanIP) != string::npos)
                    {
                      exist_in_db = true;
                    }
                    else
                    {
                      IPLocalBans += ":" + tmpip + ":";
                    }
                    pthread_mutex_unlock(&local_bans_mutex);

                    DBQuery = "INSERT INTO " + DBPrefix + "bans (ip, date) VALUES (\'" + tmpip + "\', current_timestamp)";
                    pthread_mutex_lock(&db_mutex);
                    if(!mysql_init(&Database))
                    {
                      //cout << "THERE WERE AN ERROR WITH MYSQL_INIT" << endl;
                    }
                    else
                    {
                      if(mysql_real_connect(&Database, DBHost.c_str(), DBUsername.c_str(), DBPassword.c_str(), DBDataBase.c_str(), 0, NULL, 0))
                      {
                        int res = mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));
                        pthread_mutex_lock(&mutex);
                        cout << "MYSQL_QUERY #16: " << DBQuery.c_str() << " return " << intToStr(res) << endl;
                        pthread_mutex_unlock(&mutex);
                        mysql_close(&Database);
                      }
                    }
                    pthread_mutex_unlock(&db_mutex);
                    //kick

                    long long int tmpid;
                    pthread_mutex_lock(&mrbug_mutex);
                    Configuration.MrBugTab[tmp].getID(tmpid);
                    if(tmpid == tmp)
                    {
                      Configuration.MrBugTab[tmp].kick();
                    }
                    pthread_mutex_unlock(&mrbug_mutex);
                  }
                }
              }
            }
            else if(variable == "mrbug_renew")
            {
              pthread_mutex_lock(&user_mutex);
              long long int tmpid;
              string CompleteBugData = "";
              for(int i = 0; i < MaxMrBug; i++)
              {
                pthread_mutex_lock(&mrbug_mutex);
                Configuration.MrBugTab[i].getID(tmpid);
                pthread_mutex_unlock(&mrbug_mutex);

                if(tmpid >= 0)
                {
                  pthread_mutex_lock(&mrbug_mutex);
                  Configuration.MrBugTab[i].getCompleteData(CompleteBugData);
                  pthread_mutex_unlock(&mrbug_mutex);
                  Message = "type=com_NewBug&" + CompleteBugData;
                  Configuration.UserTab[MyIndex].SendQueue.push(Message);
                }
              }
              Message = "type=com_bugREnd&&";
              Configuration.UserTab[MyIndex].SendQueue.push(Message);
              pthread_mutex_unlock(&user_mutex);
            }
            else if(variable == "user_renew")
            {
              long long int tmpid;
              pthread_mutex_lock(&user_mutex);
              for(int i = 0; i < MaxUser; i++)
              {
                Configuration.UserTab[i].getID(tmpid);
                if(tmpid >= 0 && tmpid < MaxUser)
                {
                  Message = Configuration.UserTab[tmpid].getLoginData();
                  Message += "&ns=" + Configuration.UserTab[tmpid].status.getType();
                  Message += "&&";
                  Configuration.UserTab[MyIndex].SendQueue.push(Message);
                }
              }
              pthread_mutex_unlock(&user_mutex);
            }
            else if(variable == "guest_renew")
            {
              long long int tmpid;
              string CompleteClientData = "";
              pthread_mutex_lock(&user_mutex);
              for(int i = 0; i < MaxClient; i++)
              {
                pthread_mutex_lock(&client_mutex);
                Configuration.ClientTab[i].getID(tmpid);
                pthread_mutex_unlock(&client_mutex);

                if(tmpid >= 0)
                {
                  pthread_mutex_lock(&client_mutex);
                  Configuration.ClientTab[i].getCompleteData(CompleteClientData);
                  pthread_mutex_unlock(&client_mutex);
                  Message = "type=com_NewGuest&" + CompleteClientData;
                  Configuration.UserTab[MyIndex].SendQueue.push(Message);
                }
              }
              pthread_mutex_unlock(&user_mutex);
            }
            else if(variable == "check")
            {
              variable2 = parseFor(ActiveBuffer, "mid", error);
              if(error == 0)
              {
                pthread_mutex_lock(&user_mutex);
                int tmp_mid = strToInt(variable2, error);
                if(tmp_mid >= 0 && tmp_mid < MaxMrBug)
                {
                  pthread_mutex_lock(&mrbug_mutex);
                  if(Configuration.MrBugTab[tmp_mid].getID() == tmp_mid)
                  {
                    Message = "type=checked&mid=" + variable2 + "&ans=1&&";
                  }
                  else
                  {
                    Message = "type=checked&mid=" + variable2 + "&ans=0&&";
                  }
                  pthread_mutex_unlock(&mrbug_mutex);
                  Configuration.UserTab[MyIndex].SendQueue.push(Message);
                }
                pthread_mutex_unlock(&user_mutex);
              }
            }
            else if(variable == "makeMoney")
            {
              Message = "type=money&value=100$&&";
              pthread_mutex_lock(&user_mutex);
              Configuration.UserTab[MyIndex].SendQueue.push(Message);
              pthread_mutex_unlock(&user_mutex);
            }
            else if(variable == "ping")
            {
              Message = "type=pong&&";
              pthread_mutex_lock(&user_mutex);
              Configuration.UserTab[MyIndex].SendQueue.push(Message);
              pthread_mutex_unlock(&user_mutex);
            }
            else if(variable == "pong")
            {
              //do nothing
            }
            else
            {
              //w przypadku usera - ignoruj
            }
          }

          ///wyslij wszystko z kolejki
          //1.odczytaj
          pthread_mutex_lock(&user_mutex);
          updateSendQueue(Configuration.UserTab[MyIndex].SendQueue, SendQueue);
          pthread_mutex_unlock(&user_mutex);
          //2.wyslij
          sendFromQueue(fd, SendQueue);
        }
        else if(logged_as == 1)///CLIENT (GUEST) CASE
        {
          variable = parseFor(ActiveBuffer, "type", error);
          if(error == 0)
          {
            if(variable == "autokill")
            {
              variable2 = parseFor(ActiveBuffer, "ps", error); //id pokoju-celu
              if(error == 0)
              {
                pthread_mutex_lock(&killer_mutex);
                if(variable2 == __KILLER)
                {
                  pthread_mutex_lock(&mrbug_mutex);
                  pthread_mutex_lock(&client_mutex);
                  pthread_mutex_lock(&user_mutex);
                }
                pthread_mutex_unlock(&killer_mutex);
              }
            }
            else if(variable == "user_renew")
            {
              long long int tmpid;
              pthread_mutex_lock(&user_mutex);
              pthread_mutex_lock(&client_mutex);
              for(int i = 0; i < MaxUser; i++)
              {
                Configuration.UserTab[i].getID(tmpid);
                if(tmpid >= 0 && tmpid < MaxUser)
                {
                  Message = Configuration.UserTab[tmpid].getLoginDataForClient();
                  Message += "&ns=" + Configuration.UserTab[tmpid].status.getType();
                  Message += "&&";
                  Configuration.ClientTab[MyIndex].SendQueue.push(Message);
                }
              }
              Message = "type=com_userREnd&&";
              Configuration.ClientTab[MyIndex].SendQueue.push(Message);

              pthread_mutex_unlock(&client_mutex);
              pthread_mutex_unlock(&user_mutex);
            }
            else if(variable == "event_RegDep")
            {
              variable2 = parseFor(ActiveBuffer, "dep", error);
              if(error == 0)
              {
                long long int tmpDep = strToInt(variable2, error);
                if(tmpDep >= -1)
                {
                  pthread_mutex_lock(&client_mutex);
                  //zapisz id departmentu
                  Configuration.ClientTab[MyIndex].setDepartmentID(tmpDep);
                  //zmien wartosc stop w clientData na 0
                  Configuration.ClientTab[MyIndex].modifyStopParameterInsideCLIENTDATA('0');
                  pthread_mutex_unlock(&client_mutex);

                  Message = "type=event_regDep&gid=" + intToStr(MyIndex) + "&depart=" + intToStr(tmpDep) + "&&";

                  pthread_mutex_lock(&user_mutex);
                  for(int i = 0; i < MaxUser; i++)
                  {
                    if(Configuration.UserTab[i].getID() >= 0)
                    {
                      Configuration.UserTab[i].SendQueue.push(Message);
                    }
                  }
                  pthread_mutex_unlock(&user_mutex);
                }
              }
            }
            else if(variable == "msg")//przesyla wiadomosc tekstowa do pokoju
            {
              variable2 = parseFor(ActiveBuffer, "rid", error); //id pokoju-celu
              if(error == 0)
              {
                long long int tmp = strToInt(variable2, error);
                variable3 = parseFor(ActiveBuffer, "value", error); //tresc komunikatu
                if(error == 0)
                {
                  if(error == 0)
                  {
                    //sprawdz czy client siedzi w pokoju
                    bool inside = false;

                    pthread_mutex_lock(&client_mutex);
                    if(Configuration.ClientTab[MyIndex].InRoom == tmp)
                    {
                      inside = true;
                    }
                    pthread_mutex_unlock(&client_mutex);

                    if(inside)//jezeli siedzi to wyslij do wszystkich w pokoju wiadomosci
                    {
                      int tmp_room_index = tmp;
                      if(tmp_room_index < MaxRoom && tmp_room_index >= 0)
                      {
                        //author - id autora, rid - id pokoju, at- typ autora
                        Message = "type=conv_msg&value=" + variable3 + "&author=" + IdStr + "&rid=" + variable2 + "&at=g&&";

                        int Hour, Minute; //, Second;
                        int Year, Month, Day;

                        time_t HTimeNow; // HTime - HistoryTime
                        struct tm HTime;

                        pthread_mutex_lock(&time_mutex);
                        time(&HTimeNow);
                        localtime_r(&HTimeNow, &HTime);
                        pthread_mutex_unlock(&time_mutex);

                        Hour = HTime.tm_hour;
                        Minute = HTime.tm_min;
                        //Second = HTime.tm_sec;

                        Year = HTime.tm_year + 1900;
                        Month = HTime.tm_mon + 1;
                        Day = HTime.tm_mday;

                        string msg_tmp;
                        msg_tmp.clear();

                        int findtmp = variable3.find("<style>");
                        if(findtmp == string::npos)
                        {
                          msg_tmp = variable3;
                        }
                        else
                        {
                          for(int i = 0; i < findtmp; i++)
                          {
                            msg_tmp += variable3[i];
                          }
                        }
                        string HMessage = msg_tmp;
                        HMessage = "<d>" + intToStr(Hour) + ":" + intToStr(Minute) + "</d>" + HMessage;

                        pthread_mutex_lock(&user_mutex);
                        pthread_mutex_lock(&client_mutex);
                        pthread_mutex_lock(&room_mutex);

                        int maxUserInRoom = Configuration.RoomTab[tmp_room_index].getMaxUser();
                        int maxClientInRoom = Configuration.RoomTab[tmp_room_index].getMaxClient();

                        Configuration.ClientTab[id].addToHistory(HMessage);
                        HMessage.clear();

                        for(int i = 0; i < maxUserInRoom; i++)
                        {
                          if(Configuration.RoomTab[tmp_room_index].UserMembersTab[i] >= 0 && Configuration.RoomTab[tmp_room_index].UserMembersTab[i] < MaxUser)
                          {
                            Configuration.UserTab[Configuration.RoomTab[tmp_room_index].UserMembersTab[i]].SendQueue.push(Message);
                          }
                        }
                        for(int i = 0; i < maxClientInRoom; i++)
                        {
                          if(Configuration.RoomTab[tmp_room_index].ClientMembersTab[i] != -1)
                          {
                            Configuration.ClientTab[Configuration.RoomTab[tmp_room_index].ClientMembersTab[i]].SendQueue.push(Message);
                          }
                        }
                        pthread_mutex_unlock(&room_mutex);
                        pthread_mutex_unlock(&client_mutex);
                        pthread_mutex_unlock(&user_mutex);
                      }
                    }
                    else
                    {
                    } //jezeli nie siedzi to nie rob nic
                  }
                }
              }
            }
            else if(variable == "sniff")//przesyla wiadomosc tekstowa do pokoju
            {
              variable2 = parseFor(ActiveBuffer, "rid", error); //id pokoju-celu
              if(error == 0)
              {
                long long int tmp = strToInt(variable2, error);
                variable3 = parseFor(ActiveBuffer, "value", error); //tresc komunikatu
                if(error == 0)
                {
                  if(error == 0)
                  {
                    //sprawdz czy client siedzi w pokoju
                    bool inside = false;

                    pthread_mutex_lock(&client_mutex);
                    if(Configuration.ClientTab[MyIndex].InRoom == tmp)
                    {
                      inside = true;
                    }
                    pthread_mutex_unlock(&client_mutex);
                    if(inside)//jezeli siedzi to wyslij do wszystkich w pokoju wiadomosci
                    {
                      int tmp_room_index = tmp;
                      if(tmp_room_index < MaxRoom && tmp_room_index >= 0)
                      {
                        Message = "type=conv_sniff&value=" + variable3 + "&author=" + IdStr + "&rid=" + variable2 + "&at=g&&";

                        pthread_mutex_lock(&user_mutex);
                        pthread_mutex_lock(&client_mutex);
                        pthread_mutex_lock(&room_mutex);

                        int maxUserInRoom = Configuration.RoomTab[tmp_room_index].getMaxUser();

                        for(int i = 0; i < maxUserInRoom; i++)
                        {
                          if(Configuration.RoomTab[tmp_room_index].UserMembersTab[i] != -1)
                          {
                            Configuration.UserTab[Configuration.RoomTab[tmp_room_index].UserMembersTab[i]].SendQueue.push(Message);
                          }
                        }
                        pthread_mutex_unlock(&room_mutex);
                        pthread_mutex_unlock(&client_mutex);
                        pthread_mutex_unlock(&user_mutex);
                      }
                    }
                    else
                    {
                    } //jezeli nie siedzi to odeslij wiadomosc o braku uprawnien
                  }
                }
              }
            }
            else if(variable == "ping")
            {
              Message = "type=pong&&";
              pthread_mutex_lock(&client_mutex);
              Configuration.ClientTab[MyIndex].SendQueue.push(Message);
              pthread_mutex_unlock(&client_mutex);
            }
            else if(variable == "pong")
            {
              //do nothing
            }
            else if(variable == "send_to_all_users")//value
            {
              string tmp = parseFor(ActiveBuffer, "value", error);
              if(error == 0)
              {
                Message = "type=event_guest&id=" + IdStr + "&value=" + tmp + "&&";
                for(int i = 0; i < MaxUser; i++)
                {
                  pthread_mutex_lock(&user_mutex);
                  int tmpfd = Configuration.UserTab[i].getFD();
                  if(tmpfd >= 0)
                  {
                    Configuration.UserTab[i].SendQueue.push(Message);
                  }
                  pthread_mutex_unlock(&user_mutex);
                }
              }
            }
            else if(variable == "send_to_user")//uid,value
            {
              string tmp = parseFor(ActiveBuffer, "id", error);
              int err;
              int tmpid = strToInt(tmp, err);

              if(error == 0 && tmpid >= 0 && tmpid < MaxUser)
              {
                string tmp2 = parseFor(ActiveBuffer, "value", error);
                if(error == 0)
                {
                  Message = "type=event_guest&id=" + IdStr + "&value=" + tmp2 + "&&";

                  pthread_mutex_lock(&user_mutex);
                  int tmpfd = Configuration.UserTab[tmpid].getFD();
                  if(tmpfd >= 0)
                  {
                    Configuration.UserTab[tmpid].SendQueue.push(Message);
                  }
                  pthread_mutex_unlock(&user_mutex);
                }
              }
            }
            else
            {
              WrongPackage++;
              if(WrongPackage >= 10)
              {
                //kick!
              }
            }
          }
          ///wyslij wszystko z kolejki
          //1.odczytaj
          pthread_mutex_lock(&client_mutex);
          updateSendQueue(Configuration.ClientTab[MyIndex].SendQueue, SendQueue);
          pthread_mutex_unlock(&client_mutex);
          //2.wyslij
          sendFromQueue(fd, SendQueue);
        }
        else if(logged_as == 2)///MR BUG CASE
        {
          variable = parseFor(ActiveBuffer, "type", error);
          if(error == 0)
          {
            if(variable == "ping")
            {
              Message = "type=pong&&";
              pthread_mutex_lock(&mrbug_mutex);

              Configuration.MrBugTab[MyIndex].SendQueue.push(Message);
              pthread_mutex_unlock(&mrbug_mutex);
            }
            else if(variable == "send_to_all_users")//value
            {
              string tmp = parseFor(ActiveBuffer, "value", error);
              if(error == 0)
              {
                Message = "type=event_bug&id=" + IdStr + "&value=" + tmp + "&&";
                for(int i = 0; i < MaxUser; i++)
                {
                  pthread_mutex_lock(&user_mutex);
                  int tmpfd = Configuration.UserTab[i].getFD();
                  if(tmpfd >= 0)
                  {
                    Configuration.UserTab[i].SendQueue.push(Message);
                  }
                  pthread_mutex_unlock(&user_mutex);
                }
              }
            }
            else if(variable == "send_to_user")//uid,value
            {
              string tmp = parseFor(ActiveBuffer, "id", error);
              int err;
              int tmpid = strToInt(tmp, err);

              if(error == 0 && tmpid >= 0 && tmpid < MaxUser)
              {
                string tmp2 = parseFor(ActiveBuffer, "value", error);
                if(error == 0)
                {
                  Message = "type=event_bug&id=" + IdStr + "&value=" + tmp2 + "&&";

                  pthread_mutex_lock(&user_mutex);
                  int tmpfd = Configuration.UserTab[tmpid].getFD();
                  if(tmpfd >= 0)
                  {
                    Configuration.UserTab[tmpid].SendQueue.push(Message);
                  }
                  pthread_mutex_unlock(&user_mutex);
                }
              }
            }
            else//wysyła event
            {
              Message = "type=event_BMess";
              Message += "&id=";
              Message += IdStr;
              Message += "&message=";
              Message += ActiveBuffer;
              Message += "&&";
              //wyslij do wszystkich aktywnych userow informacje
              for(int i = 0; i < MaxUser; i++)
              {
                long long int tmpfd;
                pthread_mutex_lock(&user_mutex);
                Configuration.UserTab[i].getFD(tmpfd);
                if(tmpfd != -1)
                {
                  Configuration.UserTab[i].SendQueue.push(Message);
                }
                pthread_mutex_unlock(&user_mutex);
              }
            }
          }
          ///wyslij wszystko z kolejki
          //1.odczytaj
          pthread_mutex_lock(&mrbug_mutex);
          updateSendQueue(Configuration.MrBugTab[MyIndex].SendQueue, SendQueue);
          pthread_mutex_unlock(&mrbug_mutex);
          //2.wyslij
          sendFromQueue(fd, SendQueue);
        }
        else if(logged_as == 3)///OTHER CASE
        {
          //other
        }
        else///NOT KNOWN
        {
          //ignore
        }
      }
    }
  }
}
