/**
 * * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: media_codec_test.cpp
 * @Author: yong.wu
 * @Email: yong.wu@horizon.ai
 * @Date: 2020-07-15
 * @Version: v0.0.2
 * @Brief: media codec unit test
 */

#include <stdio.h>
#include <string.h>
#include <gtest/gtest.h>
#include <iostream>
#include "hobotlog/hobotlog.hpp"
#include "utils/time_helper.h"
#include "media_codec/media_codec_manager.h"
#include "./media_codec_test.h"

#define MAX_FILE_NAME_LEN 256
#define MAX_FILE_NUM 100

using horizon::vision::MediaCodecManager;
using hobot::Timer;
extern cmdOptions_t cmd_options;
namespace horizon {
namespace vision {
namespace media_codec_test {

class MEDIA_CODEC_TEST : public testing::Test {
 protected:
     virtual void SetUp() {
         pic_w = cmd_options.mGlobalWidth;
         pic_h = cmd_options.mGlobalHeight;
         pic_size = cmd_options.mGlobalSize;
         y_pic_size = cmd_options.mGlobalySize;
         uv_pic_size = cmd_options.mGlobaluvSize;
         pix_fmt = static_cast<PIXEL_FORMAT_E>(cmd_options.mGlobalPixFmt);
         inputFileNameBak = cmd_options.mGlobalInFileNameBak.c_str();
         inputFilePath = cmd_options.mGlobalInFilePath.c_str();
         outputFileNameBak = cmd_options.mGlobalOutFileNameBak.c_str();
         outputFilePath = cmd_options.mGlobalOutFilePath.c_str();
         frame_buf_depth = cmd_options.mGlobalBufDepth;
         vb_num = frame_buf_depth;
         out_stream_num = cmd_options.mGlobalStreamNum;
         qfactor = cmd_options.mGlobalQfactor;
         vb_cache_enable = cmd_options.mGlobalVbCacheEnable;
         is_cbr = cmd_options.mGlobalIsCbr;
         bitrate = cmd_options.mGlobalBitrate;
     }
     virtual void TearDown() { }

