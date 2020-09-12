/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file:
 * @brief:
 * @author:
 * @email:
 * @date: 2019.12.10
 */

#ifndef __HB_COMM_VENC_H__
#define __HB_COMM_VENC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#include <stdint.h>
#include "hb_common.h"
#include "hb_type.h"
#include "./hb_errno.h"
#include "./hb_sys.h"
#include "hb_comm_video.h"
//#include "hb_comm_rc.h"

typedef enum {
	EN_ERR_VENC_UNKNOWN = 0,
    EN_ERR_VENC_NOT_FOUND = 1,
    EN_ERR_VENC_OPEN_FAIL = 2,
    EN_ERR_VENC_RESPONSE_TIMEOUT = 3,
    EN_ERR_VENC_INIT_FAIL = 4,
    EN_ERR_VENC_OPERATION_NOT_ALLOWDED = 5,
    EN_ERR_VENC_NOMEM = 6,
    EN_ERR_VENC_NO_FREE_CHANNEL = 7,
    EN_ERR_VENC_ILLEGAL_PARAM = 8,
    EN_ERR_VENC_INVALID_CHNID = 9,
    EN_ERR_VENC_INVALID_BUF = 10,
    EN_ERR_VENC_INVALID_CMD = 11,
    EN_ERR_VENC_WAIT_TIMEOUT = 12,
    EN_ERR_VENC_FILE_OPERATION_FAIL = 13,
    EN_ERR_VENC_PARAMS_SET_FAIL = 14,
    EN_ERR_VENC_PARAMS_GET_FAIL = 15,
    EN_ERR_VENC_EXIST = 16,
    EN_ERR_VENC_UNEXIST = 17,
    EN_ERR_VENC_NULL_PTR = 18,
    EN_ERR_VENC_UNSUPPORT = 19
} EN_VENC_ERR_CODE_E;

/* Venc unknown error ---->0x1007FC00(268958720) */
#define HB_ERR_VENC_UNKNOWN		                                 \
    HB_DEF_ERR(HB_ID_VENC, EN_ERR_VENC_UNKNOWN)
/* Venc device not found */
#define HB_ERR_VENC_NOT_FOUND                                    \
    HB_DEF_ERR(HB_ID_VENC, EN_ERR_VENC_NOT_FOUND)
/* Failed to open venc device */
#define HB_ERR_VENC_OPEN_FAIL                                    \
    HB_DEF_ERR(HB_ID_VENC, EN_ERR_VENC_OPEN_FAIL)
/* Timeout to operate venc device */
#define HB_ERR_VENC_RESPONSE_TIMEOUT                             \
    HB_DEF_ERR(HB_ID_VENC, EN_ERR_VENC_RESPONSE_TIMEOUT)
/* Failed to initialize venc device */
#define HB_ERR_VENC_INIT_FAIL                                    \
    HB_DEF_ERR(HB_ID_VENC, EN_ERR_VENC_INIT_FAIL)
/* Operation is not allowed */
#define HB_ERR_VENC_OPERATION_NOT_ALLOWDED                       \
    HB_DEF_ERR(HB_ID_VENC, EN_ERR_VENC_OPERATION_NOT_ALLOWDED)
/* Venc memory is not insufficient */
#define HB_ERR_VENC_NOMEM                                        \
    HB_DEF_ERR(HB_ID_VENC, EN_ERR_VENC_NOMEM)
/* No free venc channel left */
#define HB_ERR_VENC_NO_FREE_CHANNEL                              \
    HB_DEF_ERR(HB_ID_VENC, EN_ERR_VENC_NO_FREE_CHANNEL)
/* Invalid channel params */
#define HB_ERR_VENC_ILLEGAL_PARAM                                \
    HB_DEF_ERR(HB_ID_VENC, EN_ERR_VENC_ILLEGAL_PARAM)
/* Invalid channel ID */
#define HB_ERR_VENC_INVALID_CHNID                                \
    HB_DEF_ERR(HB_ID_VENC, EN_ERR_VENC_INVALID_CHNID)
/* Invalid buffer */
#define HB_ERR_VENC_INVALID_BUF                                  \
    HB_DEF_ERR(HB_ID_VENC, EN_ERR_VENC_INVALID_BUF)
/* Invalid command */
#define HB_ERR_VENC_INVALID_CMD                                  \
    HB_DEF_ERR(HB_ID_VENC, EN_ERR_VENC_INVALID_CMD)
/* Wait timeout */
#define HB_ERR_VENC_WAIT_TIMEOUT                                 \
    HB_DEF_ERR(HB_ID_VENC, EN_ERR_VENC_WAIT_TIMEOUT)
/* File cannot be operated successfully */
#define HB_ERR_VENC_FILE_OPERATION_FAIL                          \
    HB_DEF_ERR(HB_ID_VENC, EN_ERR_VENC_FILE_OPERATION_FAIL)
/* Failed to set parameters */
#define HB_ERR_VENC_PARAMS_SET_FAIL                              \
    HB_DEF_ERR(HB_ID_VENC, EN_ERR_VENC_PARAMS_SET_FAIL)
/* Failed to get parameters */
#define HB_ERR_VENC_PARAMS_GET_FAIL                              \
    HB_DEF_ERR(HB_ID_VENC, EN_ERR_VENC_PARAMS_GET_FAIL)
/* Venc channel exist */
#define HB_ERR_VENC_EXIST                                        \
    HB_DEF_ERR(HB_ID_VENC, EN_ERR_VENC_EXIST)
/* Venc channel unexist */
#define HB_ERR_VENC_UNEXIST                                      \
    HB_DEF_ERR(HB_ID_VENC, EN_ERR_VENC_UNEXIST)
/* Venc channel null ptr ---->0x1007FC12(268958738)*/
#define HB_ERR_VENC_NULL_PTR                                     \
    HB_DEF_ERR(HB_ID_VENC, EN_ERR_VENC_NULL_PTR)
/* upsupport api */
#define HB_ERR_VENC_UNSUPPORT                                    \
    HB_DEF_ERR(HB_ID_VENC, EN_ERR_VENC_UNSUPPORT)
