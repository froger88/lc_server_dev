
#include <fstream>

#include "thread_guard.h"

#include <pthread.h>
#include <bits/pthreadtypes.h>

#define _REENTRANT

using namespace std;

extern string client_id;
extern pthread_mutex_t user_mutex;
extern pthread_mutex_t mrbug_mutex;
extern pthread_mutex_t client_mutex;
extern pthread_mutex_t mutex;


extern int SHUTDOWN_STATE;

using namespace std;

void* threadGuard(void *arg)
{
  int port = 15000;
  struct sockaddr_in servaddr;
  int fd;
  bool ok = true;
  char Buffer[100];

  ///USTAWIENIE TL
  timespec TL;
  TL.tv_sec = 2; //seconds
  TL.tv_nsec = 0; //nano seconds

  timespec *timeout = &TL;

  ///KONIEC USTAWIANIA TL

  fd_set readmask;
  int nfound = 0;

  while(1)
  {
    ok = true;
    if((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
      //should to continue
      ok = false;
    }
    memset(&servaddr, 0, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if(ok == true)
    {
      if(inet_aton("127.0.0.1", &servaddr.sin_addr) <= 0)
      {
        //should to continue
        close(fd);
        ok = false;
      }
      if(ok == true)
      {
        if(connect(fd, (struct sockaddr *) & servaddr, sizeof (servaddr)) < 0)
        {
          //should to continue
          close(fd);
          ok = false;
        }
        while(ok == true)
        {
          nfound = 0;
          FD_ZERO(&readmask);
          FD_SET(fd, &readmask);
          memset(Buffer, NULL, 100);
          nfound = pselect(sizeof (readmask)*8, &readmask, NULL, &readmask, timeout, NULL);

          if(nfound == 0)
          {
            int x = write(fd, client_id.c_str(), client_id.length());
            if(x < 0)
            {
              ok = false;
              close(fd);
              sleep(300);
              break;
            }
          }
          else
          {
            memset(Buffer, NULL, 100);
            int th_str = read(fd, Buffer, 100);
            string buffer;
            if(th_str < 0)
            {
              ok = false;
              close(fd);
              sleep(300);
              break;
            }
            buffer.assign(Buffer);
            if(buffer == "kill")
            {
              //zerwij polaczenie z guardem - zeby myslal ze juz "nie zyje"
              close(fd);

              pthread_mutex_lock(&mutex);
              int MaxUser = Configuration.getMaxUser();
              int MaxClient = Configuration.getMaxClient();
              int MaxMrBug = Configuration.getMaxMrBug();
              //zmien status SHUTDOWN_STATE
              SHUTDOWN_STATE = 1;
              pthread_mutex_unlock(&mutex);

              //rozeslij informacje o majacym nastapic za 5 min padzie
              pthread_mutex_lock(&user_mutex);
              for(int i = 0; i < MaxUser; i++)
              {
                if(Configuration.UserTab[i].getID() >= 0)
                {
                  Configuration.UserTab[i].SendQueue.push("type=event_shutdown&&");
                }
              }
              pthread_mutex_unlock(&user_mutex);

              pthread_mutex_lock(&client_mutex);
              for(int i = 0; i < MaxClient; i++)
              {
                if(Configuration.ClientTab[i].getID() >= 0)
                {
                  Configuration.ClientTab[i].SendQueue.push("type=event_shutdown&&");
                }
              }
              pthread_mutex_unlock(&client_mutex);

              sleep(300); //czekaj 5 min, dopiero po tym czasie ubij

              //ustaw SHUTDOWN_STATE = 2 - wtedy wszyscy sie sami skickują
              pthread_mutex_lock(&mutex);
              SHUTDOWN_STATE = 2;
              pthread_mutex_unlock(&mutex);
              pthread_exit(0);
            }
          }
        }
      }
    }
    sleep(30);
  }
}
