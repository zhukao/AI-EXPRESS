// Copyright 2006, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <fstream>
#include "gtest/gtest.h"
#include "hobotlog/hobotlog.hpp"
#include "./media_codec_test.h"
#ifdef __cplusplus
extern "C" {
#include "./hb_venc.h"
#include "./hb_comm_video.h"
#include "./hb_common.h"
#include "./hb_comm_venc.h"
}
#endif

static const char short_options[] = "W:H:F:I:i:O:o:D:N:Q:C:h";
static const struct option long_options[] = {
    {"width", required_argument, nullptr, 'W'},
    {"height", required_argument, nullptr, 'H'},
    {"pix_format", required_argument, nullptr, 'F'},
    {"input_file_path", required_argument, nullptr, 'I'},
    {"input_file_name", required_argument, nullptr, 'i'},
    {"output_file_path", required_argument, nullptr, 'O'},
    {"output_file_name", required_argument, nullptr, 'o'},
    {"frame_buf_depth", required_argument, nullptr, 'D'},
    {"out_stream_num", required_argument, nullptr, 'N'},
    {"quality_factor", required_argument, nullptr, 'Q'},
    {"vb_cache_enable", required_argument, nullptr, 'C'},
    {"help", required_argument, nullptr, 'h'},
    {nullptr, 0, nullptr, 0}};

static std::map<std::string, int> g_pix_fmt_map = {
    {"yuv420p", HB_PIXEL_FORMAT_YUV420P},
    {"nv12", HB_PIXEL_FORMAT_NV12},
    {"nv21", HB_PIXEL_FORMAT_NV21},
    {"yuv422p", HB_PIXEL_FORMAT_YUV422P},
    {"nv16", HB_PIXEL_FORMAT_NV16},
    {"nv61", HB_PIXEL_FORMAT_NV61},
    {"uyvy422", HB_PIXEL_FORMAT_UYVY422},
    {"vyuy422", HB_PIXEL_FORMAT_VYUY422},
    {"yuyv422", HB_PIXEL_FORMAT_YUYV422},
    {"yvyu422", HB_PIXEL_FORMAT_YVYU422},
    {"yuv440p", HB_PIXEL_FORMAT_YUV440P},
    {"yuv444", HB_PIXEL_FORMAT_YUV444},
    {"yuv444p", HB_PIXEL_FORMAT_YUV444P},
    {"nv24", HB_PIXEL_FORMAT_NV24},
    {"nv42", HB_PIXEL_FORMAT_NV42},
    {"yuv400", HB_PIXEL_FORMAT_YUV400},
};

cmdOptions_t cmd_options;

static int parse_pixfmt_func(const std::string& pix_fmt) {
    int ret = 0;
    int mGlobalWidth = cmd_options.mGlobalWidth;
    int mGlobalHeight = cmd_options.mGlobalHeight;
    auto iter = g_pix_fmt_map.find(pix_fmt);
    if (iter == g_pix_fmt_map.end()) {
        LOGE << "pixel format is unknow: " << pix_fmt;
        return -1;
    } else {
        cmd_options.mGlobalPixFmt = iter->second;
        LOGI << "mGlobalPixFmt: " << cmd_options.mGlobalPixFmt;
    }

    switch (cmd_options.mGlobalPixFmt) {
    case HB_PIXEL_FORMAT_YUV420P:
    case HB_PIXEL_FORMAT_NV12:
    case HB_PIXEL_FORMAT_NV21:
        {
            cmd_options.mGlobalSize = mGlobalWidth * mGlobalHeight * 3 / 2;
            cmd_options.mGlobalySize = mGlobalWidth * mGlobalHeight;
            cmd_options.mGlobaluvSize = mGlobalWidth * mGlobalHeight / 2;
            break;
        }
    case HB_PIXEL_FORMAT_YUV422P:
    case HB_PIXEL_FORMAT_NV16:
    case HB_PIXEL_FORMAT_NV61:
    case HB_PIXEL_FORMAT_UYVY422:
    case HB_PIXEL_FORMAT_VYUY422:
    case HB_PIXEL_FORMAT_YUYV422:
    case HB_PIXEL_FORMAT_YVYU422:
    case HB_PIXEL_FORMAT_YUV440P:
        {
            cmd_options.mGlobalSize = mGlobalWidth * mGlobalHeight * 2;
            cmd_options.mGlobalySize = mGlobalWidth * mGlobalHeight;
            cmd_options.mGlobaluvSize = mGlobalWidth * mGlobalHeight;
            break;
        }
    case HB_PIXEL_FORMAT_YUV444:
    case HB_PIXEL_FORMAT_YUV444P:
    case HB_PIXEL_FORMAT_NV24:
    case HB_PIXEL_FORMAT_NV42:
        {
            cmd_options.mGlobalSize = mGlobalWidth * mGlobalHeight * 3;
            cmd_options.mGlobalySize = mGlobalWidth * mGlobalHeight;
            cmd_options.mGlobaluvSize = mGlobalWidth * mGlobalHeight * 2;
            break;
        }
    case HB_PIXEL_FORMAT_YUV400:
        {
            cmd_options.mGlobalSize = mGlobalWidth * mGlobalHeight * 3;
            cmd_options.mGlobalySize = mGlobalWidth * mGlobalHeight;
            cmd_options.mGlobaluvSize = mGlobalWidth * mGlobalHeight * 2;
            break;
        }
    default:
        {
            LOGE << "not support pix format: " << cmd_options.mGlobalPixFmt;
            ret = -1;
            break;
        }
    }
    return ret;
}