/**
* Define the H264 profile.
**/
typedef enum HB_VENC_H264_PROFILE_E {
	HB_H264_PROFILE_UNSPECIFIED,
	HB_H264_PROFILE_BP,
	HB_H264_PROFILE_MP,
	HB_H264_PROFILE_EXTENDED,
	HB_H264_PROFILE_HP,
	HB_H264_PROFILE_HIGH10,
	HB_H264_PROFILE_HIGH422,
	HB_H264_PROFILE_HIGHT444
} VENC_H264_PROFILE_E;

/**
* Define the H264 level.
**/
typedef enum HB_VENC_H264_LEVEL {
	HB_H264_LEVEL_UNSPECIFIED,
	HB_H264_LEVEL1 = 10,
	HB_H264_LEVEL1b = 9,
	HB_H264_LEVEL1_1 = 11,
	HB_H264_LEVEL1_2 = 12,
	HB_H264_LEVEL1_3 = 13,
	HB_H264_LEVEL2 = 20,
	HB_H264_LEVEL2_1 = 21,
	HB_H264_LEVEL2_2 = 22,
	HB_H264_LEVEL3 = 30,
	HB_H264_LEVEL3_1 = 31,
	HB_H264_LEVEL3_2 = 32,
	HB_H264_LEVEL4 = 40,
	HB_H264_LEVEL4_1 = 41,
	HB_H264_LEVEL4_2 = 42,
	HB_H264_LEVEL5 = 50,
	HB_H264_LEVEL5_1 = 51,
	HB_H264_LEVEL5_2 = 52,
} HB_H264_LEVEL_E;

/**
* Define the H265 level.
**/
typedef enum HB_VENC_H265_LEVEL {
	HB_H265_LEVEL_UNSPECIFIED,
	HB_H265_LEVEL1 = 30,
	HB_H265_LEVEL2 = 60,
	HB_H265_LEVEL2_1 = 63,
	HB_H265_LEVEL3 = 90,
	HB_H265_LEVEL3_1 = 93,
	HB_H265_LEVEL4 = 120,
	HB_H265_LEVEL4_1 = 123,
	HB_H265_LEVEL5 = 150,
	HB_H265_LEVEL5_1 = 153,
} HB_H265_LEVEL_E;


/******************************VENC_ATTR_S **********************************/
typedef struct HB_VENC_CHN_STATUS_S {
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
} VENC_CHN_STATUS_S;

typedef struct HB_VENC_ATTR_H264_S {
/**
 * User add profile information to SPS by setting the profile register.
 * However, if you set 0 or have done nothing to the register, VPU
 * automatically encodes a profile by using the bit depth of source picture.
 *
 * - Note: It's unchangable parameters in the same sequence.
 * - Encoding: Unsupport.
 * - Decoding: Unsupport.
 * - Default: HB_H264_PROFILE_UNSPECIFIED;
 */
	VENC_H264_PROFILE_E h264_profile;

  HB_H264_LEVEL_E h264_level;
} VENC_ATTR_H264_S;

typedef struct HB_VENC_ATTR_H265_S {
  HB_BOOL main_still_picture_profile_enable;  //  ???
  int32_t s32h265_tier;                       //设置H265 tier信息   ???
  HB_BOOL transform_skip_enabled_flag;
  uint32_t lossless_mode;  //开启无损编码模式
  uint32_t tmvp_Enable;    //使能temporal motion vector prediction
  uint32_t wpp_Enable;     //
  HB_H265_LEVEL_E h265_level;
} VENC_ATTR_H265_S;

typedef struct HB_VENC_ATTR_MJPEG_S {
  uint32_t restart_interval;  // 指定在一个独立的扫描序列中包含的MCU的个数
	HB_BOOL huff_table_valid;
	uint8_t huff_luma_dc_bits[16];
	uint8_t huff_luma_dc_val[16];
	uint8_t huff_luma_ac_bits[16];
	uint8_t huff_luma_ac_val[256];
	uint8_t huff_chroma_dc_bits[16];
	uint8_t huff_chroma_ac_bits[16];
	uint8_t huff_chroma_dc_val[16];
	uint8_t huff_chroma_ac_val[256];
	HB_BOOL extended_sequential;
} VENC_ATTR_MJPEG_S;

typedef struct HB_VENC_ATTR_JPEG_S {
  HB_BOOL dcf_enable;
  uint32_t restart_interval;  // 指定在一个独立的扫描序列中包含的MCU的个数
  uint32_t quality_factor;
  HB_BOOL huff_table_valid;
	uint8_t huff_luma_dc_bits[16];
	uint8_t huff_luma_dc_val[16];
	uint8_t huff_luma_ac_bits[16];
	uint8_t huff_luma_ac_val[256];
	uint8_t huff_chroma_dc_bits[16];
	uint8_t huff_chroma_ac_bits[16];
	uint8_t huff_chroma_dc_val[16];
	uint8_t huff_chroma_ac_val[256];
	HB_BOOL extended_sequential;
} VENC_ATTR_JPEG_S;

typedef struct HB_VENC_ATTR_S {
  PAYLOAD_TYPE_E enType; /* RW; the type of payload*/
  uint32_t u32PicWidth;  /* RW; width of a picture to be encoded, in pixel*/
  uint32_t u32PicHeight; /* RW; height of a picture to be encoded, in pixel*/

  /**
   * Pixel format.
   * !!!TODO confirm the input format.
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: 0
   */
  PIXEL_FORMAT_E enPixelFormat;

  /**
   * The number of input FrameBuffer.
   * Values[0,65536]
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: 5
   */
  uint32_t u32FrameBufferCount;
  /**
   * Specify the count of bitstream buffers.
   * Values[0,65536]
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: 5
   */
  uint32_t u32BitStreamBufferCount;
  /**
   * The value specifies that VENC should using exteranl input frame
   * buffer.
   * The valid numbers are as follows.
   *     0 : use internal frame buffer
   *     1  : use external frame buffer
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: 0
   */
  HB_BOOL bExternalFreamBuffer;

  /**
 * Specify the size of bitstream buffers. It should align with 1024.
 * Values[64*1024, 2^31-1] for H264/H265
 * Values[8*1024, 2^31-1] for MJPEG/JPEG
 * Values 0 means codec calculates the size.
 *
 * - Note: It's unchangable parameters in the same sequence.
 * - Encoding: Unsupport.
 * - Decoding: Support.
 * - Default: 10*1024*1024
 */
	uint32_t u32BitStreamBufSize;

  /**
   * VENC can rotate counterclockwise incoming pictures before starting
   * the ecode process. The decode process doesn't support rotation.
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: CODEC_ROTATION_0;
   */
  CODEC_ROTATION_E enRotation;

  /**
   * VENC can mirror incoming pictures before starting the ecode process.
   * The decode process doesn't support mirror operation.
   *
   * - Note: It's unchangable parameters in the same sequence.
   * - Default: DIRECTION_NONE;
   */
  MIRROR_FLIP_E enMirrorFlip;

  VIDEO_CROP_INFO_S stCropCfg; /* the param of the crop */

  /**
 * Set user pts. Using user frame buffer pts to set the stream pts.
 *
 * - Note: It's unchangable RC parameters.
 *		   It's only useful for H264, H265
 * - Encoding: Support.
 * - Decoding: Unsupport.
 * - Default: FALSE
 */
	HB_BOOL bEnableUserPts;

  uint32_t vlc_buf_size;

  union {
    VENC_ATTR_H264_S stAttrH264;
    VENC_ATTR_H265_S stAttrH265;
    VENC_ATTR_MJPEG_S stAttrMjpeg;
    VENC_ATTR_JPEG_S stAttrJpeg;
  };
} VENC_ATTR_S;

