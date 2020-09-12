/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * camera.h
 *	v4l2 camera interface
 *
 * Copyright (C) 2019 Horizon Robotics, Inc.
 *
 * Contact: jianghe xu<jianghe.xu@horizon.ai>
 */

#ifndef __V4L2_CAMERA_H_
#define __V4L2_CAMERA_H_

#include <pthread.h>

struct video_frame;

/** A callback function to handle incoming camera
 * frames steaming.
 */
typedef void (camera_frame_callback_t)(struct video_frame *frame, void *user_args);

/* enum & structures... */
typedef enum fcc_format {
	FCC_INVALID = -1,
	FCC_YUY2,
	FCC_NV12,
	FCC_MJPEG,
	FCC_H264,
	FCC_H265,
} fcc_format;

typedef enum io_memory {
	IO_UNKNOWN	= 0,
	IO_MMAP		= 1,
	IO_USERPTR	= 2,
	IO_OVERLAY	= 3,
	IO_DMABUF	= 4,
} io_memory;

struct video_frame {
	struct timeval timestamp;
	fcc_format fcc;
	void *mem;
	int width;
	int height;
	int length;
};

typedef struct camera_param {
	fcc_format fcc;
	int width;
	int height;
	int fps;
} camera_param_t;

typedef struct frame_desc {
	int width;
	int height;
} frame_desc;

typedef struct format_desc {
	unsigned int pixelformat;
	unsigned char description[32];

	frame_desc *frm_desc;
	int frame_count;
} format_desc;

typedef struct format_enums {
	format_desc *fmt_desc;
	int format_count;
} format_enums;

typedef struct camera {
	const char *name;
	struct v4l2_device *v4l2_dev;
	struct camera_param params;
	struct format_enums *fmt_enum;

	io_memory io_method;
	int nbufs;

	/* thread management */
	pthread_t cam_pid;
	camera_frame_callback_t *cb;
	void *user_args;
	int quit;
} camera_t;

/* api definitions */
camera_t *camera_open(const char *devname);
int camera_enum_format(camera_t *cam, format_enums *fmt_enum, int show_info);
int camera_show_format(camera_t *cam);
int camera_set_params(camera_t *cam, camera_param_t *params);
int camera_set_format(camera_t *cam, int format_index, int frame_index);
int camera_set_framerate(camera_t *cam, int fps);
int camera_start_streaming(camera_t *cam,
		camera_frame_callback_t *cb,
		void *user_args);
int camera_grab_stream(camera_t *cam);
int camera_stop_streaming(camera_t *cam);
void camera_close(camera_t *cam);

/* helper function */
fcc_format pixelformat_to_fcc(unsigned int pixelformat);
char *fcc_format_to_string(fcc_format fcc);

#endif /* __V4L2_CAMERA_H */
