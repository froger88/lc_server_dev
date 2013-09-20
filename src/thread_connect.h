#ifndef THREAD_CONNECT_H
#define THREAD_CONNECT_H

#define _REENTRANT

#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <queue>
#include <mysql/mysql.h>
#include <pthread.h>

#include <zlib.h>

#include "settings.h"
#include "main.h"
#include "log.h"
#include "parse_for.h"
#include "user.h"
#include "mrbug.h"
#include "group.h"
#include "level.h"
#include "inttostr.h"
#include "client.h"
#include "strtoint.h"
#include "send_from_queue.h"
#include "update_send_queue.h"
#include "check_header.h"
#include "unpack.h"

void* threadConnect(void *arg);

#endif