/******************************VENC_ATTR_S  end ******************************/

/******************************VENC_RC_ATTR_S*********************************/
typedef enum HB_VENC_RC_MODE_E {
  VENC_RC_MODE_NONE = -1,
  VENC_RC_MODE_H264CBR,
  VENC_RC_MODE_H264VBR,
  VENC_RC_MODE_H264AVBR,
  VENC_RC_MODE_H264FIXQP,
  VENC_RC_MODE_H264QPMAP,
  VENC_RC_MODE_H265CBR,
  VENC_RC_MODE_H265VBR,
  VENC_RC_MODE_H265AVBR,
  VENC_RC_MODE_H265FIXQP,
  VENC_RC_MODE_H265QPMAP,
  VENC_RC_MODE_MJPEGFIXQP,
  VENC_RC_MODE_BUTT,
} VENC_RC_MODE_E;

typedef struct HB_VENC_H264_CBR_S {
  /**
   * I frame interval.
   * Values[0,65536]
   *
   * - Note: It's changeable parameter in the same sequence.
   *         !!!TODO confirm this.
   *         It's related with Gop size.
   * - Default: 28
   */
  uint32_t u32IntraPeriod;

  uint32_t u32IntraQp;

  /**
   * The target average bitrate of the encoded data in kbps.
   * Values[0,700000]kps
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 0
   */
  uint32_t u32BitRate;

  /**
   * The target frame rate of the encoded data in fps.
   * Values[0,65536]fps
   *
   * - Note: It's unchangable parameter in the same sequence.
   * - Default: 0
   */
  uint32_t u32FrameRate;
  uint32_t u32InitialRcQp;
  uint32_t u32VbvBufferSize;
  HB_BOOL bMbLevelRcEnable;

  /**
   * A minimum QP of I picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 8
   */
  uint32_t u32MinIQp;

  /**
   * A maximum QP of I picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 51
   */
  uint32_t u32MaxIQp;

  /**
   * A minimum QP of P picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 8
   */
  uint32_t u32MinPQp;

  /**
   * A maximum QP of P picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 51
   */
  uint32_t u32MaxPQp;

  /**
   * A minimum QP of B picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 8
   */
  uint32_t u32MinBQp;

  /**
   * A maximum QP of B picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 51
   */
  uint32_t u32MaxBQp;

  HB_BOOL bHvsQpEnable;
  /**
   * A QP scaling factor for subCTU QP adjustment when hvs_qp_enable is 1.
   * Values[0,4]
   *!!!TODO confirm this for h264?
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 2
   */
  int32_t s32HvsQpScale;

  /**
   * Specifies maximum delta QP of HVS QP. (0 ~ 51) This value is valid
   * when hvs_qp_enable is 1.
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 10
   */
  uint32_t u32MaxDeltaQp;

  /**
   * Enables or disables QP map.
   * The valid numbers are as follows.
   *        0 : disable
   *        1 : enable
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 0
   */
  HB_BOOL bQpMapEnable;
} VENC_H264_CBR_S;

typedef struct hiVENC_PARAM_H264_CBR_S {
} VENC_PARAM_H264_CBR_S;

typedef struct HB_VENC_H264_VBR_S {
  /**
   * I frame interval.
   * Values[0,65536]
   *
   * - Note: It's changeable parameter in the same sequence.
   *         !!!TODO confirm this.
   *         It's related with Gop size.
   * - Default: 28
   */
  uint32_t u32IntraPeriod;

  /**
   * A quantization parameter of intra picture.
   * Values[0,51]
   *
   * - Note: It's changeable parameter in the same sequence.
   *         !!!TODO confirm this.
   * - Default: 0
   */
  uint32_t u32IntraQp;

  /**
   * The target frame rate of the encoded data in fps.
   * Values[0,65536]fps
   *
   * - Note: It's unchangable parameter in the same sequence.
   * - Default: 0
   */
  uint32_t u32FrameRate;

  /**
   * Enables or disables QP map.
   * The valid numbers are as follows.
   *        0 : disable
   *        1 : enable
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 0
   */
  HB_BOOL bQpMapEnable;
} VENC_H264_VBR_S;

