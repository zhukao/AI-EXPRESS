/***************************************************************************
* COPYRIGHT NOTICE
* Copyright 2019 Horizon Robotics, Inc.
* All rights reserved.
***************************************************************************/
#ifndef HB_X2A_VIO_HB_UTILS_H
#define HB_X2A_VIO_HB_UTILS_H

#include "cJSON.h"
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <errno.h>
#include "hb_vio_common.h"
#include "logging.h"

#define HB_VIO_NAME_STR_SIZE (128)
#define HB_VIO_MAX_WEIGHT 3840
#define HB_VIO_MAX_height 2160
#define HB_VIO_MAX_VC_NUM 4

#define XJ3_VIO_DEBUG (1)

#define vio_err(format, ...)			\
do {									\
	char str[30];						\
	struct timeval ts;					\
	gettimeofday(&ts, NULL);			\
	snprintf(str, sizeof(str), "%ld.%06ld", ts.tv_sec, ts.tv_usec);			\
	pr_err("[%s]%s[%d] E: "format"\n", str, __func__, __LINE__, ##__VA_ARGS__);\
} while (0)

#define vio_warn(format, ...)			\
do {									\
	char str[30];						\
	struct timeval ts;					\
	gettimeofday(&ts, NULL);			\
	snprintf(str, sizeof(str), "%ld.%06ld", ts.tv_sec, ts.tv_usec);			\
	pr_warn("[%s]%s[%d] w: "format"\n", str, __func__, __LINE__, ##__VA_ARGS__);\
} while (0)

#define vio_log(format, ...)			\
do {									\
	char str[30];						\
	struct timeval ts;					\
	gettimeofday(&ts, NULL);			\
	snprintf(str, sizeof(str), "%ld.%06ld", ts.tv_sec, ts.tv_usec);			\
	pr_info("[%s]%s[%d] L: "format"\n", str, __func__, __LINE__, ##__VA_ARGS__);\
} while (0)

#if XJ3_VIO_DEBUG
#define vio_dbg(format, ...)			\
do {									\
	char str[30];						\
	struct timeval ts;					\
	gettimeofday(&ts, NULL);			\
	snprintf(str, sizeof(str), "%ld.%06ld", ts.tv_sec, ts.tv_usec);			\
	pr_debug("[%s]%s[%d] D: "format"\n", str, __func__, __LINE__, ##__VA_ARGS__);\
} while (0)
#else
#define vio_dbg(format, ...)
#endif

#define XJ3_CPU_SETSIZE 4
#define XJ3_CPU_0 0
#define XJ3_CPU_1 1
#define XJ3_CPU_2 2
#define XJ3_CPU_3 3


int set_thread_affinity(pthread_t pid, int cpu_num);
int get_thread_policy(pthread_attr_t *attr);
int set_thread_policy(pthread_attr_t *attr,  int policy);
void get_thread_priority_rang(int policy, int *min, int *max);
int get_thread_priority(pthread_attr_t *attr);
int set_thread_priority(pthread_attr_t *attr, int priority);


void time_add_ms(struct timeval *time, uint16_t ms);
int get_cost_time_ms(struct timeval *start, struct timeval *end);
int dumpToFile(char *filename, char *srcBuf, unsigned int size);
int dumpToFile2plane(char *filename, char *srcBuf, char *srcBuf1,
		     unsigned int size, unsigned int size1);

#endif //HB_X2A_VIO_HB_UTILS_H
