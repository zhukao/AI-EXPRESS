/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     ReID_MOT Implementation
 * @author    chao.yang
 * @email     chao01.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.18
 */

#include "MOTMethod/MOT/ReID.h"
#include "hobotxstream/data_types/number.h"
#include "hobotxstream/data_types/bbox.h"
#include "hobotxstream/data_types/tracklet.h"
#include "hobotxstream/data_types/landmark.h"
#include "hobotxstream/data_types/orientation.h"
#include "MOTMethod/error_code.h"


namespace xstream {

int ReID::MotInit(const std::string &config_file_path) {
  if (!config_file_path.empty()) {
    mul_obj_tracker_ = std::make_shared<MulObjTracker>(config_file_path);
    return XSTREAM_MOT_OK;
  } else {
    return XSTREAM_MOT_ERR_PARAM;
  }
}

static Orientation TransOrientation(hbot::hobot_track::Orientation ori) {
  switch (ori) {
    case hbot::hobot_track::Orientation::Front:
      return xstream::Orientation::Front;
    case hbot::hobot_track::Orientation::Back:
      return xstream::Orientation::Back;
    case hbot::hobot_track::Orientation::LeftSide:
      return xstream::Orientation::LeftSide;
    case hbot::hobot_track::Orientation::RightSide:
      return xstream::Orientation::RightSide;
    default:
      return xstream::Orientation::Unknown;
  }
}

cv::Mat ReID::convertImageFrame2Mat(const ImageFrame *frame) {
  cv::Mat rgbImg;
  switch (frame->pixel_format_) {
    case PixelFormat::RAW_RGB: {
      rgbImg = cv::Mat(
          frame->height_, frame->width_, CV_8UC3, frame->data_);
      break;
    }
    case PixelFormat::RAW_BGR: {
      auto bgrImg = cv::Mat(
          frame->height_, frame->width_, CV_8UC3, frame->data_);
      cv::cvtColor(bgrImg, rgbImg, CV_BGR2RGB);
      break;
    }
    case PixelFormat::RAW_GRAY: {
      auto yuvImg = cv::Mat(
          frame->height_, frame->width_, CV_8UC1, frame->data_);
      cv::cvtColor(yuvImg, rgbImg, CV_GRAY2RGB);
      break;
    }
    case PixelFormat::RAW_YUV_NV21: {
      auto yuvImg = cv::Mat(
          frame->height_ * 3 / 2, frame->width_, CV_8UC1, frame->data_);
      cv::cvtColor(yuvImg, rgbImg, CV_YUV2RGB_NV21);
      break;
    }
    case PixelFormat::RAW_YUV_NV12: {
      auto yuvImg = cv::Mat(
          frame->height_ * 3 / 2, frame->width_, CV_8UC1, frame->data_);
      cv::cvtColor(yuvImg, rgbImg, CV_YUV2RGB_NV12);
      break;
    }
    case PixelFormat::RAW_YUV_I420: {
      auto yuvImg = cv::Mat(
          frame->height_ * 3 / 2, frame->width_, CV_8UC1, frame->data_);
      cv::cvtColor(yuvImg, rgbImg, CV_YUV2RGB_I420);
      break;
    }
    case PixelFormat::IMAGE_CONTAINER:break;
    default:break;
  }
  return rgbImg;
}

int ReID::track(const std::vector<BaseDataPtr> &in,
                std::vector<BaseDataPtr> &out) {
  auto out_ids = std::make_shared<BaseDataVector>();
  auto out_rects = std::make_shared<BaseDataVector>();
  auto out_disappeared_idxs = std::make_shared<BaseDataVector>();
  auto out_tracklets = std::make_shared<BaseDataVector>();

  out.push_back(std::static_pointer_cast<BaseData>(out_ids));
  out.push_back(std::static_pointer_cast<BaseData>(out_rects));
  out.push_back(std::static_pointer_cast<BaseData>(out_disappeared_idxs));
  out.push_back(std::static_pointer_cast<BaseData>(out_tracklets));

  cv::Mat src;
  uint64_t time_stamp, frame_count;
  std::list<hbot::hobot_track::BBox> face_det_list;
  std::list<hbot::hobot_track::BBox> rect_list;
  std::vector<bool> rect_status;
  std::vector<hobot::vision::HumanSkeleton> skeleton_list;
  std::vector<std::vector<float>> reid_list;

  HOBOT_CHECK_EQ(
      XStreamData2VisionType(in, time_stamp, frame_count, src, face_det_list,
      rect_list, rect_status, skeleton_list, reid_list), XSTREAM_MOT_OK);

  LOGI << "src.rows:" << src.rows
       << " src.clos:" << src.cols
       << " src.channels:" << src.channels()
       << " rect_list size:" << rect_list.size()
       << " skeleton_list size:" << skeleton_list.size()
       << " time_stamp:" << time_stamp
       << " reid_list item size:" << reid_list.size();

  if (!reid_list.empty()) {
    LOGI << " reid_list feature size:" << reid_list[0].size();
  }

  int ret = mul_obj_tracker_->TrackPro(src, src.rows, src.cols, src.channels(),
      rect_list, skeleton_list, static_cast<int64>(time_stamp), reid_list);

  if (!ret) {
    return XSTREAM_MOT_ERR_PARAM;
  }

  int ret1 = GetTrackletsResult(out_tracklets);
  LOGI << "out_tracklets size:" << out_tracklets->datas_.size();

  int ret2 = GetIdBBoxResult(out_ids, out_rects);
  LOGI << " out_ids size:" << out_ids->datas_.size();
  LOGI << " out_rects size:" << out_rects->datas_.size();

  if (ret1 == XSTREAM_MOT_OK && ret2 == XSTREAM_MOT_OK) {
    return UpdateState(frame_count, out_disappeared_idxs);
  } else {
    return XSTREAM_MOT_ERR_PARAM;
  }
}

int ReID::XStreamData2VisionType(const std::vector<BaseDataPtr> &in,
                              uint64_t &timestamp,
                              uint64_t &frame_count,
                              cv::Mat &src,
                              std::list<hbot::hobot_track::BBox> &face_det_list,
                              std::list<hbot::hobot_track::BBox> &rect_list,
                              std::vector<bool> &rect_status,
                              std::vector<hobot::vision::HumanSkeleton> \
                                &skeleton_list,
                              std::vector<std::vector<float>> &reid_list) {
  HOBOT_CHECK(!in.empty());
  auto img_frame = std::static_pointer_cast<BaseData>(in[0]);
  auto face_rects = std::static_pointer_cast<BaseDataVector>(in[1]);
  auto body_rects = std::static_pointer_cast<BaseDataVector>(in[2]);
  auto reid_features = std::static_pointer_cast<BaseDataVector>(in[3]);
  auto skeletons = std::static_pointer_cast<BaseDataVector>(in[4]);

  HOBOT_CHECK("ImageFrame" == img_frame->type_);
  HOBOT_CHECK("BaseDataVector" == face_rects->type_);
  HOBOT_CHECK("BaseDataVector" == body_rects->type_);
  HOBOT_CHECK("BaseDataVector" == reid_features->type_);
  HOBOT_CHECK("BaseDataVector" == skeletons->type_);

  auto frame = std::static_pointer_cast<xstream::ImageFrame>(img_frame);
  src = convertImageFrame2Mat(frame.get());

  timestamp = frame->time_stamp_;
  frame_count = frame->frame_id_;

#ifdef CHECK_INPUT_IMAGE
  cv::imwrite("input_"+ std::to_string(timestamp) + ".jpg", src);
#endif

  for (auto &item : face_rects->datas_) {
    assert("BBox" == item->type_);
    auto bbox = std::static_pointer_cast<xstream::BBox>(item);
    assert(bbox->values_.size() >= 4);
    face_det_list.emplace_back(bbox->values_[0], bbox->values_[1],
                               bbox->values_[2],
                               bbox->values_[3], bbox->score_, bbox->class_);
  }

  for (auto &item : body_rects->datas_) {
    assert("BBox" == item->type_);
    auto bbox = std::static_pointer_cast<xstream::BBox>(item);
    assert(bbox->values_.size() >= 4);
    rect_list.emplace_back(bbox->values_[0], bbox->values_[1], bbox->values_[2],
                           bbox->values_[3], bbox->score_, bbox->class_);
  }

  for (auto &item : reid_features->datas_) {
    assert("Feature" == item->type_);
    auto features = std::static_pointer_cast<xstream::Feature>(item);
    std::vector<float> feature(features->values_.size());
    for (int i = 0; i < features->values_.size(); i++) {
      feature[i] = features->values_[i];
    }
    reid_list.push_back(feature);
  }

  for (auto &item : skeletons->datas_) {
    assert("Landmark" == item->type_);
    auto skeleton = std::static_pointer_cast<xstream::Landmark>(item);
    hobot::vision::HumanSkeleton human_skeleton;
    human_skeleton.points.resize(skeleton->points_.size());
    human_skeleton.scores.resize(skeleton->points_.size());
    human_skeleton.point_num = static_cast<int>(skeleton->points_.size());
    for (int i = 0; i < skeleton->points_.size(); i++) {
      assert(skeleton->points_[i].values_.size() == 2);
      human_skeleton.points[i].x = skeleton->points_[i].values_[0];
      human_skeleton.points[i].y = skeleton->points_[i].values_[1];
      human_skeleton.scores[i] = skeleton->points_[i].score_;
    }
    skeleton_list.push_back(human_skeleton);
  }
  return XSTREAM_MOT_OK;
}

int ReID::UpdateState(const uint64_t &frame_id,
                      const BaseDataVectorPtr &disappeared_idxs_msg) {
  auto tracklet_list = mul_obj_tracker_->GetTrackLets();
  for (auto &i : tracklet_list) {
    auto track_id = static_cast<uint64_t>(i->tracklet_id_);
    if (mot_states_.find(track_id) == mot_states_.end()) {
      StatePtr state(new State());
      state->start_ = frame_id;
      state->last_ = frame_id;
      state->count_ = 1;
      mot_states_[track_id] = state;
    } else {
      mot_states_[track_id]->last_ = frame_id;
      mot_states_[track_id]->count_++;
    }
  }

  // remove the expired track and generate disappeared id list
  auto iter = mot_states_.begin();
  while (iter != mot_states_.end()) {
    auto &state = iter->second;
    if (frame_id > state->last_) {
      std::shared_ptr<Number> track_id(new Number());
      track_id->value_ = iter->first;
      track_id->state_ = DataState::DISAPPEARED;
      disappeared_idxs_msg->datas_.push_back(track_id);
      iter = mot_states_.erase(iter);
      continue;
    } else {
      iter++;
    }
  }
  return XSTREAM_MOT_OK;
}

int ReID::GetIdBBoxResult(const BaseDataVectorPtr &ids,
                          const BaseDataVectorPtr &bboxs) {
  if (!mul_obj_tracker_->is_ready_to_output()) {
    return XSTREAM_MOT_ERR_PARAM;
  }
  auto tracklet_list = mul_obj_tracker_->GetTrackLets();
  for (auto &tracklet : tracklet_list) {
    if (tracklet->is_update_) {
      auto &track_id = tracklet->tracklet_id_;
      std::shared_ptr<Number> ptrack_id(new Number());
      ptrack_id->value_ = track_id;

      //  get the bbox
      std::shared_ptr<BBox> pbbox;
      HOBOT_CHECK_EQ(GetTailBBox(tracklet, pbbox), XSTREAM_MOT_OK);

      // push result
      ids->datas_.push_back(ptrack_id);
      bboxs->datas_.push_back(pbbox);
    }
  }
  return XSTREAM_MOT_OK;
}

int ReID::GetTrackletsResult(const BaseDataVectorPtr &tracklets) {
  if (!mul_obj_tracker_->is_ready_to_output()) {
    return XSTREAM_MOT_ERR_PARAM;
  }
  auto tracklet_list = mul_obj_tracker_->GetTrackLets();
  for (auto &sp_tracklet : tracklet_list) {
    if (!sp_tracklet || sp_tracklet->is_deleted()) {
      continue;
    }
    std::shared_ptr<ReID_Tracklet> xstream_tracklet(new ReID_Tracklet);
    xstream_tracklet->track_id_ = sp_tracklet->tracklet_id_;
    if (sp_tracklet->is_update_) {
      xstream_tracklet->state_ = DataState::VALID;
    } else {
      xstream_tracklet->state_ = DataState::INVISIBLE;
    }
    int ret1 = xstream_tracklet->SetHobotTracklet(sp_tracklet);
    int ret2 = xstream_tracklet->SetTailFrameId();
    int ret3 = xstream_tracklet->SetTailBBox();
    int ret4 = xstream_tracklet->SetTailReidFeature();
    int ret5 = xstream_tracklet->SetTailOrientation();
    int ret6 = xstream_tracklet->SetTailOcclusion();
    int ret7 = xstream_tracklet->SetTailSkeleton();
    if (ret1 != XSTREAM_MOT_OK || ret2 != XSTREAM_MOT_OK
    || ret3 != XSTREAM_MOT_OK
    || ret4 != XSTREAM_MOT_OK || ret5 != XSTREAM_MOT_OK
    || ret6 != XSTREAM_MOT_OK
    || ret7 != XSTREAM_MOT_OK) {
      return XSTREAM_MOT_NO_RESULT;
    }
    // push result
    tracklets->datas_.push_back(xstream_tracklet);
  }
  return XSTREAM_MOT_OK;
}

int ReID::GetTailFrameId(const spTrackLet &tracklet, uint64_t &frame_id) {
  if (tracklet) {
    frame_id = static_cast<uint64_t>(tracklet->GetTailFrameId());
    return XSTREAM_MOT_OK;
  } else {
    return XSTREAM_MOT_NO_RESULT;
  }
}

int ReID::GetTailBBox(const spTrackLet &tracklet, spBBox &tail_bbox) {
  if (tracklet) {
    auto hobot_bbox = tracklet->GetTailBBox();
    tail_bbox.reset(new BBox());
    tail_bbox->values_.resize(4);
    tail_bbox->values_[0] = hobot_bbox.x1;
    tail_bbox->values_[1] = hobot_bbox.y1;
    tail_bbox->values_[2] = hobot_bbox.x2;
    tail_bbox->values_[3] = hobot_bbox.y2;
    tail_bbox->score_ = hobot_bbox.score;
    tail_bbox->class_ = static_cast<uint32_t>(hobot_bbox.attribute);
    return XSTREAM_MOT_OK;
  } else {
    return XSTREAM_MOT_NO_RESULT;
  }
}

int ReID::GetTailReidFeature(const spTrackLet &tracklet,
                             spFeature &reid_feature) {
  if (tracklet) {
    auto hobot_feature = tracklet->GetTailReidFeature();
    reid_feature.reset(new Feature());
    reid_feature->values_.resize(hobot_feature.size());
    for (int i = 0; i < hobot_feature.size(); i++) {
      reid_feature->values_[i] = hobot_feature[i];
    }
    return XSTREAM_MOT_OK;
  } else {
    return XSTREAM_MOT_NO_RESULT;
  }
}

int ReID::GetTailOrientation(const spTrackLet &tracklet,
                             Orientation &orientation) {
  if (tracklet) {
    auto hobot_orientation = tracklet->GetTailOrientation();
    orientation = TransOrientation(hobot_orientation);
    return XSTREAM_MOT_OK;
  } else {
    return XSTREAM_MOT_NO_RESULT;
  }
}

int ReID::GetTailOcclusion(const spTrackLet &tracklet, bool &occlusion) {
  if (tracklet) {
    occlusion = tracklet->GetTailOcclusion();
    return XSTREAM_MOT_OK;
  } else {
    return XSTREAM_MOT_NO_RESULT;
  }
}

int ReID::GetTailSkeleton(const spTrackLet &tracklet, spLandmark &skeleton) {
  if (tracklet) {
    auto hobot_skeleton = tracklet->GetTailSkeleton();
    skeleton.reset(new Landmark());
    skeleton->points_.resize(hobot_skeleton.points.size());
    assert(hobot_skeleton.points.size() == hobot_skeleton.scores.size());
    for (int i = 0; i < hobot_skeleton.points.size(); i++) {
      skeleton->points_[i].values_.resize(2);
      skeleton->points_[i].values_[0] = hobot_skeleton.points[i].x;
      skeleton->points_[i].values_[1] = hobot_skeleton.points[i].y;
      skeleton->points_[i].score_ = hobot_skeleton.scores[i];
    }
    return XSTREAM_MOT_OK;
  } else {
    return XSTREAM_MOT_NO_RESULT;
  }
}

int ReID_Tracklet::SetTailFrameId() {
  if (hobot_tracklet_) {
    tail_frame_id_ = static_cast<uint64_t>(hobot_tracklet_->GetTailFrameId());
    return XSTREAM_MOT_OK;
  } else {
    return XSTREAM_MOT_NO_RESULT;
  }
}

int ReID_Tracklet::SetTailBBox() {
  if (hobot_tracklet_) {
    hbot::hobot_track::BBox hobot_bbox = hobot_tracklet_->GetTailBBox();
    auto bbox = std::make_shared<BBox>();
    bbox->values_.resize(4);
    bbox->values_[0] = hobot_bbox.x1;
    bbox->values_[1] = hobot_bbox.y1;
    bbox->values_[2] = hobot_bbox.x2;
    bbox->values_[3] = hobot_bbox.y2;
    bbox->score_ = hobot_bbox.score;
    bbox->class_ = static_cast<uint32_t>(hobot_bbox.attribute);
    tail_bbox_ = bbox;
    return XSTREAM_MOT_OK;
  } else {
    return XSTREAM_MOT_NO_RESULT;
  }
}

int ReID_Tracklet::SetTailReidFeature() {
  if (hobot_tracklet_) {
    auto hobot_feature = hobot_tracklet_->GetTailReidFeature();
    auto reid = std::make_shared<Feature>();
    reid->values_.resize(hobot_feature.size());
    for (int i = 0; i < hobot_feature.size(); i++) {
      reid->values_[i] = hobot_feature[i];
    }
    tail_reid_feature_ = reid;
    return XSTREAM_MOT_OK;
  } else {
    return XSTREAM_MOT_NO_RESULT;
  }
}

int ReID_Tracklet::SetTailOrientation() {
  if (hobot_tracklet_) {
    auto hobot_orientation = hobot_tracklet_->GetTailOrientation();
    tail_orientation_ = TransOrientation(hobot_orientation);
    return XSTREAM_MOT_OK;
  } else {
    return XSTREAM_MOT_NO_RESULT;
  }
}

int ReID_Tracklet::SetTailOcclusion() {
  if (hobot_tracklet_) {
    tail_occlusion_ = hobot_tracklet_->GetTailOcclusion();
    return XSTREAM_MOT_OK;
  } else {
    return XSTREAM_MOT_NO_RESULT;
  }
}

int ReID_Tracklet::SetTailSkeleton() {
  if (hobot_tracklet_) {
    auto hobot_skeleton = hobot_tracklet_->GetTailSkeleton();
    auto skeleton = std::make_shared<Landmark>();
    skeleton->points_.resize(hobot_skeleton.points.size());
    assert(hobot_skeleton.points.size() == hobot_skeleton.scores.size());
    for (int i = 0; i < hobot_skeleton.points.size(); i++) {
      skeleton->points_[i].values_.resize(2);
      skeleton->points_[i].values_[0] = hobot_skeleton.points[i].x;
      skeleton->points_[i].values_[1] = hobot_skeleton.points[i].y;
      skeleton->points_[i].score_ = hobot_skeleton.scores[i];
    }
    tail_skeleton_ = skeleton;
    return XSTREAM_MOT_OK;
  } else {
    return XSTREAM_MOT_NO_RESULT;
  }
}

int ReID_Tracklet::SetHobotTracklet(spTrackLet tracklet) {
  if (tracklet) {
    hobot_tracklet_ = std::move(tracklet);
    return XSTREAM_MOT_OK;
  } else {
    return XSTREAM_MOT_ERR_PARAM;
  }
}

uint64_t ReID_Tracklet::GetTailFrameId() {
  return tail_frame_id_;
}

std::shared_ptr<BBox> ReID_Tracklet::GetTailBBox() {
  return tail_bbox_;
}

std::shared_ptr<Feature> ReID_Tracklet::GetTailReidFeature() {
  return tail_reid_feature_;
}

Orientation ReID_Tracklet::GetTailOrientation() {
  return tail_orientation_;
}

bool ReID_Tracklet::GetTailOcclusion() {
  return tail_occlusion_;
}

std::shared_ptr<Landmark> ReID_Tracklet::GetTailSkeleton() {
  return tail_skeleton_;
}

int ReID_Tracklet::GetBBoxOfTimeStamp(uint64_t time_stamp, BBox &bbox) {
  if (hobot_tracklet_) {
    hbot::hobot_track::BBox hobot_bbox;
    bool haskey = hobot_tracklet_->GetBBoxOfTimeStamp(
        static_cast<int64>(time_stamp), hobot_bbox);
    if (haskey) {
      bbox.values_.resize(4);
      bbox.values_[0] = hobot_bbox.x1;
      bbox.values_[1] = hobot_bbox.y1;
      bbox.values_[2] = hobot_bbox.x2;
      bbox.values_[3] = hobot_bbox.y2;
      bbox.score_ = hobot_bbox.score;
      bbox.class_ = static_cast<uint32_t>(hobot_bbox.attribute);
      return XSTREAM_MOT_OK;
    } else {
      return XSTREAM_MOT_NO_RESULT;
    }
  } else {
    return XSTREAM_MOT_ERR_PARAM;
  }
}

int ReID_Tracklet::GetReidFeatureOfTimeStamp(
    uint64_t time_stamp, Feature &reid) {
  if (hobot_tracklet_) {
    std::vector<float> hobot_feature;
    bool haskey = hobot_tracklet_->GetReidFeatureOfTimeStamp(
        static_cast<int64>(time_stamp), hobot_feature);
    if (haskey) {
      reid.values_.resize(hobot_feature.size());
      for (int i = 0; i < hobot_feature.size(); i++) {
        reid.values_[i] = hobot_feature[i];
      }
      return XSTREAM_MOT_OK;
    } else {
      return XSTREAM_MOT_NO_RESULT;
    }
  } else {
    return XSTREAM_MOT_ERR_PARAM;
  }
}

int ReID_Tracklet::GetOrientationOfTimeStamp(
    uint64_t time_stamp, Orientation &orientation) {
  if (hobot_tracklet_) {
    hbot::hobot_track::Orientation hobot_orientation;
    bool haskey = hobot_tracklet_->GetOrientationOfTimeStamp(
                      static_cast<int64>(time_stamp), hobot_orientation);
    if (haskey) {
      orientation = TransOrientation(hobot_orientation);
      return XSTREAM_MOT_OK;
    } else {
      return XSTREAM_MOT_NO_RESULT;
    }
  } else {
    return XSTREAM_MOT_ERR_PARAM;
  }
}

int ReID_Tracklet::GetOcclusionOfTimeStamp(
    uint64_t time_stamp, bool &is_occlusion) {
  if (hobot_tracklet_) {
    bool haskey = hobot_tracklet_->GetOcclusionOfTimeStamp(
        static_cast<int64>(time_stamp), is_occlusion);
    if (haskey) {
      return XSTREAM_MOT_OK;
    } else {
      return XSTREAM_MOT_NO_RESULT;
    }
  } else {
    return XSTREAM_MOT_ERR_PARAM;
  }
}

int ReID_Tracklet::GetSkeletonOfTimeStamp(
    uint64_t time_stamp, Landmark &human_skeleton) {
  if (hobot_tracklet_) {
    hobot::vision::HumanSkeleton hobot_skeleton;
    bool haskey = hobot_tracklet_->GetSkeletonOfTimeStamp(
        static_cast<int64>(time_stamp), hobot_skeleton);
    if (haskey) {
      human_skeleton.points_.resize(hobot_skeleton.points.size());
      assert(hobot_skeleton.points.size() == hobot_skeleton.scores.size());
      for (int i = 0; i < hobot_skeleton.points.size(); i++) {
        human_skeleton.points_[i].values_.resize(2);
        human_skeleton.points_[i].values_[0] = hobot_skeleton.points[i].x;
        human_skeleton.points_[i].values_[1] = hobot_skeleton.points[i].y;
        human_skeleton.points_[i].score_ = hobot_skeleton.scores[i];
      }
      return XSTREAM_MOT_OK;
    } else {
      return XSTREAM_MOT_NO_RESULT;
    }
  } else {
    return XSTREAM_MOT_ERR_PARAM;
  }
}







}  // namespace xstream