typedef struct HB_VENC_H264_AVBR_S {
  /**
   * I frame interval.
   * Values[0,65536]
   *
   * - Note: It's changeable parameter in the same sequence.
   *         !!!TODO confirm this.
   *         It's related with Gop size.
   * - Default: 28
   */
  uint32_t u32IntraPeriod;

  /**
   * A quantization parameter of intra picture.
   * Values[0,51]
   *
   * - Note: It's changeable parameter in the same sequence.
   *         !!!TODO confirm this.
   * - Default: 0
   */
  uint32_t u32IntraQp;

  /**
   * The target average bitrate of the encoded data in kbps.
   * Values[0,700000]kps
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 0
   */
  uint32_t u32BitRate;

  /**
   * The target frame rate of the encoded data in fps.
   * Values[0,65536]fps
   *
   * - Note: It's unchangable parameter in the same sequence.
   * - Default: 0
   */
  uint32_t u32FrameRate;
  uint32_t u32InitialRcQp;
  uint32_t u32VbvBufferSize;
  HB_BOOL bMbLevelRcEnable;
  /**
   * A minimum QP of I picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 8
   */
  uint32_t u32MinIQp;

  /**
   * A maximum QP of I picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 51
   */
  uint32_t u32MaxIQp;

  /**
   * A minimum QP of P picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 8
   */
  uint32_t u32MinPQp;

  /**
   * A maximum QP of P picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 51
   */
  uint32_t u32MaxPQp;

  /**
   * A minimum QP of B picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 8
   */
  uint32_t u32MinBQp;

  /**
   * A maximum QP of B picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 51
   */
  uint32_t u32MaxBQp;

  HB_BOOL bHvsQpEnable;

  /**
   * A QP scaling factor for subCTU QP adjustment when hvs_qp_enable is 1.
   * Values[0,4]
   *!!!TODO confirm this for h264?
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 2
   */
  int32_t s32HvsQpScale;

  /**
   * Specifies maximum delta QP of HVS QP. (0 ~ 51) This value is valid
   * when hvs_qp_enable is 1.
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 10
   */
  uint32_t u32MaxDeltaQp;

  /**
   * Enables or disables QP map.
   * The valid numbers are as follows.
   *        0 : disable
   *        1 : enable
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 0
   */
  HB_BOOL bQpMapEnable;
} VENC_H264_AVBR_S;

typedef struct HB_VENC_PARAM_H264_AVBR_S {
} VENC_PARAM_H264_AVBR_S;

typedef struct HB_VENC_H264_FIXQP_S {
  /**
   * I frame interval.
   * Values[0,65536]
   *
   * - Note: It's changeable parameter in the same sequence.
   *         !!!TODO confirm this.
   *         It's related with Gop size.
   * - Default: 28
   */
  uint32_t u32IntraPeriod;

  /**
   * The target frame rate of the encoded data in fps.
   * Values[0,65536]fps
   *
   * - Note: It's unchangable parameter in the same sequence.
   * - Default: 0
   */
  uint32_t u32FrameRate;

  /**
   * A force picture quantization parameter for I picture.
   * Values[0,51]
   *
   * - Note: It's changable parameter in the same sequence.
   * - Default: 0
   */
  uint32_t u32IQp;

  /**
   * A force picture quantization parameter for P picture.
   * Values[0,51]
   *
   * - Note: It's changable parameter in the same sequence.
   * - Default: 0
   */
  uint32_t u32PQp;

  /**
   * A force picture quantization parameter for B picture.
   * Values[0,51]
   *
   * - Note: It's changable parameter in the same sequence.
   * - Default: 0
   */
  uint32_t u32BQp;
} VENC_H264_FIXQP_S;

typedef struct HB_VENC_H264_QPMAP_S {
  /**
   * I frame interval.
   * Values[0,65536]
   *
   * - Note: It's changeable parameter in the same sequence.
   *         !!!TODO confirm this.
   *         It's related with Gop size.
   * - Default: 28
   */
  uint32_t u32IntraPeriod;

  /**
   * The target frame rate of the encoded data in fps.
   * Values[0,65536]fps
   *
   * - Note: It's unchangable parameter in the same sequence.
   * - Default: 0
   */
  uint32_t u32FrameRate;

  /**
   * Specify the qp map. The QP map array should be written a series
   * of 1 byte QP values for each Macroblock in raster scan order.
   * The H264 Macroblock size is 16x16.
   * QP Values[0~51]
   *
   * - Note: It's changable parameter in the same sequence.
   * - Default: NULL
   */
  unsigned char* u32QpMapArray;

  /**
   * Specify the qp map number. It's related with the picture width
   * and height.
   * Values[1,VIDEO_MAX_MB_NUM]
   *
   * - Note: It's changable parameter in the same sequence
   * - Default: 0
   */
  uint32_t u32QpMapArrayCount;
} VENC_H264_QPMAP_S;

typedef struct HB_VENC_H265_CBR_S {
  /**
   * I frame interval.
   * Values[0,65536]
   *
   * - Note: It's changeable parameter in the same sequence.
   *         !!!TODO confirm this.
   *         It's related with Gop size.
   * - Default: 28
   */
  uint32_t u32IntraPeriod;

  /**
   * A quantization parameter of intra picture.
   * Values[0,51]
   *
   * - Note: It's changeable parameter in the same sequence.
   *         !!!TODO confirm this.
   * - Default: 0
   */
  uint32_t u32IntraQp;

  /**
   * The target average bitrate of the encoded data in kbps.
   * Values[0,700000]kps
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 0
   */
  uint32_t u32BitRate;

  /**
   * The target frame rate of the encoded data in fps.
   * Values[0,65536]fps
   *
   * - Note: It's unchangable parameter in the same sequence.
   * - Default: 0
   */
  uint32_t u32FrameRate;
  uint32_t u32InitialRcQp;
  uint32_t u32VbvBufferSize;
  HB_BOOL bCtuLevelRcEnable;
  /**
   * A minimum QP of I picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 8
   */
  uint32_t u32MinIQp;

  /**
   * A maximum QP of I picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 51
   */
  uint32_t u32MaxIQp;

  /**
   * A minimum QP of P picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 8
   */
  uint32_t u32MinPQp;

  /**
   * A maximum QP of P picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 51
   */
  uint32_t u32MaxPQp;

  /**
   * A minimum QP of B picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 8
   */
  uint32_t u32MinBQp;

  /**
   * A maximum QP of B picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 51
   */
  uint32_t u32MaxBQp;
  HB_BOOL bHvsQpEnable;

  /**
   * A QP scaling factor for subCTU QP adjustment when hvs_qp_enable is 1.
   * Values[0,4]
   *!!!TODO confirm this for h264?
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 2
   */
  int32_t s32HvsQpScale;

  /**
   * Specifies maximum delta QP of HVS QP. (0 ~ 51) This value is valid
   * when hvs_qp_enable is 1.
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 10
   */
  uint32_t u32MaxDeltaQp;

  /**
   * Enables or disables QP map.
   * The valid numbers are as follows.
   *        0 : disable
   *        1 : enable
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 0
   */
  HB_BOOL bQpMapEnable;
} VENC_H265_CBR_S;

typedef struct HB_VENC_PARAM_H265_CBR_S {
} VENC_PARAM_H265_CBR_S;

