/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * uvc_gadget.h
 *	uvc gadget work routine and functions
 *	current base code from https://github.com/wlhe/uvc-gadget
 *	net stage will replace with http://git.ideasonboard.org/uvc-gadget.git
 *
 * Copyright (C) 2019 Horizon Robotics, Inc.
 *
 * Contact: jianghe xu<jianghe.xu@horizon.ai>
 */

#ifndef _UVC_GADGET_H_
#define _UVC_GADGET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/usb/ch9.h>
#include <linux/usb/video.h>
#include <linux/videodev2.h>

#include "uvc.h"

/* below h265 definition in kernel include/uapi/linux/videodev2.h,
 * but user app use toolchain's include file
 */
#define V4L2_PIX_FMT_H265     v4l2_fourcc('H', '2', '6', '5') /* H265 with start codes */

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define max_c(a, b) (((a) > (b)) ? (a) : (b))

#define clamp(val, min, max_c)                                                                                           \
    ({                                                                                                                 \
        typeof(val) __val = (val);                                                                                     \
        typeof(min) __min = (min);                                                                                     \
        typeof(max_c) __max_c = (max_c);                                                                                     \
        (void)(&__val == &__min);                                                                                      \
        (void)(&__val == &__max_c);                                                                                      \
        __val = __val < __min ? __min : __val;                                                                         \
        __val > __max_c ? __max_c : __val;                                                                                 \
    })

#define ARRAY_SIZE(a) ((sizeof(a) / sizeof(a[0])))
#define pixfmtstr(x) (x) & 0xff, ((x) >> 8) & 0xff, ((x) >> 16) & 0xff, ((x) >> 24) & 0xff

struct uvc_context;

/* ---------------------------------------------------------------------------
 * Generic stuff
 */

/* callback function definitions */
typedef int (*uvc_prepare_buffer_callback_fn) (struct uvc_context * ctx,
					       void **buf_to, int *buf_len,
					       void **entity, void *userdata);
typedef void (*uvc_release_buffer_callback_fn) (struct uvc_context * ctx,
						void **entity, void *userdata);
typedef void (*uvc_streamon_callback_fn) (struct uvc_context * ctx, int is_on,
					  void *userdata);

/* h264 nalu enum */
enum {
	H264_NALU_IDR = 5,
	H264_NALU_SPS = 7,
	H264_NALU_PPS = 8,
};

/* IO methods supported */
enum io_method {
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR,
};

/* prepare buffer callback */
struct uvc_prepare_callback {
	uvc_prepare_buffer_callback_fn cb;
	void *userdata;
};

/* release buffer callback */
struct uvc_release_callback {
	uvc_release_buffer_callback_fn cb;
	void *userdata;
};

/* stream on callback */
struct uvc_streamon_callback {
	uvc_streamon_callback_fn cb;
	void *userdata;
};

/* Buffer representing one video frame */
struct buffer {
	struct v4l2_buffer buf;
	void *start;
	size_t length;
};

/* ---------------------------------------------------------------------------
 * V4L2 and UVC device instances
 */

/* Represents a V4L2 based video capture device */
struct v4l2_device {
	/* v4l2 device specific */
	int v4l2_fd;
	int is_streaming;
	char *v4l2_devname;

	/* v4l2 buffer specific */
	enum io_method io;
	struct buffer *mem;
	unsigned int nbufs;

	/* v4l2 buffer queue and dequeue counters */
	unsigned long long int qbuf_count;
	unsigned long long int dqbuf_count;

	/* uvc device hook */
	struct uvc_device *udev;
};

/* Represents a UVC based video output device */
struct uvc_device {
	/* uvc device specific */
	int uvc_fd;
	int is_streaming;
	int run_standalone;
	char *uvc_devname;

	/* uvc control request specific */

	struct uvc_streaming_control probe;
	struct uvc_streaming_control commit;
	int control;
	struct uvc_request_data request_error_code;
	unsigned int brightness_val;

	/* uvc buffer specific */
	enum io_method io;
	struct buffer *mem;
	struct buffer *dummy_buf;
	unsigned int nbufs;
	unsigned int fcc;
	unsigned int width;
	unsigned int height;

	unsigned int bulk;
	uint8_t color;
	unsigned int imgsize;
	void *imgdata;

	unsigned int bufsize;
	void *videobuf;
	void *entity;

	/* USB speed specific */
	int mult;
	int burst;
	int maxpkt;
	enum usb_device_speed speed;

	/* h264 sequence header (special case)*/
	char sps_pps[128];
	int sps_pps_size;
	int got_spspps;
	int idr_idx;
	int h264_quirk;

	/* uvc specific flags */
	int first_buffer_queued;
	int uvc_shutdown_requested;

	/* uvc buffer queue and dequeue counters */
	unsigned long long int qbuf_count;
	unsigned long long int dqbuf_count;

	/* v4l2 device hook */
	struct v4l2_device *vdev;

	/* parent */
	void *parent;

	/* uvc prepar, release & streamon callback&state */
	struct uvc_prepare_callback prepare_cb;
	struct uvc_release_callback release_cb;
	struct uvc_streamon_callback streamon_cb;
};

struct uvc_context {
	struct uvc_device *udev;	/* uvc v4l2 output device */
	struct v4l2_device *vdev;	/* option: v4l2 capture device (such as vivid) */
	pthread_t uvc_pid;
	int exit;
};

/* ---------------------------------------------------------------------------
 * V4L2 generic stuff
 */
int v4l2_open(struct v4l2_device **v4l2, char *devname,
	      struct v4l2_format *s_fmt);
int v4l2_reqbufs(struct v4l2_device *dev, int nbufs);
int v4l2_start_capturing(struct v4l2_device *dev);
int v4l2_process_data(struct v4l2_device *dev);
int v4l2_stop_capturing(struct v4l2_device *dev);
int v4l2_uninit_device(struct v4l2_device *dev);
void v4l2_close(struct v4l2_device *dev);

/* ---------------------------------------------------------------------------
 * UVC generic stuff
 */
int uvc_open(struct uvc_device **uvc, char *devname);
int uvc_video_set_format(struct uvc_device *dev);
void uvc_events_init(struct uvc_device *dev);
int uvc_video_reqbufs(struct uvc_device *dev, int nbufs);
int uvc_video_stream(struct uvc_device *dev, int enable);
void uvc_events_process(struct uvc_device *dev);
int uvc_video_process(struct uvc_device *dev);
int uvc_uninit_device(struct uvc_device *dev);
void uvc_close(struct uvc_device *dev);

/* ---------------------------------------------------------------------------
 * Some helper function
 */
char *fcc_to_string(unsigned int fcc);

#ifdef __cplusplus
}
#endif

#endif /* _UVC_GADGET_H_ */
