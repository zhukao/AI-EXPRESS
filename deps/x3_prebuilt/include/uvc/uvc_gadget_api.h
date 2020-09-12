/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * uvc_gadget_api.h
 *	uvc gadget api for uvc gadget application development.
 *
 * Copyright (C) 2019 Horizon Robotics, Inc.
 *
 * Contact: jianghe xu<jianghe.xu@horizon.ai>
 */

#ifndef _UVC_GADGET_API_H_
#define _UVC_GADGET_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "uvc_gadget.h"

/* pix format supported */
enum uvc_fourcc_format {
	UVC_FORMAT_YUY2 = 0,
	UVC_FORMAT_NV12,
	UVC_FORMAT_MJPEG,
	UVC_FORMAT_H264,
	UVC_FORMAT_H265,
};

/**
 * struct uvc_params - uvc params (user could set)
 * @width: frontend streaming resolution width
 * @height: frontend streaming resolution height
 * @format: frontend streaming format (support yuyv, mjpeg, h264...)
 * @io_method: 0 - mmap, 1 - userptr, 2 - dma(to do...)
 * @bulk_mode: 0 - isoc, 1 - bulk (isoc has hung issue, suggest use bulk)
 * @nbufs: number of uvc buffers (eg. 2 to use ping-pong buffer)
 * @mult: usb2.0 isoc high bandwidth feature. support 1024 ~ 3072(3*1024) bytes
 *		in micro-frame
 * @burst: usb3.0 only feature, support 0 ~ 15 burst.
 * @speed: 0 - usb2.0, 1 - usb3.0
 * @h264_quirk: 0 - default,
 *		1 - combine sps, pps and IDR to one frame(begining 3 frames)
 *			xj3 soc needs h264_quirk flag, otherwise black screen!!
 */
struct uvc_params {
	unsigned int width;
	unsigned int height;
	int format;
	int io_method;
	int bulk_mode;
	int nbufs;
	int mult;
	int burst;
	int speed;
	int h264_quirk;
};

/*### function declare ###*/
/**
 * uvc_gadget_user_params_init - init uvc_params with default value
 * @params: refer to above uvc_params
 *
 * default params is 720p, mjpeg, isoc, usb2.0 ...
 * user can re-write them according to self-setting.
 */
void uvc_gadget_user_params_init(struct uvc_params *params);

/**
 * uvc_gadget_init - init uvc gadget context according to user params
 * @pctx: uvc context, just pass a pointer, context will be allocated in lib.
 * @uvc_devname: uvc device name, for example, xj3 use /dev/video8 now.
 *		suggest using NULL, as lib will auto-find the uvc device.
 * @v4l2_devname: frontend v4l2 device, like vivid, v4l2-encoder...
 *		for xj3, just NULL... as encoder doesn't use v4l2 framework
 * @uvc_params: uvc params
 *
 * example: usb_gadget_init(&pctx, NULL, NULL, &user_params);
 */
int uvc_gadget_init(struct uvc_context **pctx, char *uvc_devname,
		    char *v4l2_devname, struct uvc_params *user_params);

/**
 * uvc_set_streamon_handler - set streamon off callback handler
 * @ctx: uvc context
 * @cb_fn: streamon off callback, usually to start/stop the frontend
 *		vio/encoder thread, let stream On of Off. Which will
 *		influence the stream realtime feature(specially at
 *		open/close stage)
 * @userdata: a (void *) pointer, that user could pass some user data,
 *		which usually will be re-used in callback function.
 *
 * streamon/off will be triggered by uvc control request
 *		UVC_EVENT_STREAMON  - 0x08000002
 *		UVC_EVENT_STREAMOFF - 0x08000003
 *		In usb layer, it usually happen in set interface/altersetting stage.
 */
void uvc_set_streamon_handler(struct uvc_context *ctx,
			      uvc_streamon_callback_fn cb_fn, void *userdata);

/**
 * uvc_set_prepare_data_handler - set prepare data handler
 * @ctx: uvc context
 * @cb_fn: prepare data callback, please prepare data which needs to be fed
 *	into uvc gadget device. Please pay attention that, just tell the buffer
 *	address to *buf_to, size to *buf_len, no need memcpy!! And the **entity
 *	is for the entity of specify buffer (eg. AVFrame * or xj3 encoder frame
 *	structure VIDEO_STREAM_S *). Please refer to uvc_app.c or usb_camera for
 *	reference
 * @userdata: a (void *) pointer, that user could pass some user data,
 *		which usually will be re-used in callback function.
 *
 * prepare data handler, please remember release **entity buffer in release
 * handler. eg. AVFrame *frame use free() to free frame. VIDEO_STREAM_S *
 * use HB_VENC_ReleaseStream to free xj3 encode buffer.
 */
void uvc_set_prepare_data_handler(struct uvc_context *ctx,
				  uvc_prepare_buffer_callback_fn cb_fn,
				  void *userdata);

/**
 * uvc_set_release_data_handler - set release data handler
 * @ctx: uvc context
 * @cb_fn: release data callback, please release data which is prepared
 *	in above prepare callback. The buffer which needs to be released
 *	is in **entity. Please refer to uvc_app.c or usb_camera demo code
 *	for reference.
 * @userdata: a (void *) pointer, that user could pass some user data,
 *		which usually will be re-used in callback function.
 *
 * release data handler. eg. AVFrame *frame use free() to free frame.
 * VIDEO_STREAM_S *use HB_VENC_ReleaseStream to free xj3 encode buffer.
 *
 * prepare & release buffer needs to be implemented as a pair!!
 */
void uvc_set_release_data_handler(struct uvc_context *ctx,
				  uvc_release_buffer_callback_fn cb_fn,
				  void *userdata);

/**
 * uvc_gadget_start - start the uvc loop to handle uvc control/streaming request
 * @ctx: uvc context
 *
 * It start the uvc main loop, which use select function to monitor the related
 * device's io. eg. exceptfds is used for uvc standard control request handle.
 * writefds is for uvc standard streaming request handle.
 *
 * Anyway, uvc_gadget_start create the uvc_loop thread which is the work routine
 * for uvc control/streaming handle!!
 */
int uvc_gadget_start(struct uvc_context *ctx);

/**
 * uvc_gadget_stop - stop the uvc loop.
 * @ctx: uvc context
 *
 * Stop the uvc work routine. Usually called when user want to stop uvc-gadget.
 * (To launch another usb gadget, like adb??)
 *
 * Anyway, it is called by user app when you want to stop uvc.
 */
int uvc_gadget_stop(struct uvc_context *ctx);

/**
 * uvc_gadget_deinit - deinit uvc-gadget device.
 * @ctx: uvc context
 *
 * deinit uvc gadget device, which is opposite with of uvc_gdaget_init.
 */
void uvc_gadget_deinit(struct uvc_context *ctx);


/*### helper functions ###*/
/**
 * uvc_format_to_fcc - convert uvc format to fcc type
 * @format: uvc_fourcc_format (yuy2, mjpeg, h264...)
 *
 * helper function to convert uvc format and fcc type
 */
unsigned int uvc_format_to_fcc(enum uvc_fourcc_format format);

#ifdef __cplusplus
}
#endif

#endif /* _UVC_GADGET_API_H_ */