typedef struct HB_VENC_H265_VBR_S {
  /**
   * I frame interval.
   * Values[0,65536]
   *
   * - Note: It's changeable parameter in the same sequence.
   *         !!!TODO confirm this.
   *         It's related with Gop size.
   * - Default: 28
   */
  uint32_t u32IntraPeriod;

  /**
   * A quantization parameter of intra picture.
   * Values[0,51]
   *
   * - Note: It's changeable parameter in the same sequence.
   *         !!!TODO confirm this.
   * - Default: 0
   */
  uint32_t u32IntraQp;

  /**
   * The target frame rate of the encoded data in fps.
   * Values[0,65536]fps
   *
   * - Note: It's unchangable parameter in the same sequence.
   * - Default: 0
   */
  uint32_t u32FrameRate;

  /**
   * Enables or disables QP map.
   * The valid numbers are as follows.
   *        0 : disable
   *        1 : enable
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 0
   */
  HB_BOOL bQpMapEnable;
} VENC_H265_VBR_S;

typedef struct HB_VENC_H265_AVBR_S {
  /**
   * I frame interval.
   * Values[0,65536]
   *
   * - Note: It's changeable parameter in the same sequence.
   *         !!!TODO confirm this.
   *         It's related with Gop size.
   * - Default: 28
   */
  uint32_t u32IntraPeriod;

  /**
   * A quantization parameter of intra picture.
   * Values[0,51]
   *
   * - Note: It's changeable parameter in the same sequence.
   *         !!!TODO confirm this.
   * - Default: 0
   */
  uint32_t u32IntraQp;

  /**
   * The target average bitrate of the encoded data in kbps.
   * Values[0,700000]kps
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 0
   */
  uint32_t u32BitRate;

  /**
   * The target frame rate of the encoded data in fps.
   * Values[0,65536]fps
   *
   * - Note: It's unchangable parameter in the same sequence.
   * - Default: 0
   */
  uint32_t u32FrameRate;

  /**
   * Specifies the initial QP by user. If this value is smaller than 0 or
   * larger than 51, the initial QP is decided by F/W.
   * Values[0~51]
   *
   * - Note: It's unchangable RC parameter in the same sequence.
   * - Default: 63
   */
  uint32_t u32InitialRcQp;

  uint32_t u32VbvBufferSize;
  HB_BOOL bCtuLevelRcEnable;
  /**
   * A minimum QP of I picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 8
   */
  uint32_t u32MinIQp;

  /**
   * A maximum QP of I picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 51
   */
  uint32_t u32MaxIQp;

  /**
   * A minimum QP of P picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 8
   */
  uint32_t u32MinPQp;

  /**
   * A maximum QP of P picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 51
   */
  uint32_t u32MaxPQp;

  /**
   * A minimum QP of B picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 8
   */
  uint32_t u32MinBQp;

  /**
   * A maximum QP of B picture for rate control
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 51
   */
  uint32_t u32MaxBQp;

  /**
   * Enables or disables CU QP derivation based on CU variance. It can
   * enable CU QP adjustment for subjective quality enhancement.
   * The valid numbers are as follows.
   *        0 : disable
   *        1 : enable
   *!!!TODO confirm this for h264?
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 1
   */
  HB_BOOL bHvsQpEnable;

  /**
   * A QP scaling factor for subCTU QP adjustment when hvs_qp_enable is 1.
   * Values[0,4]
   *!!!TODO confirm this for h264?
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 2
   */
  int32_t s32HvsQpScale;

  /**
   * Specifies maximum delta QP of HVS QP. (0 ~ 51) This value is valid
   * when hvs_qp_enable is 1.
   * Values[0,51]
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 10
   */
  uint32_t u32MaxDeltaQp;

  /**
   * Enables or disables QP map.
   * The valid numbers are as follows.
   *        0 : disable
   *        1 : enable
   *
   * - Note: It's changable RC parameter in the same sequence.
   * - Default: 0
   */
  HB_BOOL bQpMapEnable;
} VENC_H265_AVBR_S;

typedef struct HB_VENC_PARAM_H265_AVBR_S {
} VENC_PARAM_H265_AVBR_S;

typedef struct HB_VENC_H265_FIXQP_S {
  /**
   * I frame interval.
   * Values[0,65536]
   *
   * - Note: It's changeable parameter in the same sequence.
   *         !!!TODO confirm this.
   *         It's related with Gop size.
   * - Default: 28
   */
  uint32_t u32IntraPeriod;

  /**
   * The target frame rate of the encoded data in fps.
   * Values[0,65536]fps
   *
   * - Note: It's unchangable parameter in the same sequence.
   * - Default: 0
   */
  uint32_t u32FrameRate;

  /**
   * A force picture quantization parameter for I picture.
   * Values[0,51]
   *
   * - Note: It's changable parameter in the same sequence.
   * - Default: 0
   */
  uint32_t u32IQp;

  /**
   * A force picture quantization parameter for P picture.
   * Values[0,51]
   *
   * - Note: It's changable parameter in the same sequence.
   * - Default: 0
   */
  uint32_t u32PQp;

  /**
   * A force picture quantization parameter for B picture.
   * Values[0,51]
   *
   * - Note: It's changable parameter in the same sequence.
   * - Default: 0
   */
  uint32_t u32BQp;
} VENC_H265_FIXQP_S;

typedef struct HB_VENC_H265_QPMAP_S {
  /**
   * I frame interval.
   * Values[0,65536]
   *
   * - Note: It's changeable parameter in the same sequence.
   *         !!!TODO confirm this.
   *         It's related with Gop size.
   * - Default: 28
   */
  uint32_t u32IntraPeriod;

  /**
   * The target frame rate of the encoded data in fps.
   * Values[0,65536]fps
   *
   * - Note: It's unchangable parameter in the same sequence.
   * - Default: 0
   */
  uint32_t u32FrameRate;

  /**
   * Specify the qp map. The QP map array should be written a series
   * of 1 byte QP values for each Macroblock in raster scan order.
   * The H264 Macroblock size is 16x16.
   * QP Values[0~51]
   *
   * - Note: It's changable parameter in the same sequence.
   * - Default: NULL
   */
  unsigned char* u32QpMapArray;

  /**
   * Specify the qp map number. It's related with the picture width
   * and height.
   * Values[1,VIDEO_MAX_MB_NUM]
   *
   * - Note: It's changable parameter in the same sequence
   * - Default: 0
   */
  uint32_t u32QpMapArrayCount;
} VENC_H265_QPMAP_S;

