/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * video_queue.c
 *	a video memcpy queue (not a zero copy queue)
 *
 * Copyright (C) 2019 Horizon Robotics, Inc.
 *
 * Contact: jianghe xu<jianghe.xu@horizon.ai>
 */

#ifndef _VIDEO_QUEUE_H
#define _VIDEO_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

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

// create and return the queue
queue *queue_create(int size);

// destroy the queue (free all the memory associate with the que even the data)
void queue_destroy(queue * que);

// queue the data into queue
// total size bytes data will be copied into queue
int enqueue(queue * que, void *data, uint32_t size);

// return the data from the queue (FIFO)
// and free up all the internally allocated memory
// but the user have to fee the returning data pointer
void *dequeue(queue * que, uint32_t * size);

// flush queue
void queue_flush(queue * que);

// queue is empty on below case:
// que->first == que->last == NULL
int queue_is_empty(queue * que);

// queue is full on below case:
// que->num == que->total
int queue_is_full(queue * que);

#ifdef __cplusplus
}
#endif

#endif /* _VIDEO_QUEUE_H */
