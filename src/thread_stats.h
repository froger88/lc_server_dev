/* 
 * File:   thread_stats.h
 * Author: froger
 *
 * Created on 23 wrzesień 2009, 23:16
 */

#ifndef THREAD_STATS_H
#define THREAD_STATS_H

#define _REENTRANT

#include <pthread.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <mysql/mysql.h>
#include <unistd.h>

#include "settings.h"
#include "main.h"
#include "parse_for.h"
#include "user.h"
#include "mrbug.h"
#include "inttostr.h"
#include "strtoint.h"
#include "ipstrtoint.h"
#include "client.h"
#include "strtoint.h"

void* threadStats(void *arg);

#endif	/* _THREAD_STATS_H */