typedef struct HB_VENC_MJPEG_FIXQP_S {
  /**
   * The target frame rate of the encoded data in fps.
   * Values[0,65536]fps
   *
   * - Note:
   * - Default: 0
   */
  uint32_t u32FrameRate;

  /**
   * Quality factor. Qualities 50..100 are converted to scaling percentage
   * 200 - 2*Q. Note that at Q=100 the scaling is 0, it will cause minimum
   * quantization loss and low compressibility. Qualities 1..50 are converted
   * to scaling percentage 5000/Q. Note that at Q=1 the scaling is 5000,
   * it will cause maximun quantization loss and high compressibility.
   * Values[1,100]
   *
   * - Note: It's changable parameters in the same sequence.
   * - Default: 50;
   */
  uint32_t u32QualityFactort;
} VENC_MJPEG_FIXQP_S;

typedef struct HB_VENC_RC_ATTR_S {
  VENC_RC_MODE_E enRcMode;
  union {
    VENC_H264_CBR_S stH264Cbr;
    VENC_H264_VBR_S stH264Vbr;
    VENC_H264_AVBR_S stH264AVbr;
    VENC_H264_FIXQP_S stH264FixQp;
    VENC_H264_QPMAP_S stH264QpMap;

    VENC_H265_CBR_S stH265Cbr;
    VENC_H265_VBR_S stH265Vbr;
    VENC_H265_AVBR_S stH265AVbr;
    VENC_H265_FIXQP_S stH265FixQp;
    VENC_H265_QPMAP_S stH265QpMap;

    VENC_MJPEG_FIXQP_S stMjpegFixQp;
  };
} VENC_RC_ATTR_S;

typedef struct HB_VENC_USER_RC_ATTR_S {
  /**
   *.
   * Specify the validation of qp map.
   *	   0 : invalid
   *	   1  : valid
   *
   * - Note:
   * - Encoding: Support.
   * - Decoding: Unsupport.
   * - Default: 0.
   */
  HB_BOOL qp_map_valid;

  /**
   * Specify the qp map. The QP map array should be written a series
   * of 1 byte QP values for each Macroblock in raster scan order.
   * The H264 Macroblock size is 16x16.
   * QP Values[0~51]
   *
   * - Note: It's changable parameter in the same sequence.
   * - Encoding: Support.
   * - Decoding: Unsupport.
   * - Default: NULL
   */
  unsigned char *qp_map_array;

  /**
   * Specify the qp map number. It's related with the picture width
   * and height.
   * Values[1,VIDEO_MAX_MB_NUM]
   *
   * - Note: It's changable parameter in the same sequence
   * - Encoding: Support.
   * - Decoding: Unsupport.
   * - Default: 0
   */
  uint32_t qp_map_array_count;
} VENC_USER_RC_ATTR_S;

/************************** VENC_RC_ATTR_S  end ******************************/

/*****************************VENC_GOP_ATTR_S*********************************/
typedef struct HB_VENC_GOP_PICTURE_CUSTOM_S {
  uint32_t u32PictureType;
  int32_t s32PocOffset;
  uint32_t u32PictureQp;
  int32_t s32NumRefPictureL0;
  int32_t s32RefPocL0;
  int32_t s32RefPocL1;
  uint32_t u32TemporalId;
} VENC_GOP_PICTURE_CUSTOM_S;

#define MC_MAX_GOP_NUM 8
#define CUSTOM_MAX_GOP_NUM MC_MAX_GOP_NUM

typedef struct HB_VENC_GOP_CUSTOM_S {
  uint32_t u32CustomGopSize;
  VENC_GOP_PICTURE_CUSTOM_S stCustomGopPicture[CUSTOM_MAX_GOP_NUM];
} VENC_GOP_CUSTOM_S;

typedef struct HB_VENC_GOP_ATTR_S {
  int32_t s32DecodingRefreshType;
  uint32_t u32GopPresetIdx;
  VENC_GOP_CUSTOM_S stCustomGopParam;
} VENC_GOP_ATTR_S;

/***************************VENC_GOP_ATTR_S  end *****************************/

typedef struct HB_VENC_CHN_ATTR_S {
  VENC_ATTR_S stVencAttr;    /*the attribute of video encoder*/
  VENC_RC_ATTR_S stRcAttr;   /*the attribute of rate  ctrl*/
  VENC_GOP_ATTR_S stGopAttr; /*the attribute of gop*/
} VENC_CHN_ATTR_S;

typedef enum HB_VENC_INTRA_REFRESH_MODE_E {
  INTRA_REFRESH_ROW = 0,
  INTRA_REFRESH_COLUMN,
  INTRA_REFRESH_STEP_SIZE,  //海思不支持
  INTRA_REFRESH_ADAPTIVE,   //海思不支持
  INTRA_REFRESH_BUTT
} VENC_INTRA_REFRESH_MODE_E;

typedef struct HB_VENC_INTRA_REFRESH_S {
  HB_BOOL
  bRefreshEnable; /*RW; Range:[0,1]; intra refresh enable, HI_TRUE:enable,*/
  VENC_INTRA_REFRESH_MODE_E enIntraRefreshMode;
  uint32_t u32RefreshNum;
  // uint32_t u32ReqIQp; //X3不支持
} VENC_INTRA_REFRESH_S;

typedef struct HB_VENC_ROI_ATTR_S {
  /**
   * It enablea ROI encoding throughout the sequence level.
   * The valid numbers are as follows.
   *     0 : disable
   *     1 : enable
   *
   * - Note: It's unchangable RC parameter in the same sequence.
   *         It's valid when rate control is on.
   * - Default: 0
   */
  uint32_t roi_enable;

  /**
   * Specify the ROI map. The ROI map array should be written a series
   * of 1 byte QP values for each Macroblock in raster scan order.
   * Values[0~51]
   *
   * - Note: It's changable parameter in the same sequence.
   *         It's valid only when roi_enable = 1.
   * - Default: 0
   */
  uint8_t* roi_map_array;

  /**
   * Specify the ROI map number.
   * Values[1~VIDEO_MAX_MB_NUM] for h264
   * Values[1~VIDEO_MAX_SUB_CTU_NUM] for h265
   *
   * - Note: It's unchangable parameter in the same sequence.
   *         It's valid only when roi_enable = 1.
   * - Default: 0
   */
  uint32_t roi_map_array_count;
} VENC_ROI_ATTR_S;

