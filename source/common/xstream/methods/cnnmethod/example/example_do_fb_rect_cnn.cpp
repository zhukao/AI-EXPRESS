/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: example_do_pose_lmk.cpp
 * @Brief:
 * @Author: zhengzheng.ge
 * @Email: zhengzheng.ge@horizon.ai
 * @Date: 2019-04-15 14:27:05
 * @Last Modified by: zhengzheng.ge
 * @Last Modified time: 2019-04-15 15:18:10
 */

#include <stdint.h>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include "CNNMethod/CNNMethod.h"
#include "CNNMethod/util/util.h"
#include "FasterRCNNMethod/FasterRCNNMethod.h"
#include "bpu_predict/bpu_io.h"
#include "bpu_predict/bpu_predict.h"
#include "hobotxstream/method.h"
#include "hobotxstream/method_factory.h"
#include "hobotxsdk/xstream_sdk.h"
#include "horizon/vision_type/vision_type.hpp"
#include "horizon/vision_type/vision_type_common.h"
#include "opencv2/opencv.hpp"
#ifdef X2
#include "./vio_wrapper.h"
#endif
#ifdef X3
#include "./vio_wrapper_global.h"
#endif

typedef std::shared_ptr<hobot::vision::ImageFrame> ImageFramePtr;
using xstream::BaseDataPtr;

struct PyramidResult {
  img_info_t result_info;
};

static void Usage() {
  std::cout << "./example do_fb_rect_cnn "
               "[pose_lmk|age_gender|anti_spf|face_quality|"
               "vehicle_type|vehicle_color|plate_num]"
               "xstream_cfg_file fb_cfg img_list out_file"
            << std::endl;
}

void DumpPoseLmk(std::vector<BaseDataPtr> &, std::ostream &, std::string);
void DumpAgeGender(std::vector<BaseDataPtr> &, std::ostream &, std::string);
void DumpAntiSpf(std::vector<BaseDataPtr> &, std::ostream &, std::string);
void DumpFaceQuality(std::vector<BaseDataPtr> &, std::ostream &, std::string);
void DumpVehicleColor(std::vector<BaseDataPtr> &, std::ostream &, std::string);
void DumpVehicleType(std::vector<BaseDataPtr> &, std::ostream &, std::string);
void DumpPlateNum(std::vector<BaseDataPtr> &, std::ostream &, std::string);
void DumpVehiclePhone(std::vector<BaseDataPtr> &, std::ostream &, std::string);
void DumpVehicleBelt(std::vector<BaseDataPtr> &, std::ostream &, std::string);
void DumpPersonAttrs(std::vector<BaseDataPtr> &, std::ostream &, std::string);

