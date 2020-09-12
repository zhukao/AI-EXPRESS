//
// Copyright 2019 Horizon Robotics.
//

#include "ssd_method/ssd_post_process_module.h"

#include <sys/time.h>

#include <cmath>
#include <exception>

#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_sdk.h"
#include "horizon/vision_type/vision_type.hpp"

using hobot::vision::BBox;

namespace xstream {
#define check_coordinate_legal(x, max) \
  if ((x) < 0.0) (x) = 0.0;            \
  if ((x) > (max)) (x) = (max);

void SSDPostProcessModule::GetFrameOutput(
    const std::vector<BPU_TENSOR_S> &input_buf, BaseDataPtr frame_output) {
  std::vector<Detection> result_vec;
  auto output = std::static_pointer_cast<BaseDataVector>(frame_output);

  SSDAux(bpu_handle_, input_buf.data(), num_layer_,
         anchors_table_, result_vec, ssd_conf_, score_threshold_,
         nms_threshold_, output_layer_big_endian_, is_tf_, true);
  int len = result_vec.size();
  int base = std::pow(2, pyr_level_ / 4);
  for (int i = 0; i < len; i++) {
    auto det = result_vec.at(i);
    bool bOutput = false;
    if (model_out_type_.empty()) {
      bOutput = true;
    } else {
      for (auto e : model_out_type_) {
        if (e == det.id) {
          bOutput = true;
          break;
        }
      }
    }
    if (false == bOutput) {
      continue;
    }
    auto xstream_box =
        std::make_shared<xstream::XStreamData<hobot::vision::BBox>>();
    xstream_box->value.score = det.score;
    xstream_box->value.x1 = det.bbox.xmin * base;
    check_coordinate_legal(xstream_box->value.x1, 1920.);
    xstream_box->value.y1 = det.bbox.ymin * base;
    check_coordinate_legal(xstream_box->value.y1, 1080.);
    xstream_box->value.x2 = det.bbox.xmax * base;
    check_coordinate_legal(xstream_box->value.x2, 1920.);
    xstream_box->value.y2 = det.bbox.ymax * base;
    check_coordinate_legal(xstream_box->value.y2, 1080.);
    xstream_box->value.id = det.id;

    LOGD << xstream_box->value << ", type = " << xstream_box->value.id
         << std::endl;
    output->datas_.push_back(xstream_box);
  }
}

int SSDPostProcessModule::Init() {

  num_layer_ = bpu_handle_.output_num / 2;

  int ret =
      xstream::LoadConfig(ssd_params_file_.c_str(), &ssd_conf_, num_layer_);
  if (ret != 0) {
    LOGF << "ssd model params parse fail." << std::endl;
  }
  if (input_platform_ == "tensorflow") {
    is_tf_ = true;
  }
  SsdAnchorsGenerator(anchors_table_, bpu_handle_.outputs, ssd_conf_,
                      bpu_handle_.output_num, is_tf_);

  // MODULE_PERF_INIT
  LOGV << "\033[31m SSDPostProcessModule init finished \033[0m";
  return 0;
}

void SSDPostProcessModule::Reset() {}
}  // namespace xstream

