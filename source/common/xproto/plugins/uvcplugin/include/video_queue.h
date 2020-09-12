#ifndef _VIDEO_QUEUE_H
#define _VIDEO_QUEUE_H

#include <pthread.h>
#include <stdint.h>

namespace horizon {
namespace vision {
namespace xproto {
namespace Uvcplugin {

struct _node {
  void *data;
  uint32_t size;
  struct _node *next;
};
typedef struct _node node;

struct _queue {
  node *first;
  node *last;
  uint32_t num;
  uint32_t total;
};

typedef struct _queue queue;

class QueueBuffer {
 public:
  QueueBuffer();
  ~QueueBuffer();
  int Init();

  static QueueBuffer *GetInstance();
  // create and return the queue
  int queue_create(int size);

  // destroy the queue (free all the memory associate with the que even the
  // data)
  void queue_destroy(queue *que);

  // queue the data into queue
  // total size bytes data will be copied into queue
  //  int enqueue(queue *que, void *data, uint32_t size);
  int enqueue(void *data, uint32_t size);
  // return the data from the queue (FIFO)
  // and free up all the internally allocated memory
  // but the user have to fee the returning data pointer
  //  void *dequeue(queue *que, uint32_t *size);
  void *dequeue(uint32_t *size);

  // flush queue
  void queue_flush();

  // queue is empty on below case:
  // que->first == que->last == NULL
  int queue_is_empty(queue *que);

  int queue_is_full(queue *que);

 private:
  pthread_mutex_t mutex;
  queue *uvc_queue_;
};
}  // namespace Uvcplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon

#endif /* _VIDEO_QUEUE_H */
