/*
 * @Description: data type and utils
 * @Author: chao.yang@horizon.ai
 * @Date: 2019-5-27 10:30:32
 * @LastEditors  : hao.tian@horizon.ai
 * @LastEditTime : 2020-01-17 13:27:34
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */

#ifndef FilterMethod_FILTER_DATA_TYPE_HPP_
#define FilterMethod_FILTER_DATA_TYPE_HPP_

#include <algorithm>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>

#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/method.h"
#include "horizon/vision_type/vision_type.hpp"
#include "json/json.h"

namespace xstream {
typedef std::shared_ptr<BaseDataVector> BaseDataVectorPtr;
typedef XStreamData<hobot::vision::ImageFrame> XStreamImageFrame;
typedef XStreamData<uint32_t> XStreamID;
typedef XStreamData<hobot::vision::Age> XStreamAge;
typedef XStreamData<hobot::vision::BBox> XStreamBBox;
typedef XStreamData<int32_t> XStreamFilterDescription;
typedef XStreamData<hobot::vision::Pose3D> XStreamPose3D;
typedef XStreamData<hobot::vision::Landmarks> XStreamLandmarks;
//  same below
typedef XStreamData<hobot::vision::Quality> XStreamQuality;
typedef XStreamData<hobot::vision::Gender> XStreamGender;
typedef XStreamData<hobot::vision::Attribute<int32_t>> XStreamAttribute;

#define SET_FILTER_ERROR_CODE(err_desp, key)               \
  if (err_desp.isMember(#key) && err_desp[#key].isInt()) { \
    key##_err_code = err_desp[#key].asInt();               \
    LOGI << #key << "_err_code: " << key##_err_code;       \
  }

#define SET_FILTER_METHOD_PARAM(json_cfg, type, key)          \
  if (json_cfg.isMember(#key) && json_cfg[#key].is##type()) { \
    key = json_cfg[#key].as##type();                          \
    config_jv_all_[#key] = key;                               \
    LOGI << #key << ": " << key;                              \
  }

enum class NormMethod {
  BPU_MODEL_NORM_BY_WIDTH_LENGTH,
  BPU_MODEL_NORM_BY_WIDTH_RATIO,
  BPU_MODEL_NORM_BY_HEIGHT_RATIO,
  BPU_MODEL_NORM_BY_LSIDE_RATIO,
  BPU_MODEL_NORM_BY_HEIGHT_LENGTH,
  BPU_MODEL_NORM_BY_LSIDE_LENGTH,
  BPU_MODEL_NORM_BY_LSIDE_SQUARE,
  BPU_MODEL_NORM_BY_DIAGONAL_SQUARE,
  BPU_MODEL_NORM_BY_NOTHING
};

struct Area : hobot::vision::BBox {
  float iou_threshold_ = 0.f;
};

class ListsModule {
 public:
  std::list<Area> &GetAreaList() { return area_list_; }

  void ClearAreaList() { area_list_.clear(); }

  void AddList(Area area) { area_list_.emplace_back(area); }

  void AddList(float x1, float y1, float x2, float y2, float threshold = 0.5) {
    Area area_;
    area_.x1 = x1;
    area_.y1 = y1;
    area_.x2 = x2;
    area_.y2 = y2;
    area_.iou_threshold_ = threshold;
    area_list_.emplace_back(area_);
  }

  virtual bool IsInArea(float x1, float y1, float x2, float y2,
                        const Area &area) = 0;

 protected:
  std::list<Area> area_list_;
};

class BlackListsModule : public ListsModule {
 public:
  bool IsInArea(float x1, float y1, float x2, float y2,
                const Area &black_area) override {
    float black_area_x1 = black_area.x1;
    float black_area_y1 = black_area.y1;
    float black_area_x2 = black_area.x2;
    float black_area_y2 = black_area.y2;

    float l_inter = std::max(x1, black_area_x1);
    float r_inter = std::min(x2, black_area_x2);
    if (l_inter >= r_inter) {
      return false;
    }
    float t_inter = std::max(y1, black_area_y1);
    float b_inter = std::min(y2, black_area_y2);
    if (t_inter >= b_inter) {
      return false;
    }
    float w_inter = r_inter - l_inter;
    float h_inter = b_inter - t_inter;
    float area_inter = w_inter * h_inter;
    float area_box1 = (x2 - x1) * (y2 - y1);
    float iou = area_inter / area_box1;
    return iou >= black_area.iou_threshold_;
  }
};

class WhiteListsModule : public ListsModule {
 public:
  bool IsInArea(float x1, float y1, float x2, float y2,
                const Area &white_area) override {
    float c_x = (x1 + x2) / 2;
    float c_y = (y1 + y2) / 2;
    if ((c_x >= white_area.x1) && (c_x <= white_area.x2) &&
        (c_y >= white_area.y1) && (c_y <= white_area.y2)) {
      return true;
    } else {
      return false;
    }
  }
};

struct SlotInfo {
  std::string name;
  std::string type;
  std::string group;
};

struct FilterParam : public xstream::InputParam {
  explicit FilterParam(const std::string &content = "")
      : InputParam("FilterMethod") {
    is_enable_this_method_ = true;
    is_json_format_ = true;
    stop_ids_.clear();
    UpdateParameter(content);
  }
  virtual int UpdateParameter(const std::string &content);

  void SetOccludeErrCode();

  void SetFilterErrCode();

  void SetOccludeParam();

  void SetFilterParam();

  void SetNormParam();

  void SetBlackArea();

  void SetWhiteArea();

  void SetStopID();

  void BuildInputSlotMap();

  void BuildFilterState();

  //  default params
  bool show_bound_rect = false;  // show bound rect on web
  int image_width = 1920;   // the width of image frame
  int image_height = 1080;  // the height of image frame
  int face_size_thr = 32;   // the threshold of snap size
  int head_size_thr = 32;   // the threshold of snap size
  int body_size_thr = 32;   // the threshold of snap size
  int hand_size_thr = 32;   // the threshold of snap size
  float face_pv_thr = 0.5;  // the threshold of snap face rect score
  float head_pv_thr = 0.5;  // the threshold of snap head rect score
  float body_pv_thr = 0.5;  // the threshold of snap body rect score
  float hand_pv_thr = 0.5;  // the threshold of snap hand rect score
  float face_expand_scale = 1.0;
  float head_expand_scale = 1.0;
  float body_expand_scale = 1.0;
  float hand_expand_scale = 1.0;
  float frontal_pitch_thr = 0;  // the threshold of pitch value in frontal area
  float frontal_yaw_thr = 0;    // the threshold of yaw value in frontal area
  float frontal_roll_thr = 0;   // the threshold of roll value in frontal area
  float frontal_thr = 0;  // the threshold of overall value in frontal area
  bool filter_with_frontal_thr = false;  // the switch of frontal filter type
  float quality_thr = 0.0;               // the threshold of snap quality
  float lmk_thr = 0;       // the threshold of snap landmarks score
  int lmk_filter_num = 0;  // the num of snap landmarks filter
  int bound_thr_w = 0;     // the boundary of crop area
  int bound_thr_h = 0;
  BlackListsModule
      black_list_module;  // we will filter the snaps in the blacklist area
  WhiteListsModule
      white_list_module;        // we will allow the snaps in the whitelist area
  int max_face_box_counts = 0;  // the max count of boxes
  int age_min = 0;
  int age_max = 100;
  float age_thr = 0;
  float brightness_min = 0;
  float brightness_max = 4;
  // occluded condition: val is larger than the thr
  // smaller value means better quality
  float left_eye_occluded_thr = 0.5;
  float right_eye_occluded_thr = 0.5;
  float left_brow_occluded_thr = 0.5;
  float right_brow_occluded_thr = 0.5;
  float forehead_occluded_thr = 0.5;
  float left_cheek_occluded_thr = 0.5;
  float right_cheek_occluded_thr = 0.5;
  float nose_occluded_thr = 0.5;
  float mouth_occluded_thr = 0.5;
  float jaw_occluded_thr = 0.5;
  // abnormal condition: val is larger than the thr
  // smaller value means better abnormalities
  float abnormal_thr = 1.0;
  // default error code
  int stop_id_err_code = 1;
  int age_err_code = -1;
  int passed_err_code = -1;
  int snap_area_err_code = -1;
  int snap_size_thr_err_code = -1;
  int frontal_thr_err_code = -1;
  int quality_thr_err_code = -1;
  int lmk_thr_err_code = -1;
  int pv_thr_err_code = -1;
  int black_list_err_code = -1;
  int white_list_err_code = -1;
  int big_face_err_code = -1;
  int brightness_err_code = -1;
  int abnormal_thr_err_code = -1;
  int expand_thr_err_code = -1;
  int left_eye_occluded_thr_err_code = -1;
  int right_eye_occluded_thr_err_code = -1;
  int left_brow_occluded_thr_err_code = -1;
  int right_brow_occluded_thr_err_code = -1;
  int forehead_occluded_thr_err_code = -1;
  int left_cheek_occluded_thr_err_code = -1;
  int right_cheek_occluded_thr_err_code = -1;
  int nose_occluded_thr_err_code = -1;
  int mouth_occluded_thr_err_code = -1;
  int jaw_occluded_thr_err_code = -1;
  // default norm method
  NormMethod e_norm_method = NormMethod::BPU_MODEL_NORM_BY_NOTHING;
  static const std::map<std::string, NormMethod> norm_method_map_;
  // default stop output id
  int stop_id = -1;
  int start_id = -1;
  std::set<int> stop_ids_;
  std::string merge_method = "head_body";
  bool head_enable = true;
  bool face_enable = true;
  bool body_enable = true;
  bool hand_enable = true;
  int enable_flag = 0;
  // 0:valid 1:filter 2:invisible 3:disappeared 4:invalid
  int filter_status = 1;
  std::map<uint32_t, SlotInfo> slot_info_;
  Json::Value config_jv_all_;  // contain all params
  Json::Value config_jv;       // contain param updated
  std::string Format() override;
};

typedef std::shared_ptr<FilterParam> FilterParamPtr;

}  // namespace xstream

#endif  // FilterMethod_FILTER_DATA_TYPE_HPP_
