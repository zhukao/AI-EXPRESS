#include "video_queue.h"

#include <malloc.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "hobotlog/hobotlog.hpp"

#define DEBUG
#undef DEBUG /* comment to show debug info */

#ifdef DEBUG
#define print printf
#else
#define print(...)
#endif

namespace horizon {
namespace vision {
namespace xproto {
namespace Uvcplugin {

QueueBuffer::QueueBuffer() {
  mutex = PTHREAD_MUTEX_INITIALIZER;
  Init();
}

QueueBuffer::~QueueBuffer() {}

int QueueBuffer::Init() {
  LOGI << "QueueBuffer::Init()";
  return queue_create(4);
  //    return 0;
}

QueueBuffer *QueueBuffer::GetInstance() {
  LOGI << "QueueBuffer::GetInstance()";
  static QueueBuffer instance;
  return &instance;
}
/**
 * create and return a new queue, with size nodes in total
 **/
int QueueBuffer::queue_create(int size) {
  LOGI << "Queue create";
  uvc_queue_ = (queue *)malloc(sizeof(queue));
  //   uvc_queue_ = (queue*)malloc(sizeof(size));
  if (!uvc_queue_) {
    fprintf(stderr, "malloc failed creating the queue\n");
    return -1;
  }

  uvc_queue_->first = NULL;
  uvc_queue_->last = NULL;
  uvc_queue_->total = size;
  uvc_queue_->num = 0;

  print("generated the queue @ %p\n", uvc_queue_);

  return 0;
}

/**
 * destroy the queue
 **/
void QueueBuffer::queue_destroy(queue *que) {
  print("Entered to queue_destroy");

  if (que == NULL) return;

  print("que is not null, que = %p\n", que);

  pthread_mutex_lock(&mutex);
  if (que->first == NULL) {
    print("que->first == NULL ...\n");
    free(que);
    pthread_mutex_unlock(&mutex);
    return;
  }

  /* destroy queue */
  node *_node = que->first;

  print("Destroy# first@ %p, last@ %p, num@ %u, total@ %u\n", que->first,
        que->last, que->num, que->total);

  while (_node != NULL) {
    // freeing the data coz it's on the heap and no one to free it
    // except for this one
    print("freeing: %p, data: %p, next: %p\n", _node, _node->data, _node->next);
    free(_node->data);
    node *tmp = _node->next;
    free(_node);
    _node = tmp;
  }

  free(que);
  que = NULL;

  pthread_mutex_unlock(&mutex);
}

/**
 * que is a queue pointer
 * total size bytes data will be copied into the queue
 **/
int QueueBuffer::enqueue(void *data, uint32_t size) {
  LOGI << "QueueBuffer::enqueue";
  void *tmp;
  node *new_node;

  print("Enqueue# in\n");
  pthread_mutex_lock(&mutex);
  LOGI << "uvc_queue num = " << uvc_queue_->num
       << " total = " << uvc_queue_->total;
  if (uvc_queue_->num >= uvc_queue_->total) {
    print("Enqueue# que is full. num(%u), total(%u)\n", uvc_queue_->num,
          uvc_queue_->total);
    pthread_mutex_unlock(&mutex);
    return -1;
  }

  new_node = (node *)malloc(sizeof(node));
  if (new_node == NULL) {
    pthread_mutex_unlock(&mutex);
    fprintf(stderr, "malloc failed to creating a node\n");
    return -1;
  }

  tmp = malloc(size);
  if (!tmp) {
    fprintf(stderr, "malloc failed for data copy\n");
    free(new_node);
    pthread_mutex_unlock(&mutex);
    return -1;
  }

  /* copy data to tmp and add tail to the queue */
  memcpy(tmp, data, size);
  new_node->data = tmp;
  new_node->size = size;
  new_node->next = NULL;

  if (uvc_queue_->first == NULL) {
    // new queue
    uvc_queue_->first = new_node;
    uvc_queue_->last = new_node;
  } else {
    uvc_queue_->last->next = new_node;
    uvc_queue_->last = new_node;
  }
  uvc_queue_->num++;

  print("Enqueue# _node@ %p, first@ %p, last@ %p, num@ %u, total@ %u\n",
        new_node, uvc_queue_->first, uvc_queue_->last, uvc_queue_->num,
        uvc_queue_->total);
  pthread_mutex_unlock(&mutex);
  print("Enqueue exit\n");

  return 0;
}

/**
 * que is a queue pointer
 * return the dequeue's buffer, size indicated the dequeue buffer length
 * if return NULL or size = 0, means no more buffer in queue
 * !! return's data should be used&freed by user
 **/
void *QueueBuffer::dequeue(uint32_t *size) {
  LOGI << "QueueBuffer::dequeue";
  LOGI << "dequeue num = " << uvc_queue_->num
       << " total = " << uvc_queue_->total;

  struct timespec t1, t2;
  long t1_ms, t2_ms, intv, dur;
  static long t1_last = 0;

  clock_gettime(CLOCK_MONOTONIC, &t1);
  t1_ms = t1.tv_sec * 1000 + t1.tv_nsec / (1000 * 1000);
  intv = t1_ms - t1_last;

  print("Dequeue# in\n");
  if (uvc_queue_ == NULL) {
    print("que is null exsting...\n");
    return NULL;
  }

  pthread_mutex_lock(&mutex);
  if (uvc_queue_->first == NULL) {
    pthread_mutex_unlock(&mutex);
    print("Dequeue# que is empty.\n");
    return NULL;
  }

  void *data;
  node *_node = uvc_queue_->first;
  if (uvc_queue_->first == uvc_queue_->last) {
    uvc_queue_->first = NULL;
    uvc_queue_->last = NULL;
  } else {
    uvc_queue_->first = _node->next;
  }

  uvc_queue_->num--;

  data = _node->data;
  *size = _node->size;

  print("Free _node@ %p, first@ %p, last@ %p, num@ %u, total@ %u\n", _node,
        uvc_queue_->first, uvc_queue_->last, uvc_queue_->num,
        uvc_queue_->total);
  free(_node);

  // copy last node
#if 0
    if (uvc_queue_->num == 0) {
    void *tmp;
    node *new_node;

    new_node = (node *)malloc(sizeof(node));
    if (new_node == NULL) {
    //    pthread_mutex_unlock(&mutex);
        fprintf(stderr, "malloc failed to creating a node\n");
    //    return -1;
    }

    tmp = malloc(*size);
    if (!tmp) {
        fprintf(stderr, "malloc failed for data copy\n");
        free(new_node);
     //   pthread_mutex_unlock(&mutex);
    //    return -1;
    }


    memcpy(tmp, data, *size);
    new_node->data = tmp;
    new_node->size = *size;
    new_node->next = NULL;

    if (uvc_queue_->first == NULL) {
        // new queue
        uvc_queue_->first = new_node;
        uvc_queue_->last = new_node;
    } else {
        uvc_queue_->last->next = new_node;
        uvc_queue_->last = new_node;
    }
    uvc_queue_->num++;
    }
#endif
  pthread_mutex_unlock(&mutex);
  print("Exiting deque\n");
  LOGI << "exiting deque";

  clock_gettime(CLOCK_MONOTONIC, &t2);

  t2_ms = t2.tv_sec * 1000 + t2.tv_nsec / (1000 * 1000);
  dur = t2_ms - t1_ms;
  LOGD << "get data queue t1_ms(" << t1_ms << "ms) t2_ms(" << t2_ms
       << "ms) interval (" << intv << "ms), and cost (" << dur << "ms)";
  t1_last = t1_ms;
  return data;
}

/**
 * que is a queue pointer
 * flush queue
 **/
void QueueBuffer::queue_flush() {
  print("Flush queue\n");
  if (uvc_queue_ == NULL) {
    print("que is null exsting...\n");
    return;
  }

  pthread_mutex_lock(&mutex);
  if (uvc_queue_->first == NULL) {
    pthread_mutex_unlock(&mutex);
    print("queue is alread empty, no need do flush.\n");
    return;
  }

  /* flush queue node */
  node *_node = uvc_queue_->first;

  print("Flush# first@ %p, last@ %p, num@ %u, total@ %u\n", uvc_queue_->first,
        uvc_queue_->last, uvc_queue_->num, uvc_queue_->total);

  while (_node != NULL) {
    // freeing the data coz it's on the heap and no one to free it
    // except for this one
    print("freeing: %p, data: %p, next: %p\n", _node, _node->data, _node->next);
    free(_node->data);
    node *tmp = _node->next;
    free(_node);
    _node = tmp;
  }

  /* reset queue to empty */
  uvc_queue_->first = NULL;
  uvc_queue_->last = NULL;
  uvc_queue_->num = 0;

  pthread_mutex_unlock(&mutex);

  return;
}

/**
 * que is a queue pointer
 * queue is empty on below case:
 * que->first == que->last == NULL
 **/
int QueueBuffer::queue_is_empty(queue *que) {
  int is_empty = 0;

  if (que == NULL) {
    print("que is null exsting...\n");
    return 0;
  }

  pthread_mutex_lock(&mutex);
  if (que->last == NULL) {
    print("queue is empty\n");
    is_empty = 1;
  }
  pthread_mutex_unlock(&mutex);

  return is_empty;
}

}  // namespace Uvcplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
