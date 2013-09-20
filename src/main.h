#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <string>
#include <cstring>
#include <sstream>

#include <sys/socket.h>
#include <sys/types.h>

#include <linux/socket.h>
#include <netinet/tcp.h>

#include <strings.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <mysql/mysql.h>
#include <fcntl.h>

#include "settings.h"
#include "thread_connect.h"
#include "thread_stats.h"
#include "thread_guard.h"
#include "thread_time_events.h"
#include "inttostr.h"
#include "cstatistics.h"

#include "parse_for.h"

extern Settings Configuration;

#endif
