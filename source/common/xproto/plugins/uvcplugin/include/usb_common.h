/***************************************************************************
 * COPYRIGHT NOTICE
 * Copyright 2019 Horizon Robotics, Inc.
 * All rights reserved.
 ***************************************************************************/
#ifndef __USB_CAMERA_H__
#define __USB_CAMERA_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum HB_MIPI_SNS_TYPE_E {
  SENSOR_ID_INVALID,
  SIF_TEST_PATTERN0_1080P,          //   1
  SIF_TEST_PATTERN_12M_RAW12_8M,    //  2
  SIF_TEST_PATTERN_12M_RAW12_12M,   //  3
  SIF_TEST_PATTERN_8M_RAW12,        //  4
  IMX327_30FPS_1952P_RAW12_LINEAR,  //   5
  OS8A10_30FPS_3840P_RAW10_LINEAR,  //  6
  OV10635_30FPS_720p_954_YUV,       //  7
  OV10635_30FPS_720p_960_YUV,       //  8
  AR0144_30FPS_720P_RAW12_MONO,     //  9
  S5KGM1SP_30FPS_4000x3000_RAW10,
  GC02M1B_25FPS_1600x1200_RAW8,
  SAMPLE_SENOSR_ID_MAX,
} MIPI_SNS_TYPE_E;

typedef struct {
  int veChn;
  int type;
  int width;
  int height;
  int bitrate;
  int vpsGrp;
  int vpsChn;
  int quit;
} vencParam;
#if 0
int sample_singlepipe_venc();
int sample_singlepipe_venc_deinit();
void venc_pthread_start(void);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
