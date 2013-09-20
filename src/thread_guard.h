/* 
 * File:   thread_guard.h
 * Author: froger
 *
 * Created on 23 wrzesień 2009, 23:20
 */

#ifndef THREAD_GUARD_H
#define THREAD_GUARD_H

#define _REENTRANT

#include <pthread.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "main.h"

void* threadGuard(void *arg);

#endif	/* _THREAD_GUARD_H */

