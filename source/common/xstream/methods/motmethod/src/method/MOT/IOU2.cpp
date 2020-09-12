/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @brief     IOU2 MOT Implementation
 * @author    dezhi.zeng
 * @email     dezhi.zeng@horizon.ai
 * @version   0.0.0.1
 * @date      2019.11.18
 */

#include <memory>
#include <fstream>
#include <string>


#include "MOTMethod/MOT/IOU2.h"
#include "MOTMethod/data_type/mot_data_type.hpp"
#include "MOTMethod/error_code.h"
#include "horizon/vision_type/vision_type.hpp"
#include "hobotlog/hobotlog.hpp"
#include "json/json.h"


#define INVALID_ID -1

namespace xstream {

typedef XStreamData<uint32_t> XStreamUint32;
typedef XStreamData<hobot::vision::BBox> XStreamBBox;

typedef std::shared_ptr<hobot::vision::ImageFrame> ImageFramePtr;
typedef XStreamData<ImageFramePtr> XStreamImageFrame;


int IOU2::MotInit(const std::string &config_file_path) {
  LOGI << "IOU2 Mot::Init " << config_file_path << std::endl;
  this->tracker_ = std::make_shared<hobot::iou_mot::Tracker>(0);

  config_param_ = std::make_shared<IOU2Param>();
  std::ifstream config_if(config_file_path);
  if (!config_if.good()) {
    LOGI << "IOU2Param: no config, using default parameters" << std::endl;
  } else {
    std::ostringstream buf;
    char ch;
    while (buf && config_if.get(ch)) {
      buf.put(ch);
    }
    config_param_->UpdateParameter(buf.str());
  }
  return SetTrackerConfig();
}

int IOU2::SetTrackerConfig() {
  auto iou2_config = GetConfig();
  auto config = hobot::iou_mot::ConfigMessage::instance();
  config->track_param.match_type = iou2_config->match_type;
  config->track_param.use_kalman_filter = iou2_config->use_kalman_filter;
  config->track_param.missing_time_thres = iou2_config->missing_time_thres;
  config->track_param.remove_invisible_track_ms
                     = iou2_config->vanish_frame_count*iou2_config->time_gap;
  config->track_param.remove_obsolete_track
                           = iou2_config->remove_obsolete_track;
  config->track_param.iou_thres = iou2_config->iou_thres;
  config->track_param.euclidean_thres = iou2_config->euclidean_thres;
  config->track_param.use_location_gain = iou2_config->use_location_gain;
  config->track_param.max_trajectory_number
                           = iou2_config->max_trajectory_number;

  config->detection_param.min_score = iou2_config->min_score;
  config->detection_param.ignore_overlap_thres
                           = iou2_config->ignore_overlap_thres;
  return XSTREAM_MOT_OK;
}
int IOU2::Track(const std::vector<BaseDataPtr> &in,
               std::vector<BaseDataPtr> &out) {
  HOBOT_CHECK(in.size() >= 2);
  auto img_frame = std::static_pointer_cast<BaseData>(in[0]);
  auto box_list = std::static_pointer_cast<BaseDataVector>(in[1]);
  HOBOT_CHECK(img_frame) << "Lost image frame";
  HOBOT_CHECK(box_list) << "Lost face_head_body boxes";
  HOBOT_CHECK("ImageFrame" == img_frame->type_);
  HOBOT_CHECK("BaseDataVector" == box_list->type_);

  auto pframe = std::static_pointer_cast<XStreamImageFrame>(img_frame);
//  time_t time_stamp = pframe->value->time_stamp/1000;
  time_t time_stamp = pframe->value->time_stamp;
  uint32_t frame_width = pframe->value->Width();
  uint32_t frame_height = pframe->value->Height();

  size_t item_size = box_list->datas_.size();
  LOGI << "data size:" << item_size;

  auto out_rects = std::make_shared<BaseDataVector>();
  auto out_disappeared_ids = std::make_shared<BaseDataVector>();

  out.push_back(std::static_pointer_cast<BaseData>(out_rects));
  out.push_back(std::static_pointer_cast<BaseData>(out_disappeared_ids));

  std::vector<hobot::iou_mot::sp_BBox> boxes;
  RectMsg2Box(&boxes, box_list.get(), frame_width, frame_height);
  tracker_->TrackPro(boxes, time_stamp, frame_width, frame_height);

  /* transfer track targets */
  copy_inrects_to_out(box_list.get(), &out_rects->datas_,
                                frame_width, frame_height);
  track_to_rects(time_stamp,
               tracker_->tracklet_list(),
               &out_rects->datas_,
               &out_disappeared_ids->datas_);

  return XSTREAM_MOT_OK;
}

void IOU2::RectMsg2Box(std::vector<hobot::iou_mot::sp_BBox> *boxes,
                                       BaseDataVector *rects_msg,
                                       const int &img_width,
                                       const int &img_height) {
  size_t size = rects_msg->datas_.size();

  for (size_t i = 0; i < size; i++) {
    auto &in_rect = rects_msg->datas_[i];
    // HOBOT_CHECK("BBox" == in_rect->type_);
    if (DataState::VALID != in_rect->state_)
      continue;
    auto bbox = std::static_pointer_cast<XStreamBBox>(in_rect);
    hobot::iou_mot::sp_BBox iou_bbox =
      std::make_shared<hobot::iou_mot::BBox_s>(
           std::max(0, static_cast<int>(std::lround(bbox->value.x1))),
           std::max(0, static_cast<int>(std::lround(bbox->value.y1))),
           std::min(img_width - 1,
                    static_cast<int>(std::lround(bbox->value.x2))),
           std::min(img_height - 1,
                    static_cast<int>(std::lround(bbox->value.y2))));
    iou_bbox->score = bbox->value.score;
    iou_bbox->box_id = i;
    boxes->emplace_back(iou_bbox);
  }
}

void IOU2::copy_inrects_to_out(BaseDataVector *rects_msg,
                std::vector<BaseDataPtr> *p_out_rects,
                const int &img_width,
                const int &img_height) {
  size_t size = rects_msg->datas_.size();

  for (size_t i = 0; i < size; i++) {
    auto &in_rect = rects_msg->datas_[i];
    // HOBOT_CHECK("BBox" == in_rect->type_);
    auto inbox = std::static_pointer_cast<XStreamBBox>(in_rect);

    std::shared_ptr<XStreamBBox> outbox(new XStreamBBox());
    outbox->type_ = "BBox";
    outbox->value.id = -1;
    outbox->value.x1 = std::max(0,
                         static_cast<int>(std::lround(inbox->value.x1)));
    outbox->value.y1 = std::max(0,
                         static_cast<int>(std::lround(inbox->value.y1)));
    outbox->value.x2 = std::min(img_width - 1,
                         static_cast<int>(std::lround(inbox->value.x2)));
    outbox->value.y2 = std::min(img_height - 1,
                         static_cast<int>(std::lround(inbox->value.y2)));
    outbox->value.score = inbox->value.score;
    outbox->state_ = DataState::INVALID;
    p_out_rects->push_back(outbox);
  }
}

void IOU2::track_to_rects(const time_t &time_stamp,
                const std::vector<hobot::iou_mot::sp_TrackLet> &tracklet_list,
                std::vector<BaseDataPtr> *p_out_rects,
                std::vector<BaseDataPtr> *p_disappeared_ids) {
  for (const auto &tracklet : tracklet_list) {
    if (tracklet->state == hobot::iou_mot::TrackLet::Deleted) {
      std::shared_ptr<XStreamUint32> track_id(new XStreamUint32());
      track_id->type_ = "Number";
      track_id->value = tracklet->track_id;
      track_id->state_ = DataState::DISAPPEARED;
      p_disappeared_ids->push_back(track_id);
    } else if (p_out_rects->size()) {
      bool flag = false;
      hobot::iou_mot::sp_Target target_out =
        tracklet->GetTargetOfTimeStamp(time_stamp, flag);
      if (target_out) {
        int box_id = target_out->body_bbox->box_id;
        if (box_id >= 0 &&
            static_cast<uint32_t>(box_id) < p_out_rects->size()) {
          auto bbox =
              std::static_pointer_cast<XStreamBBox>((*p_out_rects)[box_id]);
          bbox->value.id = tracklet->track_id;
          bbox->state_ = DataState::VALID;
        }
      }
      // if (flag) {
      //   int box_id = target_out->body_bbox->box_id;
      //   auto bbox = std::static_pointer_cast<XStreamBBox>(
      //                               (*p_out_rects)[box_id]);
      //   bbox->value.id = tracklet->track_id;
      //   bbox->state_ = DataState::VALID;

      //   HOBOT_CHECK_EQ(bbox->type_, "BBox");
      //   HOBOT_CHECK_EQ(bbox->value.x1, target_out->body_bbox->x1);
      //   HOBOT_CHECK_EQ(bbox->value.y1, target_out->body_bbox->y1);
      //   HOBOT_CHECK_EQ(bbox->value.x2, target_out->body_bbox->x2);
      //   HOBOT_CHECK_EQ(bbox->value.y2, target_out->body_bbox->y2);
      //   HOBOT_CHECK_EQ(bbox->value.score, target_out->body_bbox->score);
      // }
    }
  }
}

std::shared_ptr<IOU2Param> IOU2::GetConfig() {
  auto select_config =
      std::static_pointer_cast<IOU2Param>(config_param_);
  return select_config;
}

void IOU2::MotFinalize() {
}

int IOU2::UpdateParameter(const std::string &content) {
  int ret = config_param_->UpdateParameter(content);
  if (XSTREAM_MOT_OK == ret) {
    ret = SetTrackerConfig();
  }
  return ret;
}
}  // namespace xstream

