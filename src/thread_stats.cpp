#include "thread_stats.h"
#include <pthread.h>
#include <bits/pthreadtypes.h>

#define _REENTRANT

extern Settings Configuration;
extern CStatistics StatsTab;

extern pthread_mutex_t mutex; //zabezpiecza dzialania na zmiennych srodowiskowych
extern pthread_mutex_t mrbug_mutex; //zabezpiecza dzialania na mrbugach
extern pthread_mutex_t db_mutex; //zabezpiecza dzialania na bazie danych
extern pthread_mutex_t intToStr_mutex; //zabezpiecza wywolania funkcji
extern pthread_mutex_t parseFor_mutex; //zabezpiecza wywolania funkcji
extern pthread_mutex_t strToInt_mutex; //zabezpiecza wywolania funkcji
extern pthread_mutex_t time_mutex; //zabezpiecza działanie zwiazane z czasem
extern pthread_mutex_t rand_mutex; //zabezpiecza dzialania z wykorzystywaniem rand
extern pthread_mutex_t stat1_mutex;
extern pthread_mutex_t stat2_mutex;

extern bool stats_on;

using namespace std;

void* threadStats(void *arg)
{

  //wylosuj TL
  pthread_mutex_lock(&time_mutex);
  pthread_mutex_lock(&rand_mutex);
  srand(time(NULL));
  pthread_mutex_unlock(&rand_mutex);
  pthread_mutex_unlock(&time_mutex);
  while(1)
  {
    pthread_mutex_lock(&time_mutex);
    pthread_mutex_lock(&rand_mutex);
    int TL = (rand() % 600) + 300;
    pthread_mutex_unlock(&rand_mutex);
    pthread_mutex_unlock(&time_mutex);


    pthread_mutex_lock(&mrbug_mutex);
    int MaxBug = Configuration.getMaxMrBug();
    pthread_mutex_unlock(&mrbug_mutex);

    sleep(TL);

    pthread_mutex_lock(&mutex);
    bool statistics_on = stats_on;
    pthread_mutex_unlock(&mutex);

    if(statistics_on)
    {
      pthread_mutex_lock(&stat1_mutex);
      StatsTab.saveAndClean(MaxBug);
      pthread_mutex_unlock(&stat1_mutex);
    }
  }

  pthread_exit(0);
}