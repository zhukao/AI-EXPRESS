/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file:
 * @brief:
 * @author:
 * @email:
 * @date: 2019.12.16
 */

#ifndef __HB_COMM_VDEC_H__
#define __HB_COMM_VDEC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "hb_comm_venc.h"
#include "hb_comm_video.h"
#include "hb_common.h"
#include "hb_type.h"
#include "./hb_errno.h"
#include "./hb_sys.h"

typedef enum {
	EN_ERR_VDEC_UNKNOWN = 0,
    EN_ERR_VDEC_NOT_FOUND = 1,
    EN_ERR_VDEC_OPEN_FAIL = 2,
    EN_ERR_VDEC_RESPONSE_TIMEOUT = 3,
    EN_ERR_VDEC_INIT_FAIL = 4,
    EN_ERR_VDEC_OPERATION_NOT_ALLOWDED = 5,
    EN_ERR_VDEC_NOMEM = 6,
    EN_ERR_VDEC_NO_FREE_CHANNEL = 7,
    EN_ERR_VDEC_ILLEGAL_PARAM = 8,
    EN_ERR_VDEC_INVALID_CHNID = 9,
    EN_ERR_VDEC_INVALID_BUF = 10,
    EN_ERR_VDEC_INVALID_CMD = 11,
    EN_ERR_VDEC_WAIT_TIMEOUT = 12,
    EN_ERR_VDEC_FILE_OPERATION_FAIL = 13,
    EN_ERR_VDEC_PARAMS_SET_FAIL = 14,
    EN_ERR_VDEC_PARAMS_GET_FAIL = 15,
    EN_ERR_VDEC_EXIST = 16,
    EN_ERR_VDEC_UNEXIST = 17,
    EN_ERR_VDEC_NULL_PTR = 18
} EN_VDEC_ERR_CODE_E;

/* Vdec unknown error ---->0x1008FC00(269024256) */
#define HB_ERR_VDEC_UNKNOWN		                                 \
    HB_DEF_ERR(HB_ID_VDEC, EN_ERR_VDEC_UNKNOWN)
/* Vdec device not found */
#define HB_ERR_VDEC_NOT_FOUND                                    \
    HB_DEF_ERR(HB_ID_VDEC, EN_ERR_VDEC_NOT_FOUND)
/* Failed to open vdec device */
#define HB_ERR_VDEC_OPEN_FAIL                                    \
    HB_DEF_ERR(HB_ID_VDEC, EN_ERR_VDEC_OPEN_FAIL)
/* Timeout to operate vdec device */
#define HB_ERR_VDEC_RESPONSE_TIMEOUT                             \
    HB_DEF_ERR(HB_ID_VDEC, EN_ERR_VDEC_RESPONSE_TIMEOUT)
/* Failed to initialize vdec device */
#define HB_ERR_VDEC_INIT_FAIL                                    \
    HB_DEF_ERR(HB_ID_VDEC, EN_ERR_VDEC_INIT_FAIL)
/* Operation is not allowed */
#define HB_ERR_VDEC_OPERATION_NOT_ALLOWDED                       \
    HB_DEF_ERR(HB_ID_VDEC, EN_ERR_VDEC_OPERATION_NOT_ALLOWDED)
/* Vdec memory is not insufficient */
#define HB_ERR_VDEC_NOMEM                                        \
    HB_DEF_ERR(HB_ID_VDEC, EN_ERR_VDEC_NOMEM)
/* No free vdec channel left */
#define HB_ERR_VDEC_NO_FREE_CHANNEL                              \
    HB_DEF_ERR(HB_ID_VDEC, EN_ERR_VDEC_NO_FREE_CHANNEL)
/* Invalid channel params */
#define HB_ERR_VDEC_ILLEGAL_PARAM                                \
    HB_DEF_ERR(HB_ID_VDEC, EN_ERR_VDEC_ILLEGAL_PARAM)
