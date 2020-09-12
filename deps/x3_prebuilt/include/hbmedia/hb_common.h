/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file:
 * @brief:
 * @author:
 * @email:
 * @date: 2019.11.21
 */

#ifndef __HB_COMMON_H__
#define __HB_COMMON_H__
#include <stdint.h>
#include "hb_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#ifndef VER_X
#define VER_X 1
#endif

#ifndef VER_Y
#define VER_Y 0
#endif

#ifndef VER_Z
#define VER_Z 0
#endif

#ifndef VER_P
#define VER_P 0
#endif

#ifndef VER_B
#define VER_B 0
#endif

#ifdef HB_DEBUG
#define VER_D " Debug"
#else
#define VER_D " Release"
#endif

#define __MK_VERSION(x, y, z, p, b) #x "." #y "." #z "." #p " B0" #b
#define MK_VERSION(x, y, z, p, b) __MK_VERSION(x, y, z, p, b)
#define MPP_VERSION \
  CHBP_NAME MPP_VER_PRIX MK_VERSION(VER_X, VER_Y, VER_Z, VER_P, VER_B) VER_D

#define VERSION_NAME_MAXLEN 64
// typedef struct hbMPP_VERSION_S
// {
//     HB_CHAR aVersion[VERSION_NAME_MAXLEN];
// } MPP_VERSION_S;
//
// typedef struct hbCROP_INFO_S
// {
//     HB_BOOL bEnable;
//     CODEC_RECT_S  stRect;
// } CROP_INFO_S;
typedef enum HB_ROTATE_E {
  ROTATE_NONE = 0,
  ROTATE_90 = 1,
  ROTATE_180 = 2,
  ROTATE_270 = 3,
  ROTATE_BUTT
} ROTATE_E;

typedef struct HB_BORDER_S {
  uint32_t u32TopWidth;
  uint32_t u32BottomWidth;
  uint32_t u32LeftWidth;
  uint32_t u32RightWidth;
  uint32_t u32Color;
} BORDER_S;

typedef int32_t AI_CHN;
typedef int32_t AO_CHN;
typedef int32_t AENC_CHN;
typedef int32_t ADEC_CHN;
typedef int32_t AUDIO_DEV;
typedef int32_t AVENC_CHN;
typedef int32_t VI_DEV;
typedef int32_t VI_WAY;
typedef int32_t VI_CHN;
typedef int32_t VO_DEV;
typedef int32_t VO_LAYER;
typedef int32_t VO_CHN;
typedef int32_t VO_WBC;
typedef int32_t GRAPHIC_LAYER;
typedef int32_t VENC_CHN;
typedef int32_t VDEC_CHN;
typedef int32_t VENC_GRP;
typedef int32_t VO_GRP;
typedef int32_t VDA_CHN;
typedef int32_t IVE_HANDLE;
typedef int32_t CLS_HANDLE;
typedef int32_t FD_CHN;
typedef int32_t MD_CHN;
typedef int32_t ISP_DEV;
typedef int32_t SENSOR_ID;

#define HB_INVALID_CHN (-1)
#define HB_INVALID_WAY (-1)
#define HB_INVALID_LAYER (-1)
#define HB_INVALID_DEV (-1)
#define HB_INVALID_HANDLE (-1)
#define HB_INVALID_VALUE (-1)
#define HB_INVALID_TYPE (-1)

typedef enum hbPROFILE_TYPE_E {
  PROFILE_1080P_30 = 0,
  PROFILE_3M_30,
  PROFILE_1080P_60,
  PROFILE_5M_30,
  PROFILE_BUTT,
} PROFILE_TYPE_E;

#define MPP_MOD_VIU "vi"
#define MPP_MOD_VOU "vo"
#define MPP_MOD_HDMI "hdmi"
#define MPP_MOD_DSU "dsu"
#define MPP_MOD_VGS "vgs"
#define MPP_MOD_FISHEYE "fisheye"

#define MPP_MOD_CHNL "chnl"
#define MPP_MOD_VENC "venc"
#define MPP_MOD_GRP "grp"
#define MPP_MOD_VDA "vda"
#define MPP_MOD_VPSS "vpss"
#define MPP_MOD_RGN "rgn"
#define MPP_MOD_IVE "ive"
#define MPP_MOD_FD "fd"
#define MPP_MOD_MD "md"

