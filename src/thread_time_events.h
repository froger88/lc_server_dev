/* 
 * File:   thread_time_events.h
 * Author: froger
 *
 * Created on 22 marzec 2010, 22:28
 */

/*
 *Watek odpowiedzialny za zdarzenia wykonywane w okreslonym przedziale czasowym
 *jak np:
 *  -uaktualnianie bazy banów
 *  -uaktualnianie bazy domen * -> to zosatnie dodane na koncu, o ile wogole zostanie dodane, poniewaz zmiana il. domen wiaze sie ze zmiana max_mrbug
 *  -uaktualnianie akceptacji subdomen
 *  -uaktualnianie invitationDATA (czas do autozapraszania)
 *
 */


#ifndef _THREAD_TIME_EVENTS_H
#define	_THREAD_TIME_EVENTS_H

#define _REENTRANT

#include <pthread.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <ctime>
#include <cstdlib>

#include "main.h"

void* threadTimeEvents(void *arg);

#endif	/* _THREAD_TIME_EVENTS_H */

