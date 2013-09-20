/* 
 * File:   update_send_queue.h
 * Author: froger
 *
 * Created on 24 grudzień 2009, 22:14
 */

#ifndef _UPDATE_SEND_QUEUE_H
#define	_UPDATE_SEND_QUEUE_H

#include <queue>
#include <string>
using namespace std;

void updateSendQueue(queue<string> &source, queue<string> &destination);

#endif	/* _UPDATE_SEND_QUEUE_H */