int DoFbRectCnn(int argc, char **argv) {
  if (argc < 6) {
    Usage();
    return 1;
  }
  std::string model_name(argv[1]);
  std::string cfg_file(argv[2]);
  std::string fb_cfg(argv[3]);
  std::string img_list(argv[4]);
  std::string rlt_put_file = argv[5];

  xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();
  flow->SetConfig("config_file", cfg_file.c_str());
  flow->SetConfig("profiler", "on");
  flow->SetConfig("profiler_file", "./profiler.txt");
  flow->Init();
#ifdef X2
  img_info_t feed_back_info;
  HbVioFbWrapper fb_handle(fb_cfg);
  fb_handle.Init();
  std::ifstream img_list_file(img_list);
  std::string img_path;
  std::string gt_line;
  std::ofstream output(rlt_put_file, std::ios::out);
  float x1, y1, x2, y2;
  while (getline(img_list_file, gt_line)) {
    std::istringstream gt(gt_line);
    gt >> img_path;
    gt >> x1 >> y1 >> x2 >> y2;
    if (x1 < 0 || x2 >= 1920 || y1 < 0 || y2 >= 1080) continue;

    auto xstream_rois = std::make_shared<xstream::BaseDataVector>();
    xstream_rois->name_ = "face_box";
    auto xstream_roi =
        std::make_shared<xstream::XStreamData<hobot::vision::BBox>>();
    xstream_roi->value.x1 = x1;
    xstream_roi->value.y1 = y1;
    xstream_roi->value.x2 = x2;
    xstream_roi->value.y2 = y2;
    xstream_rois->datas_.push_back(xstream_roi);
    uint32_t effective_w, effective_h;
    fb_handle.GetImgInfo(img_path, &feed_back_info, &effective_w, &effective_h);


    xstream::InputDataPtr inputdata(new xstream::InputData());

    auto py_img = std::make_shared<hobot::vision::PymImageFrame>();
    py_img->img = feed_back_info;
    auto xstream_data = std::make_shared<xstream::XStreamData<ImageFramePtr>>();
    xstream_data->value =
        std::static_pointer_cast<hobot::vision::ImageFrame>(py_img);
    xstream_data->name_ = "pyramid";

    inputdata->datas_.push_back(
        std::static_pointer_cast<xstream::BaseData>(xstream_rois));
    inputdata->datas_.push_back(
        std::static_pointer_cast<xstream::BaseData>(xstream_data));

    auto out = flow->SyncPredict(inputdata);
    if (model_name == "pose_lmk") {
      printf("the pose_lmk is \n");
      DumpPoseLmk(out->datas_, output, img_path);
    } else if (model_name == "age_gender") {
      DumpAgeGender(out->datas_, output, img_path);
    } else if (model_name == "anti_spf") {
      DumpAntiSpf(out->datas_, output, img_path);
    } else if (model_name == "face_quality") {
      DumpFaceQuality(out->datas_, output, img_path);
    } else if (model_name == "vehicle_type") {
      DumpVehicleType(out->datas_, output, img_path);
    } else if (model_name == "vehicle_color") {
      DumpVehicleColor(out->datas_, output, img_path);
    } else if (model_name == "plate_num") {
      DumpPlateNum(out->datas_, output, img_path);
    } else if (model_name == "vehicle_phone") {
      DumpVehiclePhone(out->datas_, output, img_path);
    } else if (model_name == "vehicle_belt") {
      DumpVehicleBelt(out->datas_, output, img_path);
    } else if (model_name == "person_attrs") {
      DumpPersonAttrs(out->datas_, output, img_path);
    }
    fb_handle.FreeImgInfo(&feed_back_info);
  }
#endif
#ifdef X3
  HbVioFbWrapperGlobal fb_handle(fb_cfg);
  fb_handle.Init();
  std::ifstream img_list_file(img_list);
  std::string img_path;
  std::string gt_line;
  std::ofstream output(rlt_put_file, std::ios::out);
  float x1, y1, x2, y2;
  while (getline(img_list_file, gt_line)) {
    std::istringstream gt(gt_line);
    gt >> img_path;
    gt >> x1 >> y1 >> x2 >> y2;
    if (x1 < 0 || x2 >= 1920 || y1 < 0 || y2 >= 1080) continue;

    auto xstream_rois = std::make_shared<xstream::BaseDataVector>();
    xstream_rois->name_ = "face_box";
    auto xstream_roi =
        std::make_shared<xstream::XStreamData<hobot::vision::BBox>>();
    xstream_roi->value.x1 = x1;
    xstream_roi->value.y1 = y1;
    xstream_roi->value.x2 = x2;
    xstream_roi->value.y2 = y2;
    xstream_rois->datas_.push_back(xstream_roi);
    uint32_t effective_w, effective_h;
    auto py_img = fb_handle.GetImgInfo(img_path, &effective_w, &effective_h);

    xstream::InputDataPtr inputdata(new xstream::InputData());
    auto xstream_data = std::make_shared<xstream::XStreamData<ImageFramePtr>>();
    xstream_data->value =
        std::static_pointer_cast<hobot::vision::ImageFrame>(py_img);
    xstream_data->name_ = "pyramid";

    inputdata->datas_.push_back(
        std::static_pointer_cast<xstream::BaseData>(xstream_rois));
    inputdata->datas_.push_back(
        std::static_pointer_cast<xstream::BaseData>(xstream_data));

    auto out = flow->SyncPredict(inputdata);
    if (model_name == "pose_lmk") {
      printf("the pose_lmk is \n");
      DumpPoseLmk(out->datas_, output, img_path);
    } else if (model_name == "age_gender") {
      DumpAgeGender(out->datas_, output, img_path);
    } else if (model_name == "anti_spf") {
      DumpAntiSpf(out->datas_, output, img_path);
    } else if (model_name == "face_quality") {
      DumpFaceQuality(out->datas_, output, img_path);
    } else if (model_name == "vehicle_type") {
      DumpVehicleType(out->datas_, output, img_path);
    } else if (model_name == "vehicle_color") {
      DumpVehicleColor(out->datas_, output, img_path);
    } else if (model_name == "plate_num") {
      DumpPlateNum(out->datas_, output, img_path);
    } else if (model_name == "vehicle_phone") {
      DumpVehiclePhone(out->datas_, output, img_path);
    } else if (model_name == "vehicle_belt") {
      DumpVehicleBelt(out->datas_, output, img_path);
    } else if (model_name == "person_attrs") {
      DumpPersonAttrs(out->datas_, output, img_path);
    }
    fb_handle.FreeImgInfo(py_img);
  }
#endif
  return 0;
}

