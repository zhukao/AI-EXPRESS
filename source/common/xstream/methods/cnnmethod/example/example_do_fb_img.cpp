//
// Created by zhengzheng.ge on 2019-06-03.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "CNNMethod/CNNMethod.h"
#include "CNNMethod/util/util.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/image_tools.h"
#include "hobotxsdk/xstream_sdk.h"
#include "horizon/vision_type/vision_type.hpp"
#include "opencv2/opencv.hpp"

typedef std::shared_ptr<hobot::vision::ImageFrame> ImageFramePtr;

static void Usage() {
  std::cout << "./example do_fb_img xstream_cfg_file img_list out_file\n";
}

using xstream::CNNMethod;

using xstream::BaseData;
using xstream::BaseDataPtr;
using xstream::BaseDataVector;
using xstream::InputData;
using xstream::InputDataPtr;
using xstream::XStreamData;

using hobot::vision::BBox;
using hobot::vision::CVImageFrame;
using hobot::vision::Landmarks;
using hobot::vision::Point;
using hobot::vision::Points;
using hobot::vision::SnapshotInfo;

extern void DumpAntiSpf(std::vector<BaseDataPtr> &, std::ostream &,
                        std::string);

void normalize_roi(float x1, float y1, float x2, float y2, int total_w,
                   int total_h, float &dst_x1, float &dst_y1, float &dst_x2,
                   float &dst_y2) {
  float width = x2 - x1;
  float height = y2 - y1;
  dst_x1 = x1;
  dst_y1 = y1;
  dst_x2 = x2;
  dst_y2 = y2;

  float expand_scale = 2.0f;
  if (width < height) {
    dst_x1 = std::max(0.0f, (dst_x1 + dst_x2) / 2.0f - height / 2.0f);
    dst_x2 = std::min(static_cast<float>(total_w),
                      (dst_x1 + dst_x2) / 2.0f + height / 2.0f);
  } else {
    dst_y1 = std::max(0.0f, (dst_y1 + dst_y2) / 2.0f - width / 2.0f);
    dst_y2 = std::min(static_cast<float>(total_h),
                      (dst_y1 + dst_y2) / 2.0f + width / 2.0f);
  }
  width = dst_x2 - dst_x1;
  height = dst_y2 - dst_y1;
  dst_x1 = std::max(dst_x1 - (expand_scale - 1.0f) * width / 2.0f, 0.0f);
  dst_y1 = std::max(dst_y1 - (expand_scale - 1.0f) * height / 2.0f, 0.0f);
  dst_x2 = std::min(static_cast<float>(total_w),
                    dst_x2 + (expand_scale - 1.0f) * width / 2.0f);
  dst_y2 = std::min(static_cast<float>(total_h),
                    dst_y2 + (expand_scale - 1.0f) * height / 2.0f);
  width = dst_x2 - dst_x1 + 1;
  height = dst_y2 - dst_y1 + 1;
  /* if (static_cast<int>(width) % 2) {
    dst_x2 -= 1;
  }
  if (static_cast<int>(height) % 2) {
    dst_y2 -= 1;
  } */
}

int DoFbImg(int argc, char **argv) {
  if (argc < 4) {
    Usage();
    return -1;
  }
  std::string cfg_file = argv[1];
  std::string img_list = argv[2];
  std::string output_file = argv[3];

  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();
  flow->SetConfig("config_file", cfg_file.c_str());
  flow->SetConfig("profiler", "on");
  flow->SetConfig("profiler_file", "./profiler.txt");
  flow->Init();

  std::ifstream ifs_img_list(img_list);
  if (!ifs_img_list.is_open()) {
    LOGD << "open image list file failed." << std::endl;
    return -1;
  }

  std::ofstream output(output_file, std::ios::out);

  std::string gt_data;
  std::string input_image;
  float x1, x2, y1, y2;
  float dst_x1, dst_y1, dst_x2, dst_y2;
  while (getline(ifs_img_list, gt_data)) {
    std::istringstream gt(gt_data);
    gt >> input_image;
    gt >> x1 >> y1 >> x2 >> y2;
    auto img_bgr = cv::imread(input_image);
    int width = img_bgr.cols;
    int height = img_bgr.rows;
    LOGD << "img w h:" << width << " " << height << std::endl;
    normalize_roi(x1, y1, x2, y2, width, height, dst_x1, dst_y1, dst_x2,
                  dst_y2);
    LOGD << "src x1 y1 x2 y2:" << x1 << " " << y1 << " " << x2 << " " << y2
         << std::endl
         << "dst x1 y1 x2 y2:" << dst_x1 << " " << dst_y1 << " " << dst_x2
         << " " << dst_y2 << std::endl;

    // crop img
    cv::Point top_left(dst_x1, dst_y1), bottom_right(dst_x2, dst_y2);
    cv::Mat crop_bgr = img_bgr(cv::Rect(top_left, bottom_right)).clone();

    // convert
    /* cv::cvtColor(crop_bgr, crop_bgr, CV_BGR2YUV_I420);
    uint8_t *output_nv12 = nullptr;
    int output_nv12_size, output_nv12_1_stride, output_nv12_2_stride;
    int ret = HobotXStreamConvertImage(crop_bgr.data, dst_w * dst_h * 3 / 2, dst_w,
                                    dst_h, dst_w, dst_w / 2,
    IMAGE_TOOLS_RAW_YUV_I420, IMAGE_TOOLS_RAW_YUV_NV12, &output_nv12,
    &output_nv12_size, &output_nv12_1_stride, &output_nv12_2_stride);
    HOBOT_CHECK(ret == 0) << "HobotXStreamConvertImage failed";
    cv::Mat img_nv12 = cv::Mat(dst_h * 3 / 2, dst_w, CV_8UC1);
    uint8_t *nv12 = img_nv12.ptr<uint8_t>();
    memcpy(nv12, output_nv12, output_nv12_size);
    HobotXStreamFreeImage(output_nv12); */
#if 0
    static int nv12_idx = 0;
    std::ofstream outFile("nv12_" + std::to_string(nv12_idx++) + ".dat",
                          std::ios::out | std::ios::binary);
    outFile.write((const char *)nv12, output_nv12_size);
    outFile.close();
#endif

    auto face_img = std::make_shared<CVImageFrame>();

    face_img->img = crop_bgr;
    face_img->pixel_format =
        HorizonVisionPixelFormat::kHorizonVisionPixelFormatRawBGR;
    /* face_img->img = img_nv12;
    face_img->pixel_format =
        HorizonVisionPixelFormat::kHorizonVisionPixelFormatRawNV12; */

    auto snap = std::make_shared<
        XStreamData<std::shared_ptr<SnapshotInfo<BaseDataPtr>>>>();
    auto snap_shot_info = std::make_shared<SnapshotInfo<BaseDataPtr>>();
    snap->value = snap_shot_info;
    snap_shot_info->snap = face_img;

    auto p_persons = std::make_shared<BaseDataVector>();
    auto p_one_person = std::make_shared<BaseDataVector>();
    p_persons->datas_.push_back(p_one_person);
    p_one_person->datas_.push_back(snap);

    xstream::InputDataPtr inputdata(new xstream::InputData());
    inputdata->datas_.push_back(
        std::static_pointer_cast<xstream::BaseData>(p_persons));
    auto out = flow->SyncPredict(inputdata);
    DumpAntiSpf(out->datas_, output, input_image);
  }
  delete flow;
  return 0;
}