static bool path_exists(const std::string &fn, bool is_dir) {
    if (is_dir) {
        struct stat myStat = {0};
        if ((stat(fn.c_str(), &myStat) != 0) || !S_ISDIR(myStat.st_mode)) {
            LOGE << "Directory [ " << fn << "] does not exist.";
            return false;
        }
        return true;
    } else {
        std::ifstream file_path(fn.c_str());
        return file_path.good();
    }
}

static int check_filepath_valid(const std::string& file_path) {
    bool bret;

    bret = path_exists(file_path, true);
    if (!bret) {
        LOGE << "input file path: " << file_path << " does not exist!";
        HOBOT_CHECK(bret == true);
    }

    return 0;
}

static int check_filename_valid(const std::string& file_name) {
    bool bret;

    bret = path_exists(file_name, false);
    if (!bret) {
        LOGE << "input file name: " << file_name << " does not exist!";
        HOBOT_CHECK(bret == true);
    }

    return 0;
}

void print_usage(const char *prog)
{
    LOGI << "Usage: " << prog;
    puts("  -W --width             \t image width\n"
         "  -H --height            \t image height\n"
         "  -F --pix_format        \t input image pixel format"
         "(nv12, yuv420p, yuyv422, yuv422p and so on)\n"
         "  -I --input_file_path   \t input yuv file path\n"
         "  -i --input_file_name   \t input yuv file name\n"
         "  -O --output_file_path  \t output jpeg file path\n"
         "  -o --output_file_name  \t output jpeg file name\n"
         "  -D --frame_buf_depth   \t input yuv frame buffer depth\n"
         "  -C --vb_cache_enable   \t input vb cache enable\n"
         "  -N --out_stream_num    \t output jpeg stream number\n"
         "  -Q --quality_factor    \t output jpeg quality factor\n"
         "  -h --help              \t print usage\n");
    exit(1);
}

int parse_opts(int argc, char *argv[])
{
    int ret;

    while (1) {
        int cmd_ret;

        cmd_ret =
            getopt_long(argc, argv, short_options, long_options, NULL);

        if (cmd_ret == -1)
            break;
        LOGD << " cmd_ret:" << cmd_ret << " optarg:" << optarg;
        switch (cmd_ret) {
        case 'W':
            {
                cmd_options.mGlobalWidth = atoi(optarg);
                LOGI << "mGlobalWidth: " << cmd_options.mGlobalWidth;
                break;
            }
        case 'H':
            {
                cmd_options.mGlobalHeight = atoi(optarg);
                LOGI << "mGlobalHeight: " << cmd_options.mGlobalHeight;
                break;
            }
        case 'F':
            {
                ret = parse_pixfmt_func(optarg);
                if (ret) {
                    LOGE << "parse pixel format error!!!";
                    return -1;
                }
                break;
            }
        case 'I':
            {
                ret = check_filepath_valid(optarg);
                if (ret) {
                    LOGE << "parse input file path error!!!";
                    return -1;
                }
                const char* input_file_path = optarg;
                cmd_options.mGlobalInFilePath = input_file_path;
                LOGI << "mGlobalInFilePath: " << cmd_options.mGlobalInFilePath;
                break;
            }
        case 'i':
            {
                auto file_name = cmd_options.mGlobalInFilePath + optarg;
                ret = check_filename_valid(file_name);
                if (ret) {
                    LOGE << "parse input file name error!!!";
                    return -1;
                }
                const char* input_file_name = optarg;
                cmd_options.mGlobalInFileNameBak = input_file_name;
                LOGI << "mGlobalInFileNameBak: "
                     << cmd_options.mGlobalInFileNameBak;
                break;
            }
        case 'O':
            {
                ret = check_filepath_valid(optarg);
                if (ret) {
                    LOGE << "parse output file path error!!!";
                    return -1;
                }
                const char* output_file_path = optarg;
                cmd_options.mGlobalOutFilePath = output_file_path;
                LOGI << "mGlobalOutFilePath: "
                     << cmd_options.mGlobalOutFilePath;
                break;
            }
        case 'o':
            {
                ret = check_filename_valid(optarg);
                if (ret) {
                    LOGE << "parse output file name error!!!";
                    return -1;
                }
                const char* output_file_name = optarg;
                cmd_options.mGlobalOutFileNameBak = output_file_name;
                LOGI << "mGlobalOutFileNameBak: "
                     << cmd_options.mGlobalOutFileNameBak;
                break;
            }
        case 'D':
            {
                cmd_options.mGlobalBufDepth = atoi(optarg);
                LOGI << "mGlobalBufDepth:" << cmd_options.mGlobalBufDepth;
                break;
            }
        case 'N':
            {
                cmd_options.mGlobalStreamNum = atoi(optarg);
                LOGI << "mGlobalStreamNum: " << cmd_options.mGlobalStreamNum;
                break;
            }
        case 'Q':
            {
                cmd_options.mGlobalQfactor = atoi(optarg);
                LOGI << "mGlobalQfactor: " << cmd_options.mGlobalQfactor;
                break;
            }
        case 'C':
            {
                cmd_options.mGlobalVbCacheEnable = atoi(optarg);
                LOGI << "mGlobalVbCacheEnable: "
                     << cmd_options.mGlobalVbCacheEnable;
                break;
            }
        case 'h':
            {
                print_usage(argv[0]);
                break;
            }
        }
    }

    return 0;
}

GTEST_API_ int main(int argc, char **argv) {
    int ret;
    printf("Running main() from gtest_main.cc\n");
    SetLogLevel(HOBOT_LOG_INFO);
    testing::InitGoogleTest(&argc, argv);

    ret = parse_opts(argc, argv);
    if (ret) {
        return ret;
    }




  return RUN_ALL_TESTS();
}