void DumpPersonAttrs(std::vector<xstream::BaseDataPtr> &result,
                     std::ostream &os, std::string id) {
  os << id;

  std::vector<float> rlts;
  auto hats = std::static_pointer_cast<xstream::BaseDataVector>(result[0]);
  auto glasseds =
      std::static_pointer_cast<xstream::BaseDataVector>(result[1]);
  auto masks = std::static_pointer_cast<xstream::BaseDataVector>(result[2]);
  auto ups = std::static_pointer_cast<xstream::BaseDataVector>(result[3]);
  auto lows = std::static_pointer_cast<xstream::BaseDataVector>(result[4]);
  auto bags = std::static_pointer_cast<xstream::BaseDataVector>(result[5]);
  int target_size = hats->datas_.size();
  for (int target_idx = 0; target_idx < target_size; target_idx++) {
    auto hat = std::static_pointer_cast<xstream::XStreamData<int>>(
        hats->datas_[target_idx]);
    auto glassed = std::static_pointer_cast<xstream::XStreamData<int>>(
        glasseds->datas_[target_idx]);
    auto mask = std::static_pointer_cast<xstream::XStreamData<int>>(
        masks->datas_[target_idx]);
    auto up = std::static_pointer_cast<xstream::XStreamData<int>>(
        ups->datas_[target_idx]);
    auto low = std::static_pointer_cast<xstream::XStreamData<int>>(
        lows->datas_[target_idx]);
    auto bag = std::static_pointer_cast<xstream::XStreamData<int>>(
        bags->datas_[target_idx]);

    if (hat->state_ != xstream::DataState::VALID) {
      os << " -1 -1 -1 -1 -1 -1";
      continue;
    }
    // printf("result %d %d\n", hat->value, glassed->value);
    printf("the person attr value is %d %d %d %d %d %d\n", hat->value,
           glassed->value, mask->value, up->value, low->value, bag->value);
    os << "hat:" << hat->value;
    os << "glassed:" << glassed->value;
    os << "mask:" << mask->value;
    os << "up:" << up->value;
    os << "low:" << low->value;
    os << "bag:" << bag->value;
  }
  os << std::endl;
}