#define MPP_MOD_H264E "h264e"
#define MPP_MOD_H265E "h265e"
#define MPP_MOD_JPEGE "jpege"
#define MPP_MOD_MPEG4E "mpeg4e"

#define MPP_MOD_VDEC "vdec"
#define MPP_MOD_H264D "h264d"
#define MPP_MOD_JPEGD "jpegd"

#define MPP_MOD_AI "ai"
#define MPP_MOD_AO "ao"
#define MPP_MOD_AENC "aenc"
#define MPP_MOD_ADEC "adec"
#define MPP_MOD_AIO "aio"
#define MPP_MOD_ACODEC "acodec"

#define MPP_MOD_VB "vb"
#define MPP_MOD_SYS "sys"
#define MPP_MOD_CMPI "cmpi"

#define MPP_MOD_PCIV "pciv"
#define MPP_MOD_PCIVFMW "pcivfmw"

#define MPP_MOD_PROC "proc"
#define MPP_MOD_LOG "logmpp"
#define MPP_MOD_MST_LOG "mstlog"

#define MPP_MOD_DCCM "dccm"
#define MPP_MOD_DCCS "dccs"

#define MPP_MOD_VCMP "vcmp"
#define MPP_MOD_FB "fb"

#define MPP_MOD_RC "rc"

#define MPP_MOD_VOIE "voie"

#define MPP_MOD_TDE "tde"
#define MPP_MOD_ISP "isp"
#define MPP_MOD_USR "usr"

/* We just coyp this value of payload type from RTP/RTSP definition */
typedef enum {
  PT_PCMU = 0,
  PT_1016 = 1,
  PT_G721 = 2,
  PT_GSM = 3,
  PT_G723 = 4,
  PT_DVI4_8K = 5,
  PT_DVI4_16K = 6,
  PT_LPC = 7,
  PT_PCMA = 8,
  PT_G722 = 9,
  PT_S16BE_STEREO = 10,
  PT_S16BE_MONO = 11,
  PT_QCELP = 12,
  PT_CN = 13,
  PT_MPEGAUDIO = 14,
  PT_G728 = 15,
  PT_DVI4_3 = 16,
  PT_DVI4_4 = 17,
  PT_G729 = 18,
  PT_G711A = 19,
  PT_G711U = 20,
  PT_G726 = 21,
  PT_G729A = 22,
  PT_LPCM = 23,
  PT_CelB = 25,
  PT_JPEG = 26,
  PT_CUSM = 27,
  PT_NV = 28,
  PT_PICW = 29,
  PT_CPV = 30,
  PT_H261 = 31,
  PT_MPEGVIDEO = 32,
  PT_MPEG2TS = 33,
  PT_H263 = 34,
  PT_SPEG = 35,
  PT_MPEG2VIDEO = 36,
  PT_AAC = 37,
  PT_WMA9STD = 38,
  PT_HEAAC = 39,
  PT_PCM_VOICE = 40,
  PT_PCM_AUDIO = 41,
  PT_AACLC = 42,
  PT_MP3 = 43,
  PT_ADPCMA = 49,
  PT_AEC = 50,
  PT_X_LD = 95,
  PT_H264 = 96,
  PT_D_GSM_HR = 200,
  PT_D_GSM_EFR = 201,
  PT_D_L8 = 202,
  PT_D_RED = 203,
  PT_D_VDVI = 204,
  PT_D_BT656 = 220,
  PT_D_H263_1998 = 221,
  PT_D_MP1S = 222,
  PT_D_MP2P = 223,
  PT_D_BMPEG = 224,
  PT_MP4VIDEO = 230,
  PT_MP4AUDIO = 237,
  PT_VC1 = 238,
  PT_JVC_ASF = 255,
  PT_D_AVI = 256,
  PT_DIVX3 = 257,
  PT_AVS = 258,
  PT_REAL8 = 259,
  PT_REAL9 = 260,
  PT_VP6 = 261,
  PT_VP6F = 262,
  PT_VP6A = 263,
  PT_SORENSON = 264,
  PT_H265 = 265,
  PT_MAX = 266,

  PT_AMR = 1001,
  PT_MJPEG = 1002,
  PT_AMRWB = 1003,
  PT_BUTT
} PAYLOAD_TYPE_E;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* _HB_COMMON_H_ */
