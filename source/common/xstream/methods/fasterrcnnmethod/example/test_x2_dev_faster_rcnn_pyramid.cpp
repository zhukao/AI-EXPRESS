/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @author    yaoyao.sun
 * @date      2019.04.12
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fstream>

#include "opencv2/opencv.hpp"
#include "hobotxsdk/xstream_sdk.h"
#include "horizon/vision_type/vision_type.hpp"

#include "FasterRCNNMethod/FasterRCNNMethod.h"
#include "FasterRCNNMethod/yuv_utils.h"
#include "./dump_util.h"
#ifdef X2
#include "./vio_wrapper.h"
#endif
#ifdef X3
#include "./vio_wrapper_global.h"
#endif
#include "hobotlog/hobotlog.hpp"

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
  std::cout << "./FasterRCNNMethod_example x2_dev_faster_rcnn_pyramid "
               "[get_vio_times]\n";
}

int TestX2DEVFasterRCNNPyramid(int argc, char **argv) {
  if (argc < 2) {
    Usage();
  }
#ifdef X2
  FasterRCNNMethod faster_rcnn_method;
  faster_rcnn_method.Init("./configs/face_pose_lmk_config.json");
  std::cout << "fasterrcnn init success." << std::endl;

  HbVioDualCamera dual_camera("./configs/vio_onsemi_dual.json",
                              "./configs/hb_x2dev.json");

  int get_vio_cnt = 1;
  if (argc > 1) {
    get_vio_cnt = atoi(argv[1]);
  }

  mult_img_info_t data;
  for (int i = 0 ; i < get_vio_cnt; ++i) {
    auto ret = dual_camera.GetMultiImage(&data);
    HOBOT_CHECK(ret == 0) << "dual camera get multi image failed!!!";
    // 构造mat
    auto img0 = data.img_info[0];
    int img_height = img0.src_img.height;
    int img_witdh = img0.src_img.step;
    int img_y_len = img_height * img_witdh;
    int img_uv_len = img_height * img_witdh / 2;
    uint8_t *img_ptr = static_cast<uint8_t*>(malloc(img_y_len+img_uv_len));
    memcpy(img_ptr, reinterpret_cast<uint8_t *>(img0.src_img.y_vaddr),
           img_y_len);
    memcpy(img_ptr + img_y_len,
           reinterpret_cast<uint8_t *>(img0.src_img.c_vaddr), img_uv_len);
    cv::Mat bgr_mat;
    nv12_to_bgr(img_ptr, img_height, img_witdh, bgr_mat);
    free(img_ptr);
    cv::imwrite("pyramid_img.jpg", bgr_mat);

    auto py_image_frame_ptr = std::make_shared<PymImageFrame>();

    py_image_frame_ptr->img = img0;

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
#if 1
    static int dump_cnt = 0;
    if (dump_cnt++ < 5) {
      DumpFaceLmk(&bgr_mat, faster_rcnn_out);
    }
#endif
    std::cout << "predict success: " << i << std::endl;
    ret = dual_camera.Free(&data);
    HOBOT_CHECK(ret == 0) << "dual camera free multi image failed!!!";
  }
#endif
  return 0;
}