void DumpVehicleBelt(std::vector<xstream::BaseDataPtr> &result,
                     std::ostream &os, std::string id) {
  os << id;
  std::vector<float> rlts;
  auto types = std::static_pointer_cast<xstream::BaseDataVector>(result[0]);
  int target_size = types->datas_.size();
  for (int target_idx = 0; target_idx < target_size; target_idx++) {
    auto type = std::static_pointer_cast<xstream::XStreamData<int>>(
        types->datas_[target_idx]);

    if (type->state_ == xstream::DataState::VALID) {
      rlts.push_back(type->value);
      printf("the vehicle belt value is %d\n", type->value);
    }
  }
  for (const auto &value : rlts) {
    os << " " << value;
  }
  os << std::endl;
}

void DumpVehiclePhone(std::vector<xstream::BaseDataPtr> &result,
                      std::ostream &os, std::string id) {
  os << id;
  std::vector<float> rlts;
  auto types = std::static_pointer_cast<xstream::BaseDataVector>(result[0]);
  int target_size = types->datas_.size();
  for (int target_idx = 0; target_idx < target_size; target_idx++) {
    auto type = std::static_pointer_cast<xstream::XStreamData<int>>(
        types->datas_[target_idx]);

    if (type->state_ == xstream::DataState::VALID) {
      rlts.push_back(type->value);
      printf("the vehicle phone value is %d\n", type->value);
    }
  }
  for (const auto &value : rlts) {
    os << " " << value;
  }
  os << std::endl;
}

void DumpPlateNum(std::vector<xstream::BaseDataPtr> &result, std::ostream &os,
                  std::string id) {
  os << id;
  std::vector<std::string> rlts;
  auto types = std::static_pointer_cast<xstream::BaseDataVector>(result[0]);
  int target_size = types->datas_.size();
  for (int target_idx = 0; target_idx < target_size; target_idx++) {
    auto type = std::static_pointer_cast<
                xstream::XStreamData<std::vector<int>>>(
        types->datas_[target_idx]);

    if (type->state_ == xstream::DataState::VALID) {
      std::string temp;
      for (auto &num : type->value) {
        temp += num;
      }
      rlts.push_back(temp);
      printf("the plate num value is %s\n", temp.c_str());
    } else {
      printf("the plate num is invaild\n");
    }
  }
  for (const auto &value : rlts) {
    os << " " << value;
  }
  os << std::endl;
}

void DumpVehicleColor(std::vector<xstream::BaseDataPtr> &result,
                      std::ostream &os, std::string id) {
  os << id;
  std::vector<float> rlts;
  auto types = std::static_pointer_cast<xstream::BaseDataVector>(result[0]);
  int target_size = types->datas_.size();
  for (int target_idx = 0; target_idx < target_size; target_idx++) {
    auto type = std::static_pointer_cast<xstream::XStreamData<int>>(
        types->datas_[target_idx]);

    if (type->state_ == xstream::DataState::VALID) {
      rlts.push_back(type->value);
      printf("the vehicle color value is %d\n", type->value);
    }
  }
  for (const auto &value : rlts) {
    os << " " << value;
  }
  os << std::endl;
}

void DumpVehicleType(std::vector<xstream::BaseDataPtr> &result,
                     std::ostream &os, std::string id) {
  os << id;
  std::vector<float> rlts;
  auto types = std::static_pointer_cast<xstream::BaseDataVector>(result[0]);
  int target_size = types->datas_.size();
  for (int target_idx = 0; target_idx < target_size; target_idx++) {
    auto type = std::static_pointer_cast<xstream::XStreamData<int>>(
        types->datas_[target_idx]);

    if (type->state_ == xstream::DataState::VALID) {
      rlts.push_back(type->value);
      printf("the vehicle type value is %d\n", type->value);
    }
  }
  for (const auto &value : rlts) {
    os << " " << value;
  }
  os << std::endl;
}