/* Invalid channel ID */
#define HB_ERR_VDEC_INVALID_CHNID                                \
    HB_DEF_ERR(HB_ID_VDEC, EN_ERR_VDEC_INVALID_CHNID)
/* Invalid buffer */
#define HB_ERR_VDEC_INVALID_BUF                                  \
    HB_DEF_ERR(HB_ID_VDEC, EN_ERR_VDEC_INVALID_BUF)
/* Invalid command */
#define HB_ERR_VDEC_INVALID_CMD                                  \
    HB_DEF_ERR(HB_ID_VDEC, EN_ERR_VDEC_INVALID_CMD)
/* Wait timeout */
#define HB_ERR_VDEC_WAIT_TIMEOUT                                 \
    HB_DEF_ERR(HB_ID_VDEC, EN_ERR_VDEC_WAIT_TIMEOUT)
/* File cannot be operated successfully */
#define HB_ERR_VDEC_FILE_OPERATION_FAIL                          \
    HB_DEF_ERR(HB_ID_VDEC, EN_ERR_VDEC_FILE_OPERATION_FAIL)
/* Failed to set parameters */
#define HB_ERR_VDEC_PARAMS_SET_FAIL                              \
    HB_DEF_ERR(HB_ID_VDEC, EN_ERR_VDEC_PARAMS_SET_FAIL)
/* Failed to get parameters */
#define HB_ERR_VDEC_PARAMS_GET_FAIL                              \
    HB_DEF_ERR(HB_ID_VDEC, EN_ERR_VDEC_PARAMS_GET_FAIL)
/* Vdec channel exist */
#define HB_ERR_VDEC_EXIST                                        \
    HB_DEF_ERR(HB_ID_VDEC, EN_ERR_VDEC_EXIST)
/* Vdec channel unexist */
#define HB_ERR_VDEC_UNEXIST                                      \
    HB_DEF_ERR(HB_ID_VDEC, EN_ERR_VDEC_UNEXIST)
/* Vdedec channel null ptr ---->0x1008FC12(269024274)*/
#define HB_ERR_VDEC_NULL_PTR                                     \
    HB_DEF_ERR(HB_ID_VDEC, EN_ERR_VDEC_NULL_PTR)

typedef enum HB_VIDEO_MODE_E {
  VIDEO_MODE_STREAM = 0, /* send by stream */
  VIDEO_MODE_FRAME,      /* send by frame */
  VIDEO_MODE_BUTT
} VIDEO_MODE_E;

typedef enum HB_VIDEO_OUTPUT_ORDER_E {
  VIDEO_OUTPUT_ORDER_DISP = 0,
  VIDEO_OUTPUT_ORDER_DEC,
  VIDEO_OUTPUT_ORDER_BUTT
} VIDEO_OUTPUT_ORDER_E;

typedef enum HB_VIDEO_DEC_MODE_E {
  VIDEO_DEC_MODE_NORMAL = 0, /* Decode IPB frame */
  VIDEO_DEC_MODE_IRAP, /* Decode IRAP frame */
  VIDEO_DEC_MODE_REF, /* Decode reference frame */
  VIDEO_DEC_MODE_THUMB, /*Decode IRAP fream without DPB */
  VIDEO_DEC_MODE_BUTT
} VIDEO_DEC_MODE_E;

typedef struct HB_VDEC_USERDATA_S {
    /* userdata data is valid */
    HB_BOOL    bValid;
    /* userdata data len */
    uint32_t   u32Len;
    /* userdata data phy address */
    uint64_t   u64PhyAddr;
    /* userdata data vir address */
    uint8_t*   pu8Addr;
} VDEC_USERDATA_S;

