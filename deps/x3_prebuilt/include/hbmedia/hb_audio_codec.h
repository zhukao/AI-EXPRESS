/*************************************************************************
 *                     COPYRIGHT NOTICE
 *            Copyright 2019 Horizon Robotics, Inc.
 *                   All rights reserved.
 *************************************************************************/
#ifndef AUDIO_INCLUDE_HB_AUDIO_CODEC_H__
#define AUDIO_INCLUDE_HB_AUDIO_CODEC_H__

#include "hb_media_codec.h"
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif				// __cplusplus
	typedef enum {
		PT_AAC = 0,
		PT_FLAC = 1,
		PT_ADPCM = 2,
		PT_G726 = 3,
		PT_G711A = 4,
		PT_G711U = 5,
		PT_BUTT
	} PAYLOAD_TYPE_E;

	struct HB_AUDIO_CHANNEL_CTX {
		int32_t audioChn;
		media_codec_context_t *Context;
		media_codec_buffer_t *buffer;
	};

	struct AENC_ATTR_G711A {
		/*Reserved*/
	};

	struct AENC_ATTR_G711U {
		/*Reserved*/
	};

	struct AENC_ATTR_AAC {
		mc_aac_profile_t profile;
		mc_aac_data_type_t type;
	};

	struct AENC_ATTR_FLAC {
		mc_flac_lpc_type_t profile;
		uint32_t compression_level;
	};

	struct AENC_ATTR_ADPCM {
		/*Reserved*/
	};

	struct AENC_ATTR_G726 {
		mc_g726_bps_t bit_rate;
	};

	struct HB_AENC_PARAMS {
		uint32_t bit_rate;
		uint32_t frame_size;
		uint32_t frame_buf_count;
		uint32_t packet_count;
		mc_audio_sample_format_t sample_fmt;
		mc_audio_sample_rate_t sample_rate;
		mc_audio_channel_layout_t channel_layout;
		uint8_t channels;

		union {
			struct AENC_ATTR_G711A stG711a;
			struct AENC_ATTR_G711U stG711u;
			struct AENC_ATTR_AAC stAAC;
			struct AENC_ATTR_FLAC stFLAC;
			struct AENC_ATTR_ADPCM stADPCM;
			struct AENC_ATTR_G726 stG726;
		};
	};

	struct HB_AENC_CHN_ATTR_S {
		PAYLOAD_TYPE_E type;
		uint32_t numPerFrm;
		uint32_t bufSize;
		struct HB_AENC_PARAMS aeParams;
	};

	struct HB_AUDIO_FRAME_S {
		media_codec_buffer_type_t type;
		uint8_t *virAddr;
		uint32_t phyAddr;
		uint64_t timeStamp;
		uint32_t seq;
		uint32_t len;
		uint32_t *poolId;
	};

	struct HB_AENC_AEC_FRAME_S {
		struct HB_AUDIO_FRAME_S aec_frame;
		bool is_valid;
		bool is_sys_bind;
	};

	struct HB_AUDIO_STREAM_S {
		media_codec_buffer_type_t type;
		uint8_t *virAddr;
		uint32_t phyAddr;
		uint32_t len;
		uint32_t timeStamp;
		uint32_t seq;
	};

	struct HB_AENC_ENCODE_PARAM_S {
		media_codec_id_t type;
		int max_frm;
		char *codec_name;
		int (*audio_open_encoder)(void *encoderParam,
				void **encoder);
		int (*audio_encode_frame)(void *encoder,
				mc_audio_frame_buffer_info_t *audioFrame, uint8_t *outbuf,
				int *outlen);
		int (*audio_close_encoder)(void *encoder);
	};

	enum ADEC_MODE_E {
		ADEC_MODE_FRAME = 0,
		ADEC_MODE_STREAM = 1,
		ADEC_MODE_BUFF,
	};

	struct ADEC_ATTR_G711A {
		mc_audio_sample_rate_t sample_rate;
		uint8_t channels;
	};

	struct ADEC_ATTR_G711U {
		mc_audio_sample_rate_t sample_rate;
		uint8_t channels;
	};

	struct ADEC_ATTR_ADPCM {
		uint8_t channels;
	};

	struct ADEC_ATTR_FLAC {
		/*Reserved*/
	};

	struct ADEC_ATTR_AAC {
		mc_aac_data_type_t type;
	};

	struct ADEC_ATTR_G726 {
		mc_g726_bps_t bit_rate;
	};

	struct HB_ADEC_PARAMS {
		mc_av_stream_feeding_mode_t feed_mode;
		uint32_t packet_buf_size;
		uint32_t packet_count;
		uint32_t frame_cache_size;
		uint32_t internal_frame_size;
		uint32_t frame_buf_count;

		union {
			struct ADEC_ATTR_G711A stG711a;
			struct ADEC_ATTR_G711U stG711u;
			struct ADEC_ATTR_ADPCM stADPCM;
			struct ADEC_ATTR_AAC stAAC;
			struct ADEC_ATTR_FLAC stFLAC;
			struct ADEC_ATTR_G726 stG726;
		};
	};

	struct HB_ADEC_CHN_ATTR_S {
		PAYLOAD_TYPE_E type;
		uint32_t bufSize;
		enum ADEC_MODE_E mode;
		struct HB_ADEC_PARAMS adParams;
	};

	struct HB_ADEC_DECODE_PARAM_S {
		media_codec_id_t type;
		char *codec_name;
		int (*audio_open_decoder)(void *decoderParam,
				void **decoder);
		int (*audio_decode_frame)(void *decode, uint8_t *inbuf,
				int inlen, mc_audio_frame_buffer_info_t *outbuf,
				int *outlen);
		int (*audio_get_frame_info)(void *decoder,
				void *info);
		int (*audio_close_decoder)(void *decoder);
	};

	int HB_AENC_CreateChn(uint8_t aeChn, struct HB_AENC_CHN_ATTR_S *pstAttr);
	int HB_AENC_DestroyChn(uint8_t aeChn);
	int HB_AENC_SendFrame(uint8_t aeChn, struct HB_AUDIO_FRAME_S *pstFrame,
		struct HB_AENC_AEC_FRAME_S *pstAecFrame);
	int HB_AENC_GetStream(uint8_t aeChn, struct HB_AUDIO_STREAM_S *pstStream,
		uint32_t s32MilliSec);
	int HB_AENC_ReleaseStream(uint8_t aeChn,
		const struct HB_AUDIO_STREAM_S *pstStream);
	int HB_AENC_GetFd(uint8_t aeChn);
	int HB_AENC_RegisterEncoder(int *handle,
		struct HB_AENC_ENCODE_PARAM_S *encoder);
	int HB_AENC_UnregisterEncoder(int handle);
	int HB_AENC_GetStreamBufInfo(uint8_t aeChn,
		uint32_t *phyAddr, uint32_t *size);

	int HB_ADEC_CreateChn(uint8_t adChn, struct HB_ADEC_CHN_ATTR_S *pstAttr);
	int HB_ADEC_DestroyChn(uint8_t adChn);
	int HB_ADEC_SendStream(uint8_t adChn,
		struct HB_AUDIO_STREAM_S *pstStream, bool block);
	int HB_ADEC_GetFrame(uint8_t adChn,
		struct HB_AUDIO_FRAME_S *pstFrame, bool block);
	int HB_ADEC_ReleaseFrame(uint8_t adChn, struct HB_AUDIO_FRAME_S *pstFrame);
	int HB_ADEC_SendEndofStream(uint8_t adChn, bool bClear);
	int HB_ADEC_RegisterDecoder(int *handle,
		struct HB_ADEC_DECODE_PARAM_S *decoder);
	int HB_ADEC_UnregisterDecoder(int handle);
	int HB_ADEC_ClearChnBuffer(uint8_t adChn);
#ifdef __cplusplus
}
#endif				// __cplusplus
#endif				// AUDIO_INCLUDE_HB_AUDIO_CODEC_H__
