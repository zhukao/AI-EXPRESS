
/***************************************************************************
 * COPYRIGHT NOTICE
 * Copyright 2019 Horizon Robotics, Inc.
 * All rights reserved.
 ***************************************************************************/
#ifndef INCLUDE_MEDIA_CODEC_TEST_H_
#define INCLUDE_MEDIA_CODEC_TEST_H_
#include <string>
#ifdef __cplusplus
extern "C" {
#include "./hb_venc.h"
#include "./hb_comm_video.h"
#include "./hb_common.h"
#include "./hb_comm_venc.h"
}
#endif

typedef struct cmdOptions_s {
        int mGlobalWidth = 1920;
        int mGlobalHeight = 1080;
        int mGlobalSize = mGlobalWidth * mGlobalHeight;
        int mGlobalySize = mGlobalWidth * mGlobalHeight * 3 / 2;
        int mGlobaluvSize = mGlobalWidth * mGlobalHeight / 2;
        int mGlobalPixFmt = HB_PIXEL_FORMAT_NV12;
        int mGlobalBufDepth = 5;
        int mGlobalStreamNum = 3;
        int mGlobalQfactor = 50;
        int mGlobalVbCacheEnable = -1;
        std::string mGlobalInFilePath = "./data/";
        std::string mGlobalInFileNameBak = "";
        std::string mGlobalOutFilePath = "./data/";
        std::string mGlobalOutFileNameBak = "output_stream";
        int mGlobalIsCbr = 1;
        int mGlobalBitrate = 2000;
} cmdOptions_t;

#endif  // INCLUDE_MEDIA_CODEC_TEST_H_