typedef struct HB_VDEC_CHN_STATUS_S {
/**
 * Current input buffer count. The input buffer may be frames waiting for
 * encoding or streams waiting for decoding. During decoding,
 * if MC_FEEDING_MODE_FRAME_SIZE is set, the input_buf_cnt also means
 * frame count to be decoded.
 *
 * - Note:
 * - Encoding: Support.
 * - Decoding: Support.
 * - Default:
 */
	uint32_t cur_input_buf_cnt;

/**
 * Current input buffer size. The input buffer may be frames waiting for
 * encoding or streams waiting for decoding.
 *
 * - Note:
 * - Encoding: Support.
 * - Decoding: Support.
 * - Default:
 */
	uint64_t cur_input_buf_size;

/**
 * Current output buffer count. The output buffer may be decoded frames
 * or encoded streams.
 *
 * - Note:
 * - Encoding: Support.
 * - Decoding: Support.
 * - Default:
 */
	uint64_t cur_output_buf_cnt;

/**
 * Current output buffer size. The output buffer may be decoded frames
 * or encoded streams.
 *
 * - Note:
 * - Encoding: Support.
 * - Decoding: Support.
 * - Default:
 */
	uint64_t cur_output_buf_size;

/**
 * Left receiving frames count. It's valid only when receive_frame_number
 * in mc_video_enc_startup_params_t is set. It means the left frames count
 * that should be received.
 * @mc_video_enc_startup_params_t
 *
 * - Note:
 * - Encoding: Support.
 * - Decoding: Unsupport.
 * - Default:
 */
	uint32_t left_recv_frame;

/**
 * Left encoding frames count. It's valid only when receive_frame_number
 * in mc_video_enc_startup_params_t is set. It means the left frames count
 * that should be encoded.
 * @mc_video_enc_startup_params_t
 *
 * - Note:
 * - Encoding: Support.
 * - Decoding: Unsupport.
 * - Default:
 */
	uint32_t left_enc_frame;

/**
 * Total received buffer count. It means total received frame or stream buffer
 * count.
 *
 * - Note:
 * - Encoding: Support.
 * - Decoding: Support.
 * - Default:
 */
	uint32_t total_input_buf_cnt;

/**
 * Total processed buffer count. It means total encoded frame count or decoded
 * frame count.
 *
 * - Note:
 * - Encoding: Support.
 * - Decoding: Support.
 * - Default:
 */
	uint32_t total_output_buf_cnt;

/**
 * Camera pipeline ID. It's valid for video encoding.
 *
 * - Note:
 * - Encoding: Support.
 * - Decoding: Unsupport.
 * - Default:
 */
	int32_t pipeline;

/**
 * Camera pipeline channel port id. It's valid for video encoding.
 *
 * - Note:
 * - Encoding: Support.
 * - Decoding: Unsupport.
 * - Default:
 */
	int32_t channel_port_id;
} VDEC_CHN_STATUS_S;

typedef struct HB_VDEC_ATTR_H264_S {
  /**
   * Enable skip frame mode.
   * The valid numbers are as follows.
   *     0x00: normal DEC_PIC.
   *     0x01: skip non-IRAP.
   *     0x02: skip non-reference picture.
   *     0x03: thumbnail mode. It skips non-IRAP pictures w/o registering
   *           reference DPB.
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: 0
   */
  VIDEO_DEC_MODE_E enDecMode;
  /**
   * Support frame reordering. That is, the coded order may be different
   * from the presentation order of the corresponding frames.
   * The valid numbers are as follows.
   *     0 : disable reordering
   *     1 : enable reordering
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: 1;
   */
  VIDEO_OUTPUT_ORDER_E enOutputOrder;
  // uint32_t skip_mode;

  /**
   * Support bandwidth optimization feature which allows VPU to skip writing
   * compressed format of non-reference pictures or linear format of
   * non-display pictures to the frame buffer for BW saving reason.
   * The valid numbers are as follows.
   *     0 : disable bandwidth optimization
   *     1  : enable bandwidth optimization
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: 1
   */
  HB_BOOL bandwidth_Opt;
} VDEC_ATTR_H264_S;

