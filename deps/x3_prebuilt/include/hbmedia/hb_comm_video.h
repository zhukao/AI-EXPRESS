/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file:
 * @brief:
 * @author:
 * @email:
 * @date: 2019.12.10
 */

#ifndef __HB_COMM_VIDEO_H__
#define __HB_COMM_VIDEO_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define H264E_MAX_WIDTH 8192
#define H264E_MAX_HEIGHT 8192

#define H264E_MIN_WIDTH 256
#define H264E_MIN_HEIGHT 128

#define H265E_MAX_WIDTH 8192
#define H265E_MAX_HEIGHT 8192

#define H265E_MIN_WIDTH 256
#define H265E_MIN_HEIGHT 128

#define JPEGE_MAX_WIDTH 32768
#define JPEGE_MAX_HEIGHT 32768
#define JPEGE_MIN_WIDTH 16
#define JPEGE_MIN_HEIGHT 16

#define VENC_MAX_NAME_LEN 16
#define VENC_MAX_CHN_NUM 32
// #define VENC_MAX_GRP_NUM 16

#define VENC_MAX_ROI_NUM 8 /* The max numbers of ROI region support */
#define H264E_MIN_HW_INDEX 0
#define H264E_MAX_HW_INDEX 11
#define H264E_MIN_VW_INDEX 0
#define H264E_MAX_VW_INDEX 3

/**
* Define the MB number for max resolution = 8192x8192/(16x16).
**/
#define VIDEO_MAX_MB_NUM                0x40000

#define VDEC_MAX_CHN_NUM 32



typedef enum HB_PIXEL_FORMAT_E {
  HB_PIXEL_FORMAT_NONE = -1,
  /* planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples) */
  HB_PIXEL_FORMAT_YUV420P,
  /*
   * planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components,
   * which are interleaved (first byte U and the following byte V).
   */
  HB_PIXEL_FORMAT_NV12,
  /* as above, but U and V bytes are swapped */
  HB_PIXEL_FORMAT_NV21,
  /* planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples) */
  HB_PIXEL_FORMAT_YUV422P,
  /*
   * interleaved chroma (first byte U and the following byte V)
   * YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
   */
  HB_PIXEL_FORMAT_NV16,
  /*
   * interleaved chroma (first byte V and the following byte U)
   * YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
   */
  HB_PIXEL_FORMAT_NV61,
  /* packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr */
  HB_PIXEL_FORMAT_YUYV422,
  /* packed YUV 4:2:2, 16bpp, Y0 Cr Y1 Cb */
  HB_PIXEL_FORMAT_YVYU422,
  /* packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1 */
  HB_PIXEL_FORMAT_UYVY422,
  /* packed YUV 4:2:2, 16bpp, Cr Y0 Cb Y1 */
  HB_PIXEL_FORMAT_VYUY422,
  /* packed YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)*/
  HB_PIXEL_FORMAT_YUV444,
  /* planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)*/
  HB_PIXEL_FORMAT_YUV444P,
  /*
   * interleaved chroma (first byte U and the following byte V)
   * YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
   */
  HB_PIXEL_FORMAT_NV24,
  /*
   * interleaved chroma (first byte V and the following byte U)
   * YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
   */
  HB_PIXEL_FORMAT_NV42,
  /* planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples)*/
  HB_PIXEL_FORMAT_YUV440P,
  /* Gray Y, YUV 4:0:0 */
  HB_PIXEL_FORMAT_YUV400,

  HB_PIXEL_FORMAT_TOTAL,
} PIXEL_FORMAT_E;

typedef enum HB_CODEC_ROTATION_S {
  CODEC_ROTATION_0 = 0,
  CODEC_ROTATION_90 = 1,
  CODEC_ROTATION_180 = 2,
  CODEC_ROTATION_270 = 3,
  ROTATION_BUTT
} CODEC_ROTATION_E;

typedef enum HB_MIRROR_FLIP_E {
  DIRECTION_NONE = 0,
  VERTICAL = 1,
  HORIZONTAL = 2,
  HOR_VER = 3,
  DIRECTION_BUTT,
} MIRROR_FLIP_E;

typedef struct HB_CODEC_RECT_S {
  int32_t s32X;       /*x-coordinate of rectangle*/
  int32_t s32Y;       /*y-coordinate of rectangle*/
  uint32_t u32Width;  /*width of rectangle*/
  uint32_t u32Height; /*height of rectangle*/
} CODEC_RECT_S;

typedef struct HB_VIDEO_FRAME_PACK_S {
  hb_char* vir_ptr[3];
  uint32_t phy_ptr[3];
  uint32_t size;
  uint32_t width;
  uint32_t height;
  PIXEL_FORMAT_E pix_format;
  int32_t stride;
  int32_t vstride;
  int32_t fd[3];
  uint64_t pts;
  HB_BOOL frame_end;
  int32_t flags;
  int32_t src_idx;
} VIDEO_FRAME_PACK_S;

