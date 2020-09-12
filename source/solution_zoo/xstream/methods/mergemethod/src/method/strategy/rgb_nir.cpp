/*
 * @Description: implement of data_type
 * @Author: yutong.pan@horizon.ai
 * @Date: 2019-11-26 17:49:26
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */
#ifdef RGB_NIR_MERGE
#include "MergeMethod/strategy/rgb_nir.h"

#include <fstream>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>
#ifdef EEPROM_ENABLED
#include "./eeprom_layout.h"
#include "./x2_camera.h"
#include "./x2_camera_common.h"
#else
#include "data_type/eeprom_header.h"
#endif

namespace xstream {

int RGBNIRParam::UpdateParameter(const JsonReaderPtr &reader) {
  LOGD << "RGBNIRParam update config: " << this;
  auto ret = MergeParam::UpdateParameter(reader);
  SET_SNAPSHOT_METHOD_PARAM(reader->GetRawJson(), Int, camera_type);
  return ret;
}

std::vector<BaseDataPtr> RGBNIRStrategy::ProcessFrame(
    const std::vector<BaseDataPtr> &in, const InputParamPtr &param) {
  HOBOT_CHECK(!in.empty());
  std::vector<BaseDataPtr> out;
  if (param) {
    std::string content = param->Format();
    if (content == "pass-through") {
      LOGD << "pass through mode";
      PassThroughSingleFrame(in, &out);
    } else {
      HOBOT_CHECK(param->is_json_format_) << "json format error";
      LOGD << "normal mode";
      Json::CharReaderBuilder builder;
      builder["collectComments"] = false;
      JSONCPP_STRING error;
      std::shared_ptr<Json::CharReader> json_reader(builder.newCharReader());
      Json::Value out_jv;
      auto ret = json_reader->parse(
          content.c_str(), content.c_str() + content.size(), &out_jv, &error);
      auto reader = std::make_shared<JsonReader>(out_jv);
      ret = UpdateParameter(reader);
      HOBOT_CHECK(ret == kHorizonVisionSuccess) << "param error";
      RunSingleRGBFrame(in, &out);
    }
  } else {
    RunSingleRGBFrame(in, &out);
  }
  LOGI << "RGB NIR Mode, out size: " << out.size();
  return out;
}

void RGBNIRStrategy::Finalize() { LOGI << "RGBNIRStrategy Finalize"; }

int RGBNIRStrategy::UpdateParameter(const JsonReaderPtr &reader) {
  return (std::static_pointer_cast<RGBNIRParam>(merge_config_param_))
      ->UpdateParameter(reader);
}

std::shared_ptr<RGBNIRParam> RGBNIRStrategy::GetConfig() {
  auto config_ = std::static_pointer_cast<RGBNIRParam>(merge_config_param_);
  return config_;
}

int RGBNIRStrategy::Init(std::shared_ptr<MergeParam> config) {
  merge_config_param_ = config;

  calib_eeprom_t *read_eeprom_handle = new calib_eeprom_t;

  read_eeprom_handle->wr_mode = EEPROM_RD_MODE;
  read_eeprom_handle->addr = CAM_CALIB_START_ADDR;
  read_eeprom_handle->length = CAM_CALIB_DATA_LENGTH;

  read_eeprom_handle->buff = calib_buff_;

  if (hb_cam_process(CAMERA_EVENT_EEPROM_CALIBRATION, read_eeprom_handle) < 0) {
    std::string matrix_path = "/userdata/fundamental.yaml";
    std::ifstream ifs(matrix_path);
    if (ifs.good()) {
      cv::FileStorage fs(matrix_path, cv::FileStorage::READ);
      transfer_F_.create(3, 3, cv::DataType<double>::type);
      transfer_F_ = fs["F"].mat();
      LOGI << "Read File Mat: " << transfer_F_;
    } else {
      LOGW << "NO EEPROM & No camera calibration file (fundamental.yaml) exist."
              "Please do camera calibration first";
      delete read_eeprom_handle;
      read_eeprom_handle = nullptr;
      return kHorizonVisionFailure;
    }
  } else {
    cv::Mat eeprom_mat(3, 3, cv::DataType<double>::type,
                       read_eeprom_handle->buff);
    transfer_F_ = eeprom_mat;
    LOGI << "Read EEPROM Mat: " << transfer_F_;
  }
  delete read_eeprom_handle;

  return kHorizonVisionSuccess;
}

void RGBNIRStrategy::RunSingleRGBFrame(
    const std::vector<BaseDataPtr> &frame_input,
    std::vector<BaseDataPtr> *frame_output) {
  HOBOT_CHECK_GE(frame_input.size(), 4);
  const auto &face_box_list = frame_input[0];
  const auto &disappeared_face_id = frame_input[1];
  const auto &head_box_list = frame_input[2];
  const auto &disappeared_head_id = frame_input[3];
  HOBOT_CHECK_EQ(face_box_list->type_, "BaseDataVector");
  HOBOT_CHECK_EQ(disappeared_face_id->type_, "BaseDataVector");
  HOBOT_CHECK_EQ(head_box_list->type_, "BaseDataVector");
  HOBOT_CHECK_EQ(disappeared_head_id->type_, "BaseDataVector");
  // landmarks for Camera correction
  BaseDataPtr rgb_landmarks = nullptr;
  BaseDataPtr nir_landmarks = nullptr;
  if (frame_input.size() >= 6) {
    rgb_landmarks = frame_input[4];
    nir_landmarks = frame_input[5];
    HOBOT_CHECK_EQ(rgb_landmarks->type_, "BaseDataVector");
    HOBOT_CHECK_EQ(nir_landmarks->type_, "BaseDataVector");
  }

  BaseDataVectorPtr face_box_res = std::make_shared<BaseDataVector>();
  BaseDataVectorPtr head_box_res = std::make_shared<BaseDataVector>();
  BaseDataVectorPtr disappeared_track_id = std::make_shared<BaseDataVector>();

  RemoveDisappearedTrack(disappeared_face_id, disappeared_head_id,
                         disappeared_track_id);

  ProduceOutput(face_box_list, head_box_list, face_box_res, head_box_res,
                rgb_landmarks, nir_landmarks);

  frame_output->emplace_back(BaseDataPtr(face_box_res));
  frame_output->emplace_back(BaseDataPtr(head_box_res));
  frame_output->emplace_back(BaseDataPtr(disappeared_track_id));
  LOGD << "Process RGBNIRStrategy output";
}

std::vector<std::pair<int, int>> RGBNIRStrategy::GetMatchedPairs(
    const BaseDataPtr &face_bbox, const BaseDataPtr &head_bbox,
    const std::vector<int> &face_pts) {
  auto face_box_list = std::static_pointer_cast<BaseDataVector>(face_bbox);
  auto head_box_list = std::static_pointer_cast<BaseDataVector>(head_bbox);
  std::vector<int> face_box_ids;
  std::vector<int> head_box_ids;
  for (const auto &data : face_box_list->datas_) {
    auto bbox = std::static_pointer_cast<XStreamBBox>(data);
    face_box_ids.emplace_back(bbox->value.id);
  }
  for (const auto &data : head_box_list->datas_) {
    auto bbox = std::static_pointer_cast<XStreamBBox>(data);
    head_box_ids.emplace_back(bbox->value.id);
  }
  std::vector<std::pair<int, int>> id_pairs;
  for (unsigned int i = 0; i < face_pts.size(); ++i) {
    if (face_pts[i] != -1) {
      id_pairs.emplace_back(
          std::make_pair(face_box_ids[i], head_box_ids[face_pts[i]]));
    }
  }
  return id_pairs;
}

void RGBNIRStrategy::MergeRGBNIRTrackID(const BaseDataPtr &face_bbox,
                                        const BaseDataPtr &head_bbox,
                                        const BaseDataPtr &face_lmk,
                                        const BaseDataPtr &head_lmk,
                                        const std::set<int> &face_id_set,
                                        const std::set<int> &head_id_set,
                                        IDRelationInfo *id_merge_info) {
  HOBOT_CHECK(face_lmk);
  HOBOT_CHECK(head_lmk);
  auto face_lmk_list = std::static_pointer_cast<BaseDataVector>(face_lmk);
  auto head_lmk_list = std::static_pointer_cast<BaseDataVector>(head_lmk);
  if (face_lmk_list->datas_.empty() || head_lmk_list->datas_.empty() ||
      transfer_F_.empty()) {
    LOGD << "No rgb or nir landmarks detected";
  } else {
    std::vector<hobot::vision::Landmarks> face_pts;
    std::vector<hobot::vision::Landmarks> head_pts;
    for (unsigned int i = 0; i < face_lmk_list->datas_.size(); ++i) {
      const auto &face_data = face_lmk_list->datas_[i];
      std::shared_ptr<XStreamLandmarks> face_landmarks;
      if (face_data->state_ == xstream::DataState::VALID) {
        face_landmarks = std::static_pointer_cast<XStreamLandmarks>(face_data);
        face_pts.emplace_back(face_landmarks->value);
      } else {
        LOGE << "rgb landmarks " << i << " is not valid!!!";
      }
    }
    for (unsigned int i = 0; i < head_lmk_list->datas_.size(); ++i) {
      const auto &head_data = head_lmk_list->datas_[i];
      std::shared_ptr<XStreamLandmarks> head_landmarks;
      if (head_data->state_ == xstream::DataState::VALID) {
        head_landmarks = std::static_pointer_cast<XStreamLandmarks>(head_data);
        head_pts.emplace_back(head_landmarks->value);
      } else {
        LOGE << "nir landmarks " << i << " is not valid!!!";
      }
    }
    const float max_match_dist = 100.0;
    std::vector<int> face_pts_match_idx(face_pts.size());
    std::vector<int> head_pts_match_idx(head_pts.size());
    hobot::dual_camera_algo::DualCamTransformer dual_cam_transformer(
        transfer_F_);
    auto config = GetConfig();
    if (config->camera_type == 0) {
      dual_cam_transformer.MatchPointGroups(
          face_pts, head_pts, &face_pts_match_idx, &head_pts_match_idx,
          max_match_dist);
    } else {
      dual_cam_transformer.MatchPointGroups(
          face_pts, head_pts, &face_pts_match_idx, &head_pts_match_idx,
          max_match_dist, hobot::dual_camera_algo::Placement::Vertical);
    }

    auto id_pairs = GetMatchedPairs(face_bbox, head_bbox, face_pts_match_idx);

    for (const auto &id_pair : id_pairs) {
      auto face_mot_id = id_pair.first;
      auto head_mot_id = id_pair.second;
      // if head_track_i and face_track_i not in
      auto face_iter = id_merge_info->GetFaceMap().find(id_pair.first);
      auto head_iter = id_merge_info->GetHeadMap().find(id_pair.second);
      if (face_iter == id_merge_info->GetFaceMap().end() &&
          head_iter == id_merge_info->GetHeadMap().end()) {  // no track history
        id_merge_info->AddFaceHead(face_mot_id, head_mot_id);
      } else if (face_iter == id_merge_info->GetFaceMap().end()) {
        // head_track_i in, so set the main_id of face_track_i with main_id of
        // head_track_i
        const int main_id = head_iter->second;
        int found_track = FindConflictTrackIdFromCurrentFrame(
            id_merge_info->GetFaceMap(), face_id_set, main_id);
        if (found_track != -1) {
          //  exist current main id, change head and face id
          id_merge_info->RemoveTrack(Type::Head, head_mot_id);
          id_merge_info->AddFaceHead(face_mot_id, head_mot_id);
        } else {
          // check if there exist repeated track
          int repeated_track = FindConflictTrackIDFromAllTrack(
              id_merge_info->GetFaceMap(), main_id);
          if (repeated_track != -1) {
            id_merge_info->RemoveTrack(Type::Face, repeated_track);
          }
          id_merge_info->AddNewTrack(Type::Face, face_mot_id, main_id);
        }
      } else if (head_iter == id_merge_info->GetHeadMap().end()) {
        // face_track_i in, so set the main_id of face_track_i with main_id of
        // face_track_i
        const int main_id = face_iter->second;
        int found_track = FindConflictTrackIdFromCurrentFrame(
            id_merge_info->GetHeadMap(), head_id_set, main_id);
        if (found_track != -1) {
          //  exist current main id, change head and face id
          id_merge_info->RemoveTrack(Type::Face, face_mot_id);
          id_merge_info->AddFaceHead(face_mot_id, head_mot_id);
        } else {
          // check if there exist repeated track
          int repeated_track = FindConflictTrackIDFromAllTrack(
              id_merge_info->GetHeadMap(), main_id);
          if (repeated_track != -1) {
            id_merge_info->RemoveTrack(Type::Head, repeated_track);
          }
          id_merge_info->AddNewTrack(Type::Head, head_mot_id, main_id);
        }
      } else {
        // check if main_ids of head_track_i and face_track_i are same
        if (face_iter->second != head_iter->second) {
          // std::cout << " history no mapped" << std::endl;
          id_merge_info->RemoveTrack(Type::Head, head_mot_id);
          id_merge_info->AddNewTrack(Type::Head, head_mot_id,
                                     face_iter->second);
        } else {
          // use original main id
        }
      }
    }
  }
  // find unmatched id
  auto box_list = std::static_pointer_cast<BaseDataVector>(face_bbox);
  for (const auto &data : box_list->datas_) {
    auto box = std::static_pointer_cast<XStreamBBox>(data);
    if (box->state_ == xstream::DataState::VALID) {
      auto &track_id = box->value.id;
      auto iter = id_merge_info->GetFaceMap().find(track_id);
      if (iter == id_merge_info->GetFaceMap().end()) {
        id_merge_info->AddNewTrack(Type::Face, track_id);
      }
    }
  }

  box_list = std::static_pointer_cast<BaseDataVector>(head_bbox);
  for (const auto &data : box_list->datas_) {
    auto box = std::static_pointer_cast<XStreamBBox>(data);
    if (box->state_ == xstream::DataState::VALID) {
      auto &track_id = box->value.id;
      auto iter = id_merge_info->GetHeadMap().find(track_id);
      if (iter == id_merge_info->GetHeadMap().end()) {
        id_merge_info->AddNewTrack(Type::Head, track_id);
      }
    }
  }
}

void RGBNIRStrategy::ProduceOutput(const BaseDataPtr &facePtr,
                                   const BaseDataPtr &headPtr,
                                   const BaseDataVectorPtr &faceResPtr,
                                   const BaseDataVectorPtr &headResPtr,
                                   const BaseDataPtr &rgb_landmarks,
                                   const BaseDataPtr &nir_landmarks) {
  auto cur_face_id_set = GetIdSet(facePtr);
  auto cur_head_id_set = GetIdSet(headPtr);
  MergeRGBNIRTrackID(facePtr, headPtr, rgb_landmarks, nir_landmarks,
                     cur_face_id_set, cur_head_id_set, &id_relation_info_);
  GetMainId(facePtr, id_relation_info_.GetFaceMap(), faceResPtr);
  GetMainId(headPtr, id_relation_info_.GetHeadMap(), headResPtr);
}
}  //  namespace xstream
#endif  //  RGB_NIR_MERGE