typedef struct HB_VDEC_ATTR_H265_S {
  /**
   * Support frame reordering. That is, the coded order may be different
   * from the presentation order of the corresponding frames.
   * The valid numbers are as follows.
   *     0 : disable reordering
   *     1 : enable reordering
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: 1;
   */
  VIDEO_DEC_MODE_E enDecMode;
  /**
   * Skip frame function enable and operation mode
   * The valid numbers are as follows.
   *     0x00: normal DEC_PIC.
   *     0x01: skip non-IRAP.
   *     0x02: skip non-reference picture.
   *     0x03: thumbnail mode. It skips non-IRAP pictures w/o registering
   *           reference DPB.
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: 0
   */
  VIDEO_OUTPUT_ORDER_E enOutputOrder;

  /**
   * Handle CRA picture as BLA. It skips RASL pictures followd by
   * CRA pictures.
   * The valid numbers are as follows.
   *     0 : Don't handle CRA as BLA
   *     1  : Handle CRA as BLA
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: 0;
   */
  HB_BOOL cra_as_bla;

  /**
   * Support bandwidth optimization feature which allows VPU to skip writing
   * compressed format of non-reference pictures or linear format of
   * non-display pictures to the frame buffer for BW saving reason.
   * The valid numbers are as follows.
   *     0 : disable bandwidth optimization
   *     1 : enable bandwidth optimization
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: 1
   */
  HB_BOOL bandwidth_Opt;

  /**
   * Set the mode of temporal ID selection.
   * The valid numbers are as follows.
   *     0 : use the target temporal_id as absolute value.
   *         TARGET_DEC_TEMP_ID = TARGET_DEC_TEMP_ID_PLUS1 - 1
   *         When use of absolute value for temporal target, decoder can
   *         keep the decoding layer ID. If the SPS_MAX_SUB_LAYER is changed
   *         in the bitstream, the temporal skip ratio can be changed.
   *     1 : use the targe temporal_id as relative value.
   *         TARGET_DEC_TEMP_ID = SPS_MAX_SUB_LAYER - REL_TARGET_DEC_TEMP_ID
   *         SPS_MAX_SUB_LAYER is signalled from bitstream.
   *         When use of relative value decoder can keep the skip ratio
   *         regardless the cange of SPS_MAX_SUB_LAYER in the bitstream.
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: 0
   */
  uint32_t dec_temporal_id_mode;

  /**
   * If temporal_id_dec_mode is 0, thie field is used as an absolute target
   * temporal id(TARGET_DEC_TEMP_ID_PLUS1).
   * TARGET_DEC_TEMP_ID = TARGET_DEC_TEMP_ID_PLUS1 - 1
   * Base on TARGET_DEC_TEMP_ID,
   * - 0x0: it decodes a picture of all ranges of tempral ID, which means
   *        temporal ID decoding constraint off.
   * - 0x1~0x6: it decodes a picture if the temporal id is less than or equal
   *            to TARGET_DEC_TEMP_ID. It discards a pciture when its temporal
   *            id is greater than TARGET_DEC_TEMP_ID.
   *
   * If temporal_id_dec_mode is 1, the field is used as a relative target
   * temporal id(REL_TARGET_DEC_TEMP_ID)
   * TARGET_DEC_TEMP_ID = SPS_MAX_SUB_LAYER - REL_TARGET_DEC_TEMP_ID
   * It discards a picture when its temporal id is greater then
   * TARGET_DEC_TEMP_ID.
   * !!!TODO confirm this
   * Values[0,7]?
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: 1
   */
  uint32_t target_dec_temporal_id_plus1;
} VDEC_ATTR_H265_S;