typedef struct HB_VIDEO_FRAME_INFO_S {
  /* It indicates that decoding result for enqueued decode command
   * The valid numbers are as follows.
   *     0x00: FAIL
   *     0x01: SUCCESS
   *     0x10: SUCCESS_WITH_WARNING (success but there exist some warning)
   */
  int32_t decode_result;
  /**
   * This is a frame buffer index for the picture to be displayed at the
   * moment among frame buffers which are registered using
   * VPU_DecRegisterFrameBuffer(). Frame data to be displayed are stored into
   * the frame buffer with this index.
   * When there is no display delay, this index is always the same with
   * frame_decoded_index. However, if display delay does exist for display
   * reordering in AVC or B-frames in VC1), this index might be different
   * with frame_decoded_index. By checking this index, HOST application can
   * easily know whether sequence decoding has been finished or not.
   * The valid numbers are as follows.
   *     -3(0xFFFD) or -2(0xFFFE) : a display output cannot be given due to
   *                                picture reordering or skip option.
   *     -1(0xFFFF) : there is no more output for display
   *                  at the end of sequence decoding.
   *      > 0 : Normal display index.
   */
  int32_t frame_display_index;

  /* This is a frame buffer index of decoded picture among frame buffers
   * which were registered using VPU_DecRegisterFrameBuffer(). The currently
   * decoded frame is stored into the frame buffer specified by this index.
   * The valid numbers are as follows.
   *     -2 : it indicates that no decoded output is generated because
   *          decoder meets EOS (End Of Sequence) or skip.
   *     -1 : it indicates that decoder fails to decode a picture because
   *          there is no available frame buffer.
   *     > 0 : Normal decode index.
   */
  int32_t frame_decoded_index;

  /**
   * This is the physical start address of corresponding stream buffer.
   *
   * - Note:
   * - Encoding: Unsupport.
   * - Decoding: Support.
   * - Default:
   */
  uint64_t stream_start_addr;

 /**
  * This is the size of corresponding stream buffer.
  *
  * - Note:
  * - Encoding: Unsupport.
  * - Decoding: Support.
  * - Default:
  */
	int32_t stream_size;


  /* This is the picture type of decoded picture.
   * @see HB_h264_nal_unit_type_t
   * @see HB_h265_nal_unit_type_t
   */
  int32_t nalu_type;

  /* This is the number of error coded unit in a decoded picture (
   * frame_decoded_index)
   */
  int32_t err_mb_in_frame_decoded;

  /* This is the number of coded unit in a decoded picture.(
   * frame_decoded_index)
   */
  int32_t total_mb_in_frame_decoded;

  /* This is the number of error coded unit in a picture mapped to
   * frame_display_index.
   */
  int32_t err_mb_in_frame_display;

  /* This is the number of coded unit in a picture
   * mapped to frame_display_index.
   */
  int32_t total_mb_in_frame_display;

  /* This field reports the display rectangular region in pixel unit.*/
  CODEC_RECT_S display_rect;

  /* This field reports the width of a picture to be displayed in pixel unit */
  int32_t display_width;

  /* This field reports the height of a picture to be displayed in pixel unit*/
  int32_t display_height;

  /* This field reports the decoded rectangular region in pixel unit */
  CODEC_RECT_S decoded_rect;

  /* This is aspect ratio information for each standard.
   */
  int32_t aspect_rate_info;

  /* The numerator part of frame rate fraction. If frame rate syntax is not
   * decoded in bitstream, the value of frame_rate_numerator is equal to -1.
   * If frame_rate_numerator and frame_rate_denominator are not-zero values,
   * FrameRate is derived as follows.
   * FrameRate =frame_rate_numerator/frame_rate_denominator.
   * Otherwise if frame_rate_numerator or frame_rate_denominator are zero
   * values, the value of FrameRate is invalid.
   */
  int32_t frame_rate_numerator;

  /* The denominator part of frame rate fraction. If frame rate syntax is not
   * decoded in bitstream, the value of frame_rate_denominator is equal to -1.
   */
  int32_t frame_rate_denominator;

  /* A POC value of picture with display index. When frame_display_index is
   * -1, it returns -1.
   *
   * - Note: h265 only
   */
  int32_t display_poc;

  /**
 * A POC value of picture that has currently been decoded and with decoded
 * index. When frame_decoded_index is -1, it returns -1.
 *
 * - Note: h265 only
 * - Encoding: Unsupport.
 * - Decoding: Support.
 * - Default:
 */
  int32_t decoded_poc;

  /* This variable reports the error reason that occurs while decoding.
   * For error description, please find the 'Appendix: Error Definition'
   * in the Programmer's Guide.
   */
  int32_t error_reason;

  /* This variable reports the warning information that occurs while decoding.
   * For warning description, please find the 'Appendix: Error Definition'
   * in the Programmer's Guide.
   */
  int32_t warn_info;

  /**
 * This variable increases by 1 whenever sequence changes.
 *
 * - Note:
 * - Encoding: Unsupport.
 * - Decoding: Support.
 * - Default:
 */
	int32_t sequence_no;

/**
 * A temporal ID of Nth picture in the custom GOP.
 *
 * - Note:
 * - Encoding: Unsupport.
 * - Decoding: Support.
 * - Default:
 */
	int32_t temporal_id;

  /* If output_flag is 0, the current decoded picture is not output for
   * display. If output_flag is 1, the current decoded picture is
   * (will be) output for display.
   */
  int32_t output_flag;

  /* A CTU size
   *    16 : CTU16x16
   *    32 : CTU32x32
   *    64 : CTU64x64
   */
  int32_t ctu_size;
} VIDEO_FRAME_INFO_S;


