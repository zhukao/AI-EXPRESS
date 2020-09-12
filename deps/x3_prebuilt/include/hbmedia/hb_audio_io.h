/*************************************************************************
 *                     COPYRIGHT NOTICE
 *            Copyright 2019 Horizon Robotics, Inc.
 *                   All rights reserved.
 *************************************************************************/
#ifndef AUDIO_INCLUDE_HB_AUDIO_IO_H_
#define AUDIO_INCLUDE_HB_AUDIO_IO_H_
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif				// __cplusplus
	enum HB_AUDIO_MODE_S {
		audio_mode_master = 0,
		audio_mode_slave = 1,
		audio_mode_buff,
	};

	enum HB_AUDIO_SAMPLE_RATE_E {
		audio_sample_rate_8k = 8000,
		audio_sample_rate_16k = 16000,
		audio_sample_rate_32k = 32000,
		audio_sample_rate_48k = 48000,
		audio_sample_rate_buff,
	};

	enum HB_AUDIO_SOUND_MODE_E {
		audio_sound_mode_mono = 0,
		audio_sound_mode_stereo = 1,
		audio_sound_mode_buff,
	};

	enum HB_AUDIO_SAMPLE_FORMAT_E {
		audio_sample_format_8 = 0,
		audio_sample_format_16 = 1,
		audio_sample_format_buff,
	};

	enum HB_AUDIIO_I2S_TYPE_E {
		audio_i2s_type_i2s = 0,
		audio_i2s_type_dsp = 1,
		audio_i2s_type_buff,
	};

	struct HB_AIO_DEVICE_ATTR_S {
		enum HB_AUDIO_MODE_S workMode;
		enum HB_AUDIO_SAMPLE_RATE_E sampleRate;
		enum HB_AUDIO_SOUND_MODE_E soundMode;
		enum HB_AUDIO_SAMPLE_FORMAT_E sampleFmt;
		uint32_t periodSize;
		uint8_t periodCount;
		uint8_t exFlag;
		uint8_t channels;
		uint8_t clkSet;
		enum HB_AUDIIO_I2S_TYPE_E i2sType;
		uint32_t frameSize;
	};

	struct HB_AIO_FRAME_S {
		uint8_t *data;
		uint32_t count;
	};

	struct HB_AIN_AEC_FRAME_S {
		struct HB_AIO_FRAME_S *aecFrame;
		bool valid;
	};

	int HB_AIN_SetPubAttr(uint8_t devId, struct HB_AIO_DEVICE_ATTR_S *pstAttr);
	int HB_AIN_GetPubAttr(uint8_t devId, struct HB_AIO_DEVICE_ATTR_S *pstAttr);

	int HB_AIN_Enable(uint8_t devId);
	int HB_AIN_Disable(uint8_t devId);

	int HB_AIN_GetFrame(uint8_t devId, uint8_t chn,
		struct HB_AIO_FRAME_S *pstFrame, struct HB_AIN_AEC_FRAME_S *pstAecFrm,
		uint32_t milliSec);
	int HB_AIN_ReleaseFrame(uint8_t devId, uint8_t chn,
		struct HB_AIO_FRAME_S *pstFrame,
		struct HB_AIN_AEC_FRAME_S *pstAecFrm);

	int HB_AIN_SetVolume(uint8_t cardId, uint8_t chn, uint16_t volume);
	int HB_AIN_GetVolume(uint8_t cardId, uint8_t chn, uint16_t *volume);

	int HB_AOT_SetPubAttr(uint8_t devId, struct HB_AIO_DEVICE_ATTR_S *pstAttr);
	int HB_AOT_GetPubAttr(uint8_t devId, struct HB_AIO_DEVICE_ATTR_S *pstAttr);

	int HB_AOT_Enable(uint8_t devId);
	int HB_AOT_Disable(uint8_t devId);

	int HB_AOT_SendFrame(uint8_t devId, uint8_t chn,
		const struct HB_AIO_FRAME_S *pstData, uint32_t milliSec);

	int HB_AOT_SetVolume(uint8_t cardId, uint16_t volume);
	int HB_AOT_GetVolume(uint8_t cardId, uint16_t *volume);
#ifdef __cplusplus
}
#endif				// __cplusplus
#endif				// AUDIO_INCLUDE_HB_AUDIO_IO_H_