typedef struct HB_VENC_CU_PREDICTION_S {
  int32_t mode_decision_enable;
  uint32_t pu04_delta_rate;  //模式选择
  uint32_t pu08_delta_rate;
  uint32_t pu16_delta_rate;
  uint32_t pu32_delta_rate;
  uint32_t pu04_intra_planar_delta_rate;
  uint32_t pu04_intra_dc_delta_rate;
  uint32_t pu04_intra_angle_delta_rate;
  uint32_t pu08_intra_planar_delta_rate;
  uint32_t pu08_intra_dc_delta_rate;
  uint32_t pu08_intra_angle_delta_rate;
  uint32_t pu16_intra_planar_delta_rate;
  uint32_t pu16_intra_dc_delta_rate;
  uint32_t pu16_intra_angle_delta_rate;
  uint32_t pu32_intra_planar_delta_rate;
  uint32_t pu32_intra_dc_delta_rate;
  uint32_t pu32_intra_angle_delta_rate;
  uint32_t cu08_intra_delta_rate;
  uint32_t cu08_inter_delta_rate;
  uint32_t cu08_merge_delta_rate;
  uint32_t cu16_intra_delta_rate;
  uint32_t cu16_inter_delta_rate;
  uint32_t cu16_merge_delta_rate;
  uint32_t cu32_intra_delta_rate;
  uint32_t cu32_inter_delta_rate;
  uint32_t cu32_merge_delta_rate;
} VENC_CU_PREDICTION_S;

typedef struct HB_VENC_REF_PARAM_S {
  uint32_t use_longterm;
  uint32_t longterm_pic_period;
  uint32_t longterm_pic_using_period;
} VENC_REF_PARAM_S;

typedef struct HB_VENC_CHN_PARAM_S {
  HB_BOOL bColor2Grey;
  // uint32_t u32Priority;    //编码通道优先级参数
  // uint32_t u32MaxStrmCnt;  //最大码流缓存帧数
  // uint32_t u32PollWakeUpFrmCnt;
  // HB_VIDEO_CROP_INFO_S stCropCfg;     //裁剪静态属性不支持修改
  // VENC_FRAME_RATE_S stFrameRate;  //帧率控制
} VENC_CHN_PARAM_S;

typedef struct HB_VENC_RECV_PIC_PARAM_S {
  int32_t s32RecvPicNum;
} VENC_RECV_PIC_PARAM_S;

/***********************VENC_advanced_ATTR_S 264  start***********************/
typedef struct HB_VENC_H264_INTRA_PRED_S {
  uint32_t constrained_intra_pred_flag;
} VENC_H264_INTRA_PRED_S;

typedef struct HB_VENC_H264_DBLK_S {
  /**
   * Please refer to H.264 document.
   * Values[0~2]
   * !!!TODO the values 0~1?
   *
   * - Note: It's changable PPS parameters.
   * - Default: 0
   */
  uint32_t disable_deblocking_filter_idc;

  /**
   * Please refer to H.264 document.
   * Values[-6~6]
   *
   * - Note: It's changable PPS parameters.
   * - Default: 0
   */
  int32_t slice_alpha_c0_offset_div2;

  /**
   * Please refer to H.264 document.
   * Values[-6~6]
   *
   * - Note: It's changable PPS parameters.
   * - Default: 0
   */
  int32_t slice_beta_offset_div2;
} VENC_H264_DBLK_S;

typedef struct HB_VENC_H264_ENTROPY_S {
  /**
   * It selects the entropy coding method used in the encoding process.
   * The valid numbers are as follows.
   *     0 : CAVLC
   *     1 : CABAC
   *
   * - Note: It's changable PPS parameters.
   * !!!TODO changeable?
   * - Default: 1
   */
  uint32_t u32EntropyEncMode;

  /**
   * It specifies the index for determining the initialization table used
   * in the initialisation process for CABAC. The value of cabac_init_idc
   * shall be in the range of 0 ~ 2. Please refer to H.264 document.
   * !!!TODO Not support?
   *
   * - Note:
   * - Default: 0
   */
  //uint32_t cabac_init_idc;
} VENC_H264_ENTROPY_S;

typedef struct HB_VENC_H264_VUI_TIME_INFO_S {
  uint32_t fixed_frame_rate_flag;
  uint32_t num_units_in_tick;
  uint32_t time_scale;
} VENC_VUI_H264_TIME_INFO_S;

typedef struct HB_VENC_H264_VUI_S {
  VENC_VUI_H264_TIME_INFO_S stVuiTimeInfo;
} VENC_H264_VUI_S;

#define HB_VENC_SL_MATRIX_NUM 6  // MC_SL_MATRIX_NUM
typedef struct HB_VENC_H264_TRANS_S {
/**
 * It enables 8x8 transform.
 * The valid numbers are as follows.
 *     0 : disable 8x8 transform (BP)
 *     1 : enable 8x8 transform (HP)
 * - Note: It's changable parameter in the same sequence.
 * - Encoding: Support.
 * - Decoding: Unsupport.
 * - Default: 0
 */
  uint32_t transform_8x8_enable;
/**
 * The value of chroma(Cb) QP offset.
 * Values[-12~12]
 *
 * - Note: It's changable parameter in the same sequence.
 * - Encoding: Support.
 * - Decoding: Unsupport.
 * - Default: 0
 */
  int32_t chroma_cb_qp_offset;
/**
 * The value of chroma(Cr) QP offset.
 * Values[-12~12]
 *
 * - Note: It's changable parameter in the same sequence.
 * - Encoding: Support.
 * - Decoding: Unsupport.
 * - Default: 0
 */
  int32_t chroma_cr_qp_offset;
/**
 * Enable the user scaling list. ScalingList should include every scaling
 * list according to INTER/INTRA, prediction mode 4x4/8x8, and LUMA/CHROMA
 * just as H.264/AVC standard defines.
 * The valid numbers are as follows.
 *     0 : disable user scaling list
 *     1 : enable using user defined scaling list
 *     2 : enable using default standard scaling list
 * - Note: It's unchangable parameter in the same sequence.
 * - Encoding: Support.
 * - Decoding: Unsupport.
 * - Default: 0
 */
  uint32_t user_scaling_list_enable;
/**
 * The element is listed as follows. And each element has 16 coefficients.
 * "INTRA4X4_LUMA, INTRA4X4_CHROMAU, INTRA4X4_CHROMAV,
 * INTER4X4_LUMA, INTER4X4_CHROMAU, INTER4X4_CHROMAV"
 * Values[1~255]
 *
 * - Note: It's unchangable parameter in the same sequence.
 * - Encoding: Support.
 * - Decoding: Unsupport.
 * - Default:
 */
  uint8_t scaling_list_4x4[HB_VENC_SL_MATRIX_NUM][16];
/**
 * The element is listed as follows. And each element has 64 coefficients.
 * "INTRA8X8_LUMA,INTER8X8_LUMA"
 * Values[1~255]
 *
 * - Note: It's unchangable parameter in the same sequence.
 * - Encoding: Support.
 * - Decoding: Unsupport.
 * - Default:
 */
  uint8_t scaling_list_8x8[2][64];
} VENC_H264_TRANS_S;