typedef struct HB_VIDEO_FRAME_INFO_JPEG_S {
  /* It indicates that decoding result for enqueued decode command
   * The valid numbers are as follows.
   *     0x00: FAIL with error block
   *     0x01: SUCCESS
   *
   * - Note:
   * - Encoding: Unsupport.
   * - Decoding: Support.
   * - Default:
   */
  int32_t decode_result;

  /**
   * This is a frame buffer index for the picture to be displayed at the
   * moment among frame buffers which are registered using
   * VPU_DecRegisterFrameBuffer(). Frame data to be displayed are stored into
   * the frame buffer with this index.
   * !!!TODO confirm this
   *     -1(0xFFFF) : ?
   *      > 0 : Normal display index.
   *
   * - Note:
   * - Encoding: Unsupport.
   * - Decoding: Support.
   * - Default:
   */
  int32_t frame_display_index;

  /**
   * This is the physical start address of corresponding stream buffer.
   *
   * - Note:
   * - Encoding: Unsupport.
   * - Decoding: Support.
   * - Default:
   */
  uint64_t stream_start_addr;

 /**
  * This is the size of corresponding stream buffer.
  *
  * - Note:
  * - Encoding: Unsupport.
  * - Decoding: Support.
  * - Default:
  */
	int32_t stream_size;

  /* JPEG error restart index. It's valid only if decode_result is success.
   *
   * - Note:
   * - Encoding: Unsupport.
   * - Decoding: Support.
   * - Default:
   */
  int32_t err_rst_idx;

  /* JPEG error MCU position X. It's valid only if decode_result is success.
   *
   * - Note:
   * - Encoding: Unsupport.
   * - Decoding: Support.
   * - Default:
   */
  int32_t err_pos_x;

  /* JPEG error MCU position Y. It's valid only if decode_result is success.
   *
   * - Note:
   * - Encoding: Unsupport.
   * - Decoding: Support.
   * - Default:
   */
  int32_t err_pos_y;

  /* This field reports the width of a picture to be displayed in pixel unit.
   *
   * - Note:
   * - Encoding: Unsupport.
   * - Decoding: Support.
   * - Default:
   */
  int32_t display_width;

  /* This field reports the height of a picture to be displayed in pixel unit.
   *
   * - Note:
   * - Encoding: Unsupport.
   * - Decoding: Support.
   * - Default:
   */
  int32_t display_height;

} VIDEO_FRAME_INFO_JPEG_S;

typedef struct HB_VIDEO_FRAME_S {
  VIDEO_FRAME_PACK_S stVFrame;
  union {
    VIDEO_FRAME_INFO_S stFrameInfo;
    VIDEO_FRAME_INFO_JPEG_S stJpegInfo;
  };
} VIDEO_FRAME_S;

typedef struct HB_VIDEO_PACK_S {
  hb_char* vir_ptr;
  uint32_t phy_ptr;
  uint32_t size;
  uint64_t pts;
  uint32_t fd;
  uint32_t src_idx;
  HB_BOOL stream_end;
} VIDEO_STREAM_PACK_S;

typedef struct HB_VIDEO_STREAM_INFO_S {
  HB_BOOL frame_index;
  uint64_t frame_start_addr;
  int32_t frame_size;
  int32_t nalu_type;
  uint32_t slice_idx;
  uint32_t slice_num;
  uint32_t dependent_slice_num;
  uint32_t independent_slice_num;
  uint32_t pic_skipped;
  uint32_t intra_block_num;
  uint32_t skip_block_num;
  uint32_t avg_mb_qp;
  uint32_t enc_pic_byte;
  int32_t enc_gop_pic_idx;
  int32_t enc_pic_poc;
  uint32_t enc_src_idx;
  uint32_t enc_pic_cnt;
  int32_t enc_error_reason;
  int32_t enc_warn_info;
  uint32_t frame_cycle;
} VIDEO_STREAM_INFO_S;

typedef struct HB_VIDEO_STREAM_INFO_JPEG_S {
  uint64_t frame_start_addr;
	int32_t frame_size;
	uint32_t slice_idx;
	uint32_t slice_num;
  uint32_t frame_cycle;
} VIDEO_STREAM_INFO_JPEG_S;

typedef struct HB_VIDEO_STREAM_S {
  VIDEO_STREAM_PACK_S pstPack;
  union {
    VIDEO_STREAM_INFO_S stStreamInfo;
    VIDEO_STREAM_INFO_JPEG_S stJpegInfo;
  };
} VIDEO_STREAM_S;

typedef struct HB_VIDEO_CROP_INFO_S {
  HB_BOOL bEnable;
  CODEC_RECT_S stRect;
} VIDEO_CROP_INFO_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef  __HB_COMM_VIDEO_H__ */