void DumpAntiSpf(std::vector<xstream::BaseDataPtr> &result, std::ostream &os,
                 std::string id) {
  os << id;
  std::vector<float> rlts;
  auto anti_spfs =
      std::static_pointer_cast<xstream::BaseDataVector>(result[0]);
  int target_size = anti_spfs->datas_.size();
  for (int target_idx = 0; target_idx < target_size; target_idx++) {
    auto anti_spf = std::static_pointer_cast<
        xstream::XStreamData<hobot::vision::Attribute<int>>>(
        anti_spfs->datas_[target_idx]);
    if (anti_spf->state_ == xstream::DataState::VALID) {
      rlts.push_back(anti_spf->value.score);
    }
  }
  for (const auto &value : rlts) {
    os << " " << value;
  }
  os << std::endl;
}

void DumpAgeGender(std::vector<xstream::BaseDataPtr> &result,
                   std::ostream &os,
                   std::string id) {
  os << id;
  auto ages = std::static_pointer_cast<xstream::BaseDataVector>(result[0]);
  auto genders = std::static_pointer_cast<xstream::BaseDataVector>(result[1]);
  int target_size = ages->datas_.size();
  for (int target_idx = 0; target_idx < target_size; target_idx++) {
    auto age =
        std::static_pointer_cast<xstream::XStreamData<hobot::vision::Age>>(
            ages->datas_[target_idx]);
    auto gender =
        std::static_pointer_cast<xstream::XStreamData<hobot::vision::Gender>>(
            genders->datas_[target_idx]);
    if (age->state_ != xstream::DataState::VALID) {
      os << " -1 -1 -1";
      continue;
    }
    os << " " << age->value.min << " " << age->value.max;
    os << " " << gender->value.value;
  }
  os << std::endl;
}

void DumpPoseLmk(std::vector<xstream::BaseDataPtr> &result, std::ostream &os,
                 std::string id) {
  os << id;
  auto lmks = std::static_pointer_cast<xstream::BaseDataVector>(result[0]);
  auto poses = std::static_pointer_cast<xstream::BaseDataVector>(result[1]);
  int target_size = lmks->datas_.size();
  for (int target_idx = 0; target_idx < target_size; target_idx++) {
    auto lmk = std::static_pointer_cast<xstream::XStreamData<
               hobot::vision::Landmarks>>(lmks->datas_[target_idx]);
    auto pose =
        std::static_pointer_cast<xstream::XStreamData<hobot::vision::Pose3D>>(
            poses->datas_[target_idx]);
    if (lmk->state_ != xstream::DataState::VALID) {
      continue;
    }
    for (auto &point : lmk->value.values) {
      os << " " << point.x << " " << point.y;
    }
    os << " " << pose->value.pitch << " " << pose->value.yaw << " "
       << pose->value.roll;
  }
  os << std::endl;
}
void DumpFaceQuality(std::vector<BaseDataPtr> &result,
                     std::ostream &os,
                     std::string id) {
  os << id;
  auto rois = std::static_pointer_cast<xstream::BaseDataVector>(result[0]);
  int target_size = rois->datas_.size();
  for (int target_idx = 0; target_idx < target_size; target_idx++) {
    auto roi =
        std::static_pointer_cast<xstream::XStreamData<hobot::vision::BBox>>(
            rois->datas_[target_idx]);
    os << " " << roi->value.x1 << " " << roi->value.y1 << " " << roi->value.x2
       << " " << roi->value.y2;
    for (size_t attr_idx = 1; attr_idx < result.size(); attr_idx++) {
      auto attrs =
          std::static_pointer_cast<xstream::BaseDataVector>(result[attr_idx]);
      auto attr = std::static_pointer_cast<
          xstream::XStreamData<hobot::vision::Attribute<int>>>(
          attrs->datas_[target_idx]);
      if (attr->state_ == xstream::DataState::VALID) {
        if (attr_idx == 2) {
          os << " " << attr->value.value;
        } else {
          os << " " << attr->value.score;
        }
      } else {
        os << " -1.0";
      }
    }
  }
  os << std::endl;
}
