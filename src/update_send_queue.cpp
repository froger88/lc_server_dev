#include "update_send_queue.h"

void updateSendQueue(queue<string> &source, queue<string> &destination)
{
  while(!source.empty())
  {
    destination.push(source.front());
    source.pop();
  }
  return;
}
