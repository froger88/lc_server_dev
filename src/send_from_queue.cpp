#include "send_from_queue.h"
#include <iostream>
using namespace std;

void sendFromQueue(int fd, queue<string> &Queue)
{
  string Message;
  if(fd >= 0)
  {
    while(!Queue.empty())
    {
      Message = Queue.front();
      Queue.pop();
      bool sent = _write(fd, Message.c_str(), Message.length());
      if(!sent)
      {
        break;
      }
    }
  }
  return;
}