 public:
     int pic_w;
     int pic_h;
     int pic_size;
     int y_pic_size;
     int uv_pic_size;
     PIXEL_FORMAT_E pix_fmt;
     const char* inputFileNameBak;
     const char* inputFilePath;
     const char* outputFileNameBak;
     const char* outputFilePath;
     char inputFileName[MAX_FILE_NAME_LEN];
     char inputFileNameTemp[MAX_FILE_NAME_LEN];
     char outputFileName[MAX_FILE_NAME_LEN];
     char outputFileNameTemp[MAX_FILE_NAME_LEN];
     int frame_buf_depth;
     int vb_num;
     int out_stream_num;
     int qfactor;
     int vb_cache_enable;
     int is_cbr;
     int bitrate;
};


typedef struct _MediaCodecTestContext_s {
    int pic_w;
    int pic_h;
    int pic_size;
    int y_pic_size;
    int u_pic_size;
    int v_pic_size;
    PIXEL_FORMAT_E pix_fmt;
    const char *fmt_name;
    char *inputFileName;
    char *outputFileNameTemp;
    int frame_buf_depth;
    int qfactor;
    int vb_num;
    int out_stream_num;
    int vb_cache_enable;
    int is_cbr;
    int bitrate;
} MediaCodecTestContext;

static void do_jpeg_encoding(MediaCodecTestContext *arg) {
    MediaCodecTestContext *ctx = reinterpret_cast<MediaCodecTestContext *>(arg);
    ASSERT_NE(ctx, nullptr);

    int chn, ret, i;
    int y_readsize, u_readsize, v_readsize;
    FILE *inFile = NULL;
    FILE *outFile[MAX_FILE_NUM] = { NULL };
    char outputFileName[MAX_FILE_NAME_LEN];
    iot_venc_src_buf_t *frame_buf = nullptr;
    iot_venc_stream_buf_t *stream_buf = nullptr;
    const char *fmt_name = ctx->fmt_name;
    char *inputFileName = ctx->inputFileName;
    char *outputFileNameTemp = ctx->outputFileNameTemp;
    int pic_w = ctx->pic_w;
    int pic_h = ctx->pic_h;
    int pic_size = ctx->pic_size;
    int y_pic_size = ctx->y_pic_size;
    int u_pic_size = ctx->u_pic_size;
    int v_pic_size = ctx->v_pic_size;
    PIXEL_FORMAT_E pix_fmt = ctx->pix_fmt;
    int frame_buf_depth = ctx->frame_buf_depth;
    int qfactor = ctx->qfactor;
    int vb_num = ctx->vb_num;
    int vb_cache_enable = ctx->vb_cache_enable;
    int out_stream_num = ctx->out_stream_num;
    int is_cbr = ctx->is_cbr;
    int bitrate = ctx->bitrate;

    LOGI << "inputFileName: " << inputFileName;
    inFile = fopen(inputFileName, "rb");
    HOBOT_CHECK(inFile != NULL);

    /* 1. media codec init */
    /* 1.1 get media codec manager and module init */
    MediaCodecManager &manager = MediaCodecManager::Get();
    ret = manager.ModuleInit();
    HOBOT_CHECK(ret == 0);
    /* 1.2 get media codec venc chn and open cal encode time*/
    chn = manager.GetEncodeChn();
    /* 1.3 media codec venc chn init */
    LOGI << "pic_w:" << pic_w << " pic_h:" << pic_h << " chn:" << chn
         << " frame_buf_depth:" << frame_buf_depth << " pix_fmt:" << pix_fmt;
    ret = manager.EncodeChnInit(chn, PT_JPEG, pic_w, pic_h,
            frame_buf_depth, pix_fmt, is_cbr, bitrate);
    HOBOT_CHECK(ret == 0);
    /* 1.4 set media codec venc jpg chn qfactor params */
    LOGI << "jpeg quality_factor: " << qfactor;
    ret = manager.SetUserQfactorParams(chn, qfactor);
    HOBOT_CHECK(ret == 0);
    /* 1.5 set media codec venc jpg chn qfactor params */
    ret = manager.EncodeChnStart(chn);
    HOBOT_CHECK(ret == 0);
    /* 1.6 alloc media codec vb buffer init */
    LOGI << " pic_w:" << pic_w << " pic_h:" << pic_h
         << " pic_size:" << pic_size << " vb_num:" << vb_num
         << " vb_cache_enable:" << vb_cache_enable;
    ret = manager.VbBufInit(chn, pic_w, pic_h, pic_w, pic_size, vb_num,
            vb_cache_enable);
    HOBOT_CHECK(ret == 0);

    /* 2. start encode yuv to jpeg */
    HOBOT_CHECK(out_stream_num < MAX_FILE_NUM);
    for (i = 0; i < out_stream_num; i++) {
        snprintf(outputFileName, MAX_FILE_NAME_LEN, "%s_%s_%d_%d.jpg",
                outputFileNameTemp, fmt_name, pic_h, i);
        LOGI << "outputFileName: " << outputFileName;
        outFile[i] = fopen(outputFileName, "wb");
        HOBOT_CHECK(outFile[i] != NULL);
        /* 2.1 get media codec vb buf for store src yuv data */
        ret = manager.GetVbBuf(chn, &frame_buf);
        LOGE << "frame_buf: " << frame_buf;
        HOBOT_CHECK(ret == 0);
        /* 2.2 get src yuv data */
        if (y_pic_size > 0) {
            y_readsize = fread(frame_buf->frame_info.vir_ptr[0], 1,
                    y_pic_size, inFile);
            LOGI << "y_readsize: " << y_readsize;
        }
        if (u_pic_size > 0) {
            u_readsize = fread(frame_buf->frame_info.vir_ptr[1], 1,
                    u_pic_size, inFile);
            LOGI << "u_readsize: " << u_readsize;
        }
        if (v_pic_size > 0) {
            v_readsize = fread(frame_buf->frame_info.vir_ptr[2], 1,
                    v_pic_size, inFile);
            LOGI << "v_readsize: " << v_readsize;
        }
        rewind(inFile);
        /* 2.3. encode yuv data to jpg */
        auto ts0 = Timer::current_time_stamp();
        ret = manager.EncodeYuvToJpg(chn, frame_buf, &stream_buf);
        HOBOT_CHECK(ret == 0);
        auto ts1 = Timer::current_time_stamp();
        LOGI << "******Encode " << fmt_name << " to jpeg cost: "
             << ts1 - ts0 << "ms";
        fwrite(stream_buf->stream_info.pstPack.vir_ptr,
                stream_buf->stream_info.pstPack.size, 1, outFile[i]);
        /* 2.4 free jpg stream buf */
        ret = manager.FreeStream(chn, stream_buf);
        HOBOT_CHECK(ret == 0);
        /* 2.5 free media codec vb buf */
        ret = manager.FreeVbBuf(chn, frame_buf);
        HOBOT_CHECK(ret == 0);
    }

    /* 3. media codec deinit */
    /* 3.1 media codec chn stop */
    ret = manager.EncodeChnStop(chn);
    HOBOT_CHECK(ret == 0);
    /* 3.2 media codec chn deinit */
    ret = manager.EncodeChnDeInit(chn);
    HOBOT_CHECK(ret == 0);
    /* 3.3 media codec vb buf deinit */
    ret = manager.VbBufDeInit(chn);
    HOBOT_CHECK(ret == 0);
    /* 3.4 media codec module deinit */
    ret = manager.ModuleDeInit();
    HOBOT_CHECK(ret == 0);

    if (inFile) fclose(inFile);
    for (i = 0; i < out_stream_num; i++) {
        if (outFile[i])
            fclose(outFile[i]);
    }

}

TEST_F(MEDIA_CODEC_TEST, nv12_to_jpeg) {
    MediaCodecTestContext ctx = { 0 };
    const char *inputFileNameTemp = "input_nv12_1080p.yuv";

    memset(inputFileName, 0x00, sizeof(inputFileName));
    strncat(inputFileName, inputFilePath, strlen(inputFilePath));
    if (strlen(inputFileNameBak) == 0) {
        strncat(inputFileName, inputFileNameTemp, strlen(inputFileNameTemp));
    } else {
        strncat(inputFileName, inputFileNameBak, strlen(inputFileNameBak));
    }
    memset(outputFileNameTemp, 0x00, sizeof(outputFileNameTemp));
    strncat(outputFileNameTemp, outputFilePath, strlen(outputFilePath));
    strncat(outputFileNameTemp, outputFileNameBak, strlen(outputFileNameBak));


    ctx.fmt_name = "nv12";
    ctx.inputFileName = inputFileName;
    ctx.outputFileNameTemp = outputFileNameTemp;
    ctx.frame_buf_depth = frame_buf_depth;
    ctx.qfactor = qfactor;
    ctx.vb_num = vb_num;
    ctx.pic_w = pic_w;
    ctx.pic_h = pic_h;
    ctx.pic_size = pic_w * pic_h * 3 / 2;
    ctx.y_pic_size = pic_w * pic_h;   // data store 2 plane
    ctx.u_pic_size = pic_w * pic_h / 2;  // u_pic_size is equal u+v
    ctx.v_pic_size = 0;
    ctx.pix_fmt = HB_PIXEL_FORMAT_NV12;
    ctx.out_stream_num = out_stream_num;
    if (vb_cache_enable == -1) {
        ctx.vb_cache_enable = 1;
    } else {
        ctx.vb_cache_enable = vb_cache_enable;
    }
    do_jpeg_encoding(&ctx);
}

TEST_F(MEDIA_CODEC_TEST, yuv420p_to_jpeg) {
    MediaCodecTestContext ctx = { 0 };
    const char *inputFileNameTemp = "input_yuv420p_1080p.yuv";

    memset(inputFileName, 0x00, sizeof(inputFileName));
    strncat(inputFileName, inputFilePath, strlen(inputFilePath));
    if (strlen(inputFileNameBak) == 0) {
        strncat(inputFileName, inputFileNameTemp, strlen(inputFileNameTemp));
    } else {
        strncat(inputFileName, inputFileNameBak, strlen(inputFileNameBak));
    }
    memset(outputFileNameTemp, 0x00, sizeof(outputFileNameTemp));
    strncat(outputFileNameTemp, outputFilePath, strlen(outputFilePath));
    strncat(outputFileNameTemp, outputFileNameBak, strlen(outputFileNameBak));


    ctx.fmt_name = "yuv420p";
    ctx.inputFileName = inputFileName;
    ctx.outputFileNameTemp = outputFileNameTemp;
    ctx.frame_buf_depth = frame_buf_depth;
    ctx.qfactor = qfactor;
    ctx.vb_num = vb_num;
    ctx.pic_w = pic_w;
    ctx.pic_h = pic_h;
    ctx.pic_size = pic_w * pic_h * 3 / 2;
    ctx.y_pic_size = pic_w * pic_h;   // data store 3 plane
    ctx.u_pic_size = pic_w * pic_h / 4;  // u_pic_size
    ctx.v_pic_size = pic_w * pic_h / 4;  // v_pic_size
    ctx.pix_fmt = HB_PIXEL_FORMAT_YUV420P;
    ctx.out_stream_num = out_stream_num;
    if (vb_cache_enable == -1) {
        ctx.vb_cache_enable = 1;
    } else {
        ctx.vb_cache_enable = vb_cache_enable;
    }
    do_jpeg_encoding(&ctx);
}

TEST_F(MEDIA_CODEC_TEST, yuyv422_to_jpeg) {
    MediaCodecTestContext ctx = { 0 };
    const char *inputFileNameTemp = "input_yuyv422_1080p.yuv";

    memset(inputFileName, 0x00, sizeof(inputFileName));
    memset(outputFileName, 0x00, sizeof(outputFileName));
    strncat(inputFileName, inputFilePath, strlen(inputFilePath));
    if (strlen(inputFileNameBak) == 0) {
        strncat(inputFileName, inputFileNameTemp, strlen(inputFileNameTemp));
    } else {
        strncat(inputFileName, inputFileNameBak, strlen(inputFileNameBak));
    }
    memset(outputFileNameTemp, 0x00, sizeof(outputFileNameTemp));
    strncat(outputFileNameTemp, outputFilePath, strlen(outputFilePath));
    strncat(outputFileNameTemp, outputFileNameBak, strlen(outputFileNameBak));

    ctx.fmt_name = "yvyu422";
    ctx.inputFileName = inputFileName;
    ctx.outputFileNameTemp = outputFileNameTemp;
    ctx.frame_buf_depth = frame_buf_depth;
    ctx.qfactor = qfactor;
    ctx.vb_num = vb_num;
    ctx.pic_w = pic_w;
    ctx.pic_h = pic_h;
    ctx.pic_size = pic_w * pic_h * 2;
    ctx.y_pic_size = pic_w * pic_h * 2;  // data store 1 plane
    ctx.u_pic_size = 0;
    ctx.v_pic_size = 0;
    ctx.pix_fmt = HB_PIXEL_FORMAT_YUYV422;
    ctx.out_stream_num = out_stream_num;
    if (vb_cache_enable == -1) {
        ctx.vb_cache_enable = 0;
    } else {
        ctx.vb_cache_enable = vb_cache_enable;
    }
    do_jpeg_encoding(&ctx);
}

TEST_F(MEDIA_CODEC_TEST, yuv422p_to_jpeg) {
    MediaCodecTestContext ctx = { 0 };
    const char *inputFileNameTemp = "input_yuv422p_1080p.yuv";

    memset(inputFileName, 0x00, sizeof(inputFileName));
    memset(outputFileName, 0x00, sizeof(outputFileName));
    strncat(inputFileName, inputFilePath, strlen(inputFilePath));
    if (strlen(inputFileNameBak) == 0) {
        strncat(inputFileName, inputFileNameTemp, strlen(inputFileNameTemp));
    } else {
        strncat(inputFileName, inputFileNameBak, strlen(inputFileNameBak));
    }
    memset(outputFileNameTemp, 0x00, sizeof(outputFileNameTemp));
    strncat(outputFileNameTemp, outputFilePath, strlen(outputFilePath));
    strncat(outputFileNameTemp, outputFileNameBak, strlen(outputFileNameBak));

    ctx.fmt_name = "yuv422p";
    ctx.inputFileName = inputFileName;
    ctx.outputFileNameTemp = outputFileNameTemp;
    ctx.frame_buf_depth = frame_buf_depth;
    ctx.qfactor = qfactor;
    ctx.vb_num = vb_num;
    ctx.pic_w = pic_w;
    ctx.pic_h = pic_h;
    ctx.pic_size = pic_w * pic_h * 2;
    ctx.y_pic_size = pic_w * pic_h;  // data store 3 plane
    ctx.u_pic_size = pic_w * pic_h / 2;
    ctx.v_pic_size = pic_w * pic_h / 2;
    ctx.pix_fmt = HB_PIXEL_FORMAT_YUV422P;
    ctx.out_stream_num = out_stream_num;
    if (vb_cache_enable == -1) {
        ctx.vb_cache_enable = 1;
    } else {
        ctx.vb_cache_enable = vb_cache_enable;
    }
    do_jpeg_encoding(&ctx);
}


}  // namespace media_codec_test
}  // namespace vision
}  // namespace horizon