typedef struct HB_VDEC_ATTR_MJPEG_S {
  /**
   * VDEC can rotate counterclockwise incoming pictures before starting
   * the ecode process. The decode process doesn't support rotation.
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: CODEC_ROTATION_0;
   */
  CODEC_ROTATION_E enRotation;

  /**
   * VDEC can mirror incoming pictures before starting the ecode process.
   * The decode process doesn't support mirror operation.
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: DIRECTION_NONE;
   */
  MIRROR_FLIP_E enMirrorFlip;

  VIDEO_CROP_INFO_S stCropCfg; /* the param of the crop */
} VDEC_ATTR_MJPEG_S;

typedef struct HB_VDEC_ATTR_JPEG_S {
  /**
   * VDEC can rotate counterclockwise incoming pictures before starting
   * the ecode process. The decode process doesn't support rotation.
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: CODEC_ROTATION_0;
   */
  CODEC_ROTATION_E enRotation;

  /**
   * VDEC can mirror incoming pictures before starting the ecode process.
   * The decode process doesn't support mirror operation.
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: DIRECTION_NONE;
   */
  MIRROR_FLIP_E enMirrorFlip;

  VIDEO_CROP_INFO_S stCropCfg; /* the param of the crop */
} VDEC_ATTR_JPEG_S;

typedef struct HB_VDEC_CHN_ATTR_S {
  PAYLOAD_TYPE_E enType; /* RW; video type to be decoded */
  VIDEO_MODE_E enMode;   /* RW; send by stream or by frame */
  // uint32_t u32PicWidth;  /* RW; max pic width */
  // uint32_t u32PicHeight; /* RW; max pic height */

  /**
   * Output pixel format.
   * !!!TODO confirm the available output format.
   * Values:YUV420,YUV422,YUV444 for MJPEG and jpeg
   * Values[MC_PIXEL_FORMAT_YUV420P,MC_PIXEL_FORMAT_NV12,MC_PIXEL_FORMAT_NV21]
   * for H264 and H265
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: MC_PIXEL_FORMAT_YUV420P
   */
  PIXEL_FORMAT_E enPixelFormat;

  /**
   * Specify the size of bitstream buffer. The buffers are internally
   * allocated by MediaCodec. It's size should be larger than the feeding
   * size. Usually, it should align with 1024.
   * Values[1024, 2^31-1]
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: 10*1024*1024
   */
  uint32_t u32StreamBufSize; /* RW; stream buf size(Byte) */

  /**
   * Specify the count of bitstream buffers.
   * Values[5,65536]
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: 5
   */
  uint32_t u32StreamBufCnt; /* RW; frame buf size(Byte) */

  /**
   * The value specifies that VDEC should using exteranl input bitstream
   * buffer.
   * The valid numbers are as follows.
   *     0 : use internal bitstream buffer
   *     1  : use external bitstream buffer
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: 0
   */
  HB_BOOL bExternalBitStreamBuf;

  /**
   * The size of FrameBuffer is decided by the MediaCodec according to the
   * sequence information. But users can specify the count of FrameBuffer
   * buffers. VPU may delay decoded picture display for display reordering
   * when H.264/H.265, pic_order_cnt_type 0 or 1 case and for B-frame
   * handling in VC1 decoder. If the specified count is less then the
   * required count, MediaCodec with H264/H265 will modify the
   * specified count to the value that the maximum display frame buffer
   * delay for buffering decoded picture reorder plus.
   * (extra frame buffer number(1) + 1). MediaCodec with MJPEG/JPEG will
   * choose at least 2 frame buffer.
   * Values[5,31]
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: 5
   */
  uint32_t u32FrameBufCnt; /* RW; frame buf num */

  uint32_t vlc_buf_size;

  union {
    VDEC_ATTR_H264_S stAttrH264;
    VDEC_ATTR_H265_S stAttrH265;
    VDEC_ATTR_MJPEG_S stAttrMjpeg;
    VDEC_ATTR_JPEG_S stAttrJpeg;
  };
} VDEC_CHN_ATTR_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef  __HB_COMM_VDEC_H__ */