typedef struct HB_VENC_H264_SLICE_SPLIT_S {
  int32_t h264_slice_mode;
  int32_t h264_slice_arg;
  //int32_t slice_loop_filter_across_slices_enabled_flag;
} VENC_H264_SLICE_SPLIT_S;

/*********************VENC_advanced_ATTR_S 265  start *************************/
typedef struct HB_VENC_H265_PU_S {
  uint32_t intra_nxn_enable;  //海思没有
  uint32_t max_num_merge;     //海思没有
  uint32_t constrained_intra_pred_flag;
  uint32_t strong_intra_smoothing_enabled_flag;
} VENC_H265_PU_S;

typedef struct HB_VENC_H265_DBLK_S {
  /**
   * Please refer to H.265 document.
   * Values[0~1]
   * !!!TODO the values 0~1?
   *
   * - Note: It's changable PPS parameters.
   * - Default: 0
   */
  uint32_t slice_deblocking_filter_disabled_flag;
  /**
   * Please refer to H.265 document.
   * Values[-6~6]
   *
   * - Note: It's changable PPS parameters.
   * - Default: 0
   */
  int32_t slice_beta_offset_div2;
  /**
   * Please refer to H.265 document.
   * Values[-6~6]
   *
   * - Note: It's changable PPS parameters.
   * - Default: 0
   */
  int32_t slice_tc_offset_div2;
  /**
 * It enables filtering across slice boundaries for in-loop deblocking.
 * The valid numbers are as follows.
 *     0 : disable
 *     1 : enable
 * - Note: It's changable PPS parameters.
 * - Encoding: Support.
 * - Decoding: Unsupport.
 * - Default: 1.
 */
	uint32_t slice_loop_filter_across_slices_enabled_flag;
} VENC_H265_DBLK_S;

typedef struct HB_VENC_VUI_H265_TIME_INFO_S {
  uint32_t num_units_in_tick;
  uint32_t time_scale;
  uint32_t num_ticks_poc_diff_one_minus1;
} VENC_VUI_H265_TIME_INFO_S;

typedef struct HB_VENC_H265_VUI_S {
  VENC_VUI_H265_TIME_INFO_S stVuiTimeInfo;
} VENC_H265_VUI_S;

typedef struct HB_VENC_H265_TRANSFORM_PARAMS {
  int32_t chroma_cb_qp_offset;
  int32_t chroma_cr_qp_offset;
  uint32_t user_scaling_list_enable;
  uint8_t scaling_list_4x4[HB_VENC_SL_MATRIX_NUM][16];
  uint8_t scaling_list_8x8[HB_VENC_SL_MATRIX_NUM][64];
  uint8_t scaling_list_16x16[HB_VENC_SL_MATRIX_NUM][64];
  uint8_t scaling_list_32x32[2][64];
  uint8_t scaling_list_dc_16x16[HB_VENC_SL_MATRIX_NUM];
  uint8_t scaling_list_dc_32x32[2];
} VENC_H265_TRANS_S;

typedef struct HB_VENC_H265_SLICE_SPLIT_S {
  int32_t h265_independent_slice_mode;
  int32_t h265_independent_slice_arg;
  int32_t h265_dependent_slice_mode;
  int32_t h265_dependent_slice_arg;
 // uint32_t slice_loop_filter_across_slices_enabled_flag;
} VENC_H265_SLICE_SPLIT_S;

typedef struct HB_VENC_H265_SAO_S {
  /**
   * It applies the sample adaptive offset process to the reconstructed
   * picture after the deblocking filter process. It enables both luma and
   * chroma components.
   * The valid numbers are as follows.
   *     0 : disable
   *     1 : enable
   *
   * - Note: It's unchangable parameter.
   * - Default: 0
   */
  uint32_t sample_adaptive_offset_enabled_flag;
} VENC_H265_SAO_S;

typedef struct HB_VENC_JPEG_PARAM_S {
  uint32_t u32Qfactor;
  uint8_t u8LumaQuantTable[64];
  uint8_t u8ChromaQuantTable[64];
  uint16_t u16LumaQuantEsTable[64];
  uint16_t u16ChromaQuantEsTable[64];
  uint32_t u32RestartInterval;
  VIDEO_CROP_INFO_S stCropCfg;
} VENC_JPEG_PARAM_S;

typedef struct HB_VENC_MJPEG_PARAM_S {
  uint8_t u8LumaQuantTable[64];
  uint8_t u8ChromaQuantTable[64];
  uint16_t u16LumaQuantEsTable[64];
  uint16_t u16ChromaQuantEsTable[64];
  uint32_t u32RestartInterval;
} VENC_MJPEG_PARAM_S;

typedef struct HB_USER_FRAME_INFO_S {
    VIDEO_FRAME_S stUserFrame;
    VENC_USER_RC_ATTR_S stUserRcInfo;
} USER_FRAME_INFO_S;

typedef enum HB_VENC_JPEG_ENCODE_MODE_E
{
  JPEG_ENCODE_ALL = 0,
  JPEG_ENCODE_SNAP = 1,
  JPEG_ENCODE_BUTT,
} HB_VENC_JPEG_ENCODE_MODE_E;
/*************************VENC_advanced_ATTR_S  end **************************/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HB_COMM_VENC_H__ */
