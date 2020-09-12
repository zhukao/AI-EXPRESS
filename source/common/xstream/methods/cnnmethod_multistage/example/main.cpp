/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: CNNMethodPredictor.cpp
 * @Brief: definition of the CNNMethodPredictor
 * @Author: zhengzheng.ge
 * @Email: zhengzheng.ge@horizon.ai
 * @Date: 2019-04-15 14:27:05
 * @Last Modified by: zhengzheng.ge
 * @Last Modified time: 2019-05-06 16:23:27
 */
#include <iostream>
#include <map>
#include <string>
#include "hobotlog/hobotlog.hpp"
#include "horizon/vision_type/vision_type.hpp"
#include "horizon/vision_type/vision_type_common.h"
#include "opencv2/opencv.hpp"
#ifdef X2
#include "./vio_wrapper.h"
#endif
#ifdef X3
#include "./vio_wrapper_global.h"
#endif

typedef int (*example_fn)(int, char **);

extern int DoFbRectCnn(int argc, char **argv);
extern int DoFbFeature(int argc, char **argv);

int PrintUsage() {
  std::cout
      << "Usage: example [command] [args]\n"
      << "\n"
      << "Command:\n"
      << "         get_model_info model_file bpu_config\n"
      << "         do_pyramid\n"
      << "         do_fb_det_cnn "
         "[pose_lmk|age_gender|anti_spf] xstream_cfg_file "
         "fb_cfg img_list out_file\n"
      << "         do_fb_feature xstream_cfg_file img_lmk_list out_file\n"
      << "         do_fb_img xstream_cfg_file img_list out_file\n"
      << "         do_fb hb_cfg_file file_name\n"
      << "         do_fb_rect_cnn [pose_lmk|age_gender|anti_spf|face_quality] "
         "xstream_cfg_file fb_cfg img_list out_file\n"
      << "         do_det xstream_cfg_file fb_cfg img_list\n"
      << "         ver_feature model_file bpu_config "
         "nv12_after_affine_list.txt out_file\n"
      << "         ver_rect_pyd method_cfg_file hb_vio_cfg_file gt.txt "
         "out_file\n"
      << "         det_from_camera xstream_config vio_config_file "
         "camera_config_file output\n";
  return 0;
}

static std::map<std::string, example_fn> examples = {
    {"do_fb_feature", DoFbFeature},
    {"do_fb_rect_cnn", DoFbRectCnn},
};

int main(int argc, char **argv) {
  SetLogLevel(HOBOT_LOG_DEBUG);
  if (argc > 1 && examples.count(argv[1])) {
    examples[argv[1]](argc - 1, argv + 1);
    std::cout << "main thread exit" << std::endl;
  } else {
    std::cout << "incorrect parameters" << std::endl;
  }
  return 0;
}
