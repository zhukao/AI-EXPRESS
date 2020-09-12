/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: PreProcessor.cpp
 * @Brief: definition of the PreProcessor
 * @Author: zhe.sun
 * @Date: 2020-01-10 11:41:17
 * @Last Modified by: qingpeng.liu
 * @Last Modified time: 2020-05-19 17:26:20
 */

#include "CNNPredictor/CNNPredictor.h"
#include <string>
#include <vector>
#include <map>
#include "util/util.h"
#include "bpu_predict/bpu_predict.h"
#include "hobotlog/hobotlog.hpp"
#include "horizon/vision_type/vision_type.hpp"
#include "./plat_cnn.h"
#ifdef X3
#include "./bpu_predict_x3.h"
#endif

namespace xstream {
namespace CnnProc {
DECLARE_MethodCreator(lmk_pre)
DECLARE_MethodCreator(img_pre)
DECLARE_MethodCreator(rect_pre)
DECLARE_MethodCreator(vehicle_img_pre)

static std::map<std::string, std::function<MethodPtr()>>
  s_cnnmethod_pre_registry = {
  { "CNNLmkPreMethod",  lmk_pre_creator},
  { "CNNImgPreMethod",  img_pre_creator},
  { "CNNRectPreMethod",  rect_pre_creator},
  { "CNNVehicleImgPreMethod", vehicle_img_pre_creator}
};

bool CNNPredictorQuery(const std::string& method_name) {
  if (s_cnnmethod_pre_registry.count(method_name)) {
    return true;
  } else {
    return false;
  }
}

MethodPtr CNNPredictorCreate(const std::string& method_name) {
  HOBOT_CHECK(s_cnnmethod_pre_registry.count(method_name))
    << "CNNPredictorCreate error: " << method_name << std::endl;
  return s_cnnmethod_pre_registry[method_name]();
}

std::mutex CNNPredictor::init_mutex_;
int32_t CNNPredictor::Init(const std::string &cfg_path) {
  std::ifstream infile(cfg_path.c_str());
  HOBOT_CHECK(infile.good()) << "CNNMethod error config file path:" << cfg_path;

  std::stringstream buffer;
  buffer << infile.rdbuf();
  config_.reset(new CNNMethodConfig(buffer.str()));
  config_->SetSTDStringValue("parent_path", get_parent_path(cfg_path));
  std::unique_lock<std::mutex> lock(init_mutex_);

  // 1. model_name_
  model_version_ = config_->GetSTDStringValue("model_version", "unknown");
  model_name_ = config_->GetSTDStringValue("model_name");
  HOBOT_CHECK(model_name_.size() > 0) << "must set model_name";

  model_path_ = config_->GetSTDStringValue("model_file_path");
  std::string bpu_cfg_path = config_->GetSTDStringValue("bpu_config_path");
  max_handle_num_ = config_->GetIntValue("max_handle_num", -1);
  input_type_ = config_->GetSTDStringValue("in_msg_type");
  HOBOT_CHECK(model_path_.size() > 0) << "must set model_file_path";
  HOBOT_CHECK(bpu_cfg_path.size() > 0) << "must set bpu_config_cfg";

  std::string parent_path = config_->GetSTDStringValue("parent_path");
  model_path_ = parent_path + model_path_;
  bpu_cfg_path = parent_path + bpu_cfg_path;
  LOGD << "model_file_path:" << model_path_ << std::endl
       << "bpu_config_path:" << bpu_cfg_path
       << "parent path" << parent_path;

  // 2. load bpu
  bpu_handle_ = BPUModelManager::Get().GetBpuHandle(model_path_, bpu_cfg_path);
  LOGI << "BPU version:" << BPU_getVersion(bpu_handle_);

  BPUModelInfo output_model_info, input_model_info;
  int ret = BPU_getModelOutputInfo(
      bpu_handle_, model_name_.c_str(), &output_model_info);
  HOBOT_CHECK(ret == 0) << "get model output info failed, model:"
                        << model_name_;
  ret = BPU_getModelInputInfo(
      bpu_handle_, model_name_.c_str(), &input_model_info);
  HOBOT_CHECK(ret == 0) << "get model input info failed, model:"
                        << model_name_;

  // 3. model_info_
  model_info_.Init(bpu_handle_, model_name_, &input_model_info,
                    &output_model_info);

  // 4. feature_bufs_
  feature_bufs_.resize(model_info_.output_layer_size_.size());
  for (std::size_t i = 0; i < model_info_.output_layer_size_.size(); i++) {
    feature_bufs_[i].resize(model_info_.output_layer_size_[i]);
  }
  // 5. fake_img_handle_
  ret = BPU_createFakeImageHandle(model_info_.input_nhwc_[1],
                                  model_info_.input_nhwc_[2],
                                  &fake_img_handle_);
  HOBOT_CHECK(ret == 0) << "create fake image handle failed";
  return 0;
}

void CNNPredictor::Finalize() {
  if (fake_img_handle_) {
    BPU_releaseFakeImageHandle(fake_img_handle_);
  }
  if (bpu_handle_) {
    BPUModelManager::Get().ReleaseBpuHandle(model_path_);
  }
}

int CNNPredictor::UpdateParameter(xstream::InputParamPtr ptr) {
  if (ptr->is_json_format_) {
    std::string content = ptr->Format();
    CNNMethodConfig cf(content);
    UpdateParams(cf.config, config_->config);
    return 0;
  } else {
    HOBOT_CHECK(0) << "only support json format config";
    return -1;
  }
}

InputParamPtr CNNPredictor::GetParameter() const {
  return std::static_pointer_cast<xstream::InputParam>(config_);
}

std::string CNNPredictor::GetVersion() const {
  return "";  // TODO(zhe.sun)
}

int CNNPredictor::RunModelFromResizer(
    hobot::vision::PymImageFrame &pym_image, BPUBBox *box, int box_num,
    int *resizable_cnt, BPU_Buffer_Handle *output_buf, int output_size,
    BPUModelHandle *model_handle) {
#ifdef X2
  int ret = BPU_runModelFromResizer(
      bpu_handle_, model_name_.c_str(),
      reinterpret_cast<BPUPyramidBuffer>(&(pym_image.img)), box, box_num,
      resizable_cnt, output_buf, output_size, model_handle);
#endif
#ifdef X3
  bpu_predict_x3::PyramidResult bpu_predict_pyramid;
  Convert(pym_image, bpu_predict_pyramid);
  int ret = BPU_runModelFromResizer(
      bpu_handle_, model_name_.c_str(),
      static_cast<BPUPyramidBuffer>(&bpu_predict_pyramid), box, box_num,
      resizable_cnt, output_buf, output_size, model_handle);
#endif
  if (ret != 0 && *resizable_cnt == 0) {
    LOGI << "no box pass resizer";
    return -1;
  } else if (ret != 0) {
    LOGE << "BPU_runModelFromResizer failed:" << BPU_getLastError(bpu_handle_);
    return -1;
  }

  return 0;
}

int CNNPredictor::RunModelFromImage(uint8_t *data,
                                 int data_size,
                                 BPU_Buffer_Handle *output_buf,
                                 int output_size,
                                 BPUModelHandle *model_handle,
                                 BPUFakeImage *fake_img_ptr) {
  fake_img_ptr = BPU_getFakeImage(fake_img_handle_, data, data_size);
  if (fake_img_ptr == nullptr) {
    LOGE << "get fake image failed";
    return -1;
  }

  int ret = BPU_runModelFromImage(bpu_handle_,
                                  model_name_.c_str(),
                                  fake_img_ptr,
                                  output_buf,
                                  output_size,
                                  model_handle);
  if (ret != 0) {
    LOGE << "BPU_runModelFromImage failed:" << BPU_getLastError(bpu_handle_);
    BPU_releaseFakeImage(fake_img_handle_, fake_img_ptr);
    return -1;
  }

  return 0;
}

int CNNPredictor::NormalizeRoi(hobot::vision::BBox *src,
                            hobot::vision::BBox *dst,
                            NormParams norm_params,
                            uint32_t total_w,
                            uint32_t total_h,
                            FilterMethod filter_method) {
  *dst = *src;
  float box_w = dst->x2 - dst->x1;
  float box_h = dst->y2 - dst->y1;
  float center_x = (dst->x1 + dst->x2) / 2.0f;
  float center_y = (dst->y1 + dst->y2) / 2.0f;
  float w_new = box_w;
  float h_new = box_h;
  NormMethod norm_method = norm_params.norm_type;
  float norm_ratio = norm_params.expand_scale;
  float aspect_ratio = norm_params.aspect_ratio;

  switch (norm_method) {
    case NormMethod::BPU_MODEL_NORM_BY_WIDTH_LENGTH: {
      w_new = box_w * norm_ratio;
      h_new = box_h + w_new - box_w;
      if (h_new <= 0) return -1;
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_WIDTH_RATIO:
    case NormMethod::BPU_MODEL_NORM_BY_HEIGHT_RATIO:
    case NormMethod::BPU_MODEL_NORM_BY_LSIDE_RATIO: {
      h_new = box_h * norm_ratio;
      w_new = box_w * norm_ratio;
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_HEIGHT_LENGTH: {
      h_new = box_h * norm_ratio;
      w_new = box_w + h_new - box_h;
      if (w_new <= 0) return -1;
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_LSIDE_LENGTH: {
      if (box_w > box_h) {
        w_new = box_w * norm_ratio;
        h_new = box_h + w_new - box_w;
        if (h_new <= 0) return -1;
      } else {
        h_new = box_h * norm_ratio;
        w_new = box_w + h_new - box_h;
        if (w_new <= 0) return -1;
      }
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_LSIDE_SQUARE: {
      if (box_w > box_h) {
        w_new = box_w * norm_ratio;
        h_new = w_new / aspect_ratio;
      } else {
        h_new = box_h * norm_ratio;
        w_new = h_new * aspect_ratio;
      }
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_DIAGONAL_SQUARE: {
      float diagonal = sqrt(pow(box_w, 2.0) + pow(box_h, 2.0));
      w_new = h_new = diagonal * norm_ratio;
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_WIDTH_SQUARE: {
        w_new = box_w * norm_ratio;
        h_new = w_new;
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_HEIGHT_SQUARE: {
        h_new = box_h * norm_ratio;
        w_new = h_new;
    } break;
    case NormMethod::BPU_MODEL_NORM_BY_NOTHING:
      break;
    default:
      return 0;
  }
  dst->x1 = center_x - w_new / 2;
  dst->x2 = center_x + w_new / 2;
  dst->y1 = center_y - h_new / 2;
  dst->y2 = center_y + h_new / 2;

  if (FilterRoi(src, dst, total_w, total_h, filter_method)) {
    *dst = *src;
    return -1;
  }

  dst->x1 = dst->x1 < 0 ? 0.0f : dst->x1;
  dst->y1 = dst->y1 < 0 ? 0.0f : dst->y1;
  dst->x2 = dst->x2 > total_w ? total_w : dst->x2;
  dst->y2 = dst->y2 > total_h ? total_h : dst->y2;
  LOGD << "norm roi[x1, y1, x2, y2]: [" << dst->x1 << ", " << dst->y1 << ", "
       << dst->x2 << ", " << dst->y2 << "]";
  return 0;
}

int CNNPredictor::FilterRoi(hobot::vision::BBox *src,
                         hobot::vision::BBox *dst,
                         int src_w,
                         int src_h,
                         FilterMethod filter_method) {
  switch (filter_method) {
    case FilterMethod::OUT_OF_RANGE: {
      if (dst->x1 < 0 || dst->y1 < 0 || dst->x2 > src_w || dst->y2 > src_h)
        return -1;
    } break;
    default:
      return 0;
  }
  return 0;
}

}  // namespace CnnProc
}  // namespace xstream
