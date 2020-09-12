/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     snapshot_data_type header
 * @author    chao.yang
 * @email     chao01.yang@horizon.ai
 * @version   0.0.1
 * @date      2019.07.25
 */

#ifndef MOTMETHOD_DATA_TYPE_MOT_DATA_TYPE_HPP_
#define MOTMETHOD_DATA_TYPE_MOT_DATA_TYPE_HPP_

#include <memory>
#include <string>
#include <vector>
#include "horizon/vision_type/vision_type.hpp"
#include "hobotxstream/method.h"
#include "MOTMethod/error_code.h"
#include "json/json.h"

namespace xstream {

#define SET_MOT_METHOD_PARAM(json_cfg, type, key)                            \
        if (json_cfg.isMember(#key) && json_cfg[#key].is##type())            \
            key = json_cfg[#key].as##type()

class MOTParam : public xstream::InputParam {
 public:
  explicit MOTParam(const std::string &content) : InputParam("MOTMethod") {
    UpdateParameter(content);
    is_enable_this_method_ = true;
    is_json_format_ = true;
    unique_name_ = "MOTMethod";
  }

  virtual int UpdateParameter(const std::string &content) {
    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    JSONCPP_STRING error;
    std::shared_ptr<Json::CharReader> json_reader(builder.newCharReader());
    try {
      bool ret = json_reader->parse(content.c_str(), content.c_str()
          + content.size(), &config_jv, &error);
      SET_MOT_METHOD_PARAM(config_jv, String, tracker_type);
      if (ret) {
        return XSTREAM_MOT_OK;
      } else {
        return XSTREAM_MOT_ERR_PARAM;
      }
    } catch (std::exception &e) {
      return XSTREAM_MOT_ERR_PARAM;
    }
  }

  std::string Format() override {
    return config_jv.toStyledString();
  }

 protected:
  std::string tracker_type = "";
  Json::Value config_jv;
};

struct IOUParam : MOTParam {
 public:
  explicit IOUParam(const std::string &content = "")
      : MOTParam(content) {
    UpdateParameter(content);
  }

  int UpdateParameter(const std::string &content) override {
    int ret = MOTParam::UpdateParameter(content);
    if (ret != XSTREAM_MOT_OK)
      return ret;
    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    JSONCPP_STRING error;
    std::shared_ptr<Json::CharReader> json_reader(builder.newCharReader());
    try {
      ret = json_reader->parse(content.c_str(), content.c_str()
          + content.size(), &config_jv, &error);
      SET_MOT_METHOD_PARAM(config_jv, Bool, update_no_target_predict);
      SET_MOT_METHOD_PARAM(config_jv, Bool, support_hungarian);
      SET_MOT_METHOD_PARAM(config_jv, Bool, need_check_merge);
      SET_MOT_METHOD_PARAM(config_jv, String, device);
      SET_MOT_METHOD_PARAM(config_jv, Bool, original_bbox);
      SET_MOT_METHOD_PARAM(config_jv, UInt, max_track_target_num);
      SET_MOT_METHOD_PARAM(config_jv, UInt, max_det_target_num);
      SET_MOT_METHOD_PARAM(config_jv, UInt, vanish_frame_count);
      SET_MOT_METHOD_PARAM(config_jv, UInt, time_gap);
      if (ret) {
        return XSTREAM_MOT_OK;
      } else {
        return XSTREAM_MOT_ERR_PARAM;
      }
    } catch (std::exception &e) {
      return XSTREAM_MOT_ERR_PARAM;
    }
  }

  bool update_no_target_predict = false;
  bool support_hungarian = false;
  bool need_check_merge = false;
  bool original_bbox = true;
  std::string device = "X2";
  uint32_t max_track_target_num = 256;
  uint32_t max_det_target_num = 256;
  uint32_t vanish_frame_count = 30;
  uint32_t time_gap = 40;
};

#ifdef ENABLE_IOU2
struct IOU2Param : MOTParam {
 public:
  explicit IOU2Param(const std::string &content = "")
      : MOTParam(content) {
    UpdateParameter(content);
  }

  int UpdateParameter(const std::string &content) override {
    int ret = MOTParam::UpdateParameter(content);
    if (ret != XSTREAM_MOT_OK)
      return ret;
    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    JSONCPP_STRING error;
    std::shared_ptr<Json::CharReader> json_reader(builder.newCharReader());
    try {
      ret = json_reader->parse(content.c_str(), content.c_str()
          + content.size(), &config_jv, &error);
      SET_MOT_METHOD_PARAM(config_jv, String, match_type);
      SET_MOT_METHOD_PARAM(config_jv, Bool, use_kalman_filter);
      SET_MOT_METHOD_PARAM(config_jv, Int, missing_time_thres);
      SET_MOT_METHOD_PARAM(config_jv, UInt, vanish_frame_count);
      SET_MOT_METHOD_PARAM(config_jv, UInt, time_gap);
      SET_MOT_METHOD_PARAM(config_jv, Int, remove_obsolete_track);
      SET_MOT_METHOD_PARAM(config_jv, Double, iou_thres);
      SET_MOT_METHOD_PARAM(config_jv, Double, euclidean_thres);
      SET_MOT_METHOD_PARAM(config_jv, Int, use_location_gain);
      SET_MOT_METHOD_PARAM(config_jv, Int, max_trajectory_number);

      SET_MOT_METHOD_PARAM(config_jv, Double, min_score);
      SET_MOT_METHOD_PARAM(config_jv, Double, ignore_overlap_thres);
      if (ret) {
        return XSTREAM_MOT_OK;
      } else {
        return XSTREAM_MOT_ERR_PARAM;
      }
    } catch (std::exception &e) {
      return XSTREAM_MOT_ERR_PARAM;
    }
  }

  std::string match_type = "Euclidean";
  bool use_kalman_filter = false;
  int missing_time_thres = 2;
  uint32_t vanish_frame_count = 50;
  uint32_t time_gap = 40;
  int remove_obsolete_track = 0;
  float iou_thres = 0.2f;
  float euclidean_thres = 200.0f;
  int use_location_gain = 1;
  int max_trajectory_number = 3;


  float min_score = 0.9f;
  float ignore_overlap_thres = 0.9f;
};
#endif  // ENABLE_IOU2


}  // namespace xstream

#endif  // MOTMETHOD_DATA_TYPE_MOT_DATA_TYPE_HPP_
