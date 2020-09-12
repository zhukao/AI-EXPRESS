/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @author    yaoyao.sun
 * @date      2019.04.12
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "./dump_util.h"
#include "./stopwatch.h"
#include "FasterRCNNMethod/FasterRCNNMethod.h"
#include "FasterRCNNMethod/faster_rcnn_imp.h"
#include "FasterRCNNMethod/yuv_utils.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_sdk.h"
#ifdef X2
#include "./vio_wrapper.h"
#endif
#ifdef X3
#include "./vio_wrapper_global.h"
#endif
#include "json/json.h"
#include "opencv2/opencv.hpp"
#include "horizon/vision_type/vision_type.hpp"

using xstream::BaseData;
using xstream::BaseDataPtr;
using xstream::BaseDataVector;
using xstream::InputData;
using xstream::InputDataPtr;
using xstream::XStreamData;

using xstream::FasterRCNNMethod;
using hobot::vision::ImageFrame;
using hobot::vision::PymImageFrame;

static void Usage() {
  std::cout << "./FasterRCNNMethod_example dump_face_det_result image_list\n";
}

int DumpFaceDetResult(int argc, char **argv) {
  if (argc < 2) {
    Usage();
    return -1;
  }
  FasterRCNNMethod faster_rcnn_method;
  std::string image_list = argv[1];
  std::string model_config_file = "./configs/face_config.json";

  faster_rcnn_method.Init(model_config_file);

  std::ifstream ifs(image_list);
  if (!ifs.is_open()) {
    std::cout << "open image list file failed." << std::endl;
    return -1;
  }
  std::string input_image;
#ifdef X2
  HbVioFbWrapper fb_vio("./configs/vio_onsemi0230_fb.json");
  auto ret = fb_vio.Init();
  HOBOT_CHECK(ret == 0) << "fb vio init failed!!!";
  img_info_t data;

  std::ofstream ofs("face_det_result.txt");
  while (getline(ifs, input_image)) {
    cv::Mat bgr_img = cv::imread(input_image);
    int width = bgr_img.cols;
    int height = bgr_img.rows;
    cv::Mat img_nv12;
    bgr_to_nv12(bgr_img.data, height, width, img_nv12);
    ret = fb_vio.GetImgInfo(img_nv12.data, width, height, &data);
    HOBOT_CHECK(ret == 0) << "fb vio get image failed!!!";
    // vio_debug::print_info(data);

    auto py_image_frame_ptr = std::make_shared<PymImageFrame>();
    py_image_frame_ptr->img = data;

    auto xstream_pyramid =
        std::make_shared<XStreamData<std::shared_ptr<ImageFrame>>>();
    xstream_pyramid->value = py_image_frame_ptr;

    std::vector<std::vector<BaseDataPtr>> input;
    std::vector<xstream::InputParamPtr> param;
    input.resize(1);
    input[0].push_back(xstream_pyramid);
    auto xstream_output = faster_rcnn_method.DoProcess(input, param);
    assert(xstream_output.size() == 1);
    auto faster_rcnn_out = xstream_output[0];
    static int frame_cnt = 0;
    ofs << input_image;
    for (auto &in_rect :
        std::static_pointer_cast<BaseDataVector>(faster_rcnn_out[0])->datas_) {
      auto rect = std::static_pointer_cast<XStreamData<BBox>>(in_rect);
      ofs << " " << rect->value.x1 << " " << rect->value.y1 << " "
          << rect->value.x2 << " " << rect->value.y2 << " "
          << rect->value.score;
    }
    ofs << "\n";
    ret = fb_vio.FreeImgInfo(&data);
    HOBOT_CHECK(ret == 0) << "fb vio free image failed!!!";
    LOGD << "predict success: " << ++frame_cnt;
  }
#endif
#ifdef X3
  HbVioFbWrapperGlobal fb_vio("./configs/vio_onsemi0230_fb.json");
  auto ret = fb_vio.Init();
  HOBOT_CHECK(ret == 0) << "fb vio init failed!!!";

  std::ofstream ofs("face_det_result.txt");
  while (getline(ifs, input_image)) {
    cv::Mat bgr_img = cv::imread(input_image);
    int width = bgr_img.cols;
    int height = bgr_img.rows;
    cv::Mat img_nv12;
    bgr_to_nv12(bgr_img.data, height, width, img_nv12);
    auto py_image_frame_ptr = fb_vio.GetImgInfo(img_nv12.data, width, height);
    HOBOT_CHECK(py_image_frame_ptr != nullptr) << "fb vio get image failed!!!";

    auto xstream_pyramid =
        std::make_shared<XStreamData<std::shared_ptr<ImageFrame>>>();
    xstream_pyramid->value = py_image_frame_ptr;

    std::vector<std::vector<BaseDataPtr>> input;
    std::vector<xstream::InputParamPtr> param;
    input.resize(1);
    input[0].push_back(xstream_pyramid);
    auto xstream_output = faster_rcnn_method.DoProcess(input, param);
    assert(xstream_output.size() == 1);
    auto faster_rcnn_out = xstream_output[0];
    static int frame_cnt = 0;
    ofs << input_image;
    for (auto &in_rect :
        std::static_pointer_cast<BaseDataVector>(faster_rcnn_out[0])->datas_) {
      auto rect = std::static_pointer_cast<XStreamData<BBox>>(in_rect);
      ofs << " " << rect->value.x1 << " " << rect->value.y1 << " "
          << rect->value.x2 << " " << rect->value.y2 << " "
          << rect->value.score;
    }
    ofs << "\n";
    ret = fb_vio.FreeImgInfo(py_image_frame_ptr);
    HOBOT_CHECK(ret == 0) << "fb vio free image failed!!!";
    LOGD << "predict success: " << ++frame_cnt;
  }
#endif
  faster_rcnn_method.Finalize();
  return 0;
}
