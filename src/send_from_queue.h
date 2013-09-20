/* 
 * File:   send_from_queue.h
 * Author: froger
 *
 * Created on 24 grudzień 2009, 22:03
 */

#ifndef _SEND_FROM_QUEUE_H
#define	_SEND_FROM_QUEUE_H

#include <string>
#include <queue>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/socket.h>
#include <netinet/tcp.h>
#include "write.h"

using namespace std;

void sendFromQueue(int fd, queue <string> &Queue);

#endif	/* _SEND_FROM_QUEUE_H */

