//
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
// Created by peng02.li on 12/21/19.
//

#ifndef CNNMETHOD_UTIL_ACTDATAPREPROCESS_H_
#define CNNMETHOD_UTIL_ACTDATAPREPROCESS_H_

#include <math.h>
#include <assert.h>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <memory>
#include <fstream>
#include <mutex>
#include "horizon/vision_type/vision_type.hpp"
#include "hobotxsdk/xstream_data.h"
#include "hobotlog/hobotlog.hpp"
#include "xproto/utils/singleton.h"

using hobot::vision::BBox;
using hobot::vision::Point;
using hobot::vision::Landmarks;
using xstream::BaseData;
using xstream::BaseDataVector;
using xstream::XStreamData;

struct Tensor {
 public:
  Tensor(int dim1_, int dim2_, int dim3_, int dim4_) :
        dim1(dim1_), dim2(dim2_), dim3(dim3_), dim4(dim4_) {
    if (!(dim1_ > 0 && dim2_ > 0 && dim3_ > 0 && dim4_ > 0)) {
        LOGE << "Failed to create tensor of shape: " << dim1_ << " "
             << dim2_ << " " << dim3_ << " " << dim4_;
    }
    HOBOT_CHECK(dim1_ >= 0 && dim2_ > 0 && dim3_ > 0 && dim4_ > 0);
    data = std::vector<float>(dim1_ * dim2_ * dim3_ * dim4_);
  }

  float At(int x1, int x2, int x3, int x4) {
    return data[x1 * dim2 * dim3 * dim4 + x2 * dim3 * dim4 + x3 * dim4 + x4];
  }

  void Set(int x1, int x2, int x3, int x4, float value) {
    if (!(x1 >= 0 && x1 < dim1) || !(x2 >= 0 && x2 < dim2) ||
        !(x3 >= 0 && x3 < dim3) || !(x4 >= 0 && x4 < dim4)) {
      LOGE << "Error set tensor at position: " << x1 << " " << x2 << " "
           << x3 << " " << x4 << ", with shape: " << dim1 << " "
           << dim2 << " " << dim3 << " " << dim4;
    }
    HOBOT_CHECK(x1 >= 0 && x1 < dim1);
    HOBOT_CHECK(x2 >= 0 && x2 < dim2);
    HOBOT_CHECK(x3 >= 0 && x3 < dim3);
    HOBOT_CHECK(x4 >= 0 && x4 < dim4);
    data[x1 * dim2 * dim3 * dim4 + x2 * dim3 * dim4 + x3 * dim4 + x4] = value;
  }

  void Display(int box_idx, int box_num, int handle_num) {
    std::stringstream sstream;
    sstream.precision(17);
    sstream.setf(std::ios::fixed);
    sstream << "[";
    for (int n = 0; n < dim1; ++n) {
      sstream << "[";
      for (int c = 0; c < dim4; ++c) {
        sstream << "[";
        for (int h = 0; h < dim2; ++h) {
          sstream << "[";
          for (int w = 0; w < dim3; ++w) {
            auto value =
                data[n * dim2 * dim3 * dim4 + h * dim3 * dim4 + w * dim4 + c];
            sstream << value;
            if (w != dim3 - 1) sstream << ", ";
          }
          sstream << "]";
          if (h != dim2 - 1) sstream << ", ";
        }
        sstream << "]";
        if (c != dim4 - 1) sstream << ", ";
      }
      sstream << "]";
      if (n != dim1 - 1) sstream << ", ";
    }
    sstream << "]";
    if (box_idx != box_num - 1 || box_idx != handle_num - 1) {
      sstream << ", ";
    }
    std::fstream outfile;
    outfile.open("lmkseq_input_feature.txt", std::ios_base::app);
    outfile << sstream.str().c_str();
  }

  std::vector<float> data;

 private:
  int dim1;
  int dim2;
  int dim3;
  int dim4;
};

class FeatureSequenceBuffer {
 public:
  FeatureSequenceBuffer() {
    max_len_ = 100;
    len_ = 0;
    feats_ = std::make_shared<BaseDataVector>();
    boxes_ = std::make_shared<BaseDataVector>();
  }
  explicit FeatureSequenceBuffer(int buff_len_) :
      len_(0), max_len_(buff_len_) {
    feats_ = std::make_shared<BaseDataVector>();
    boxes_ = std::make_shared<BaseDataVector>();
  }

  void Update(std::shared_ptr<BaseData> box,
              std::shared_ptr<BaseData> kps, float timestamp) {
    while (len_ > max_len_) {
      LOGD << "overflow, removing..., len: " << len_;
      timestamps_.erase(timestamps_.begin());
      feats_->datas_.erase(feats_->datas_.begin());
      boxes_->datas_.erase(boxes_->datas_.begin());
      len_ -= 1;
    }
    feats_->datas_.push_back(kps);
    boxes_->datas_.push_back(box);
    timestamps_.push_back(timestamp);
    len_ += 1;
  }

  void GetClipFeatByEnd(std::shared_ptr<BaseDataVector> kpses,
                        std::shared_ptr<BaseDataVector> boxes, int num,
                        float stride, float margin, float end) {
    kpses->datas_.clear();
    boxes->datas_.clear();
    if (len_ < 1) {
        LOGW << "len_ < 1, return...";
        return;
    }

    std::vector<int> clip_idxs(num, -1);
    for (int frame_idx = 0; frame_idx < num; ++frame_idx) {
      // timestamp for expected candidate
      float curtime = end - frame_idx * stride;
      // time gap
      float restime = 1e10;
      for (int idx = len_ - 1; idx >= 0; --idx) {
        if (fabs(timestamps_[idx] - curtime) < restime) {
          restime = fabs(timestamps_[idx] - curtime);
          if (restime < margin) {
              clip_idxs[num - 1 - frame_idx] = idx;
          }
        } else {
          break;
        }
      }
    }

    int maxValue = clip_idxs[0];
    int minValue = clip_idxs[0];
    for (auto & item : clip_idxs) {
      if (item < minValue) {
        minValue = item;
      }
      if (item > maxValue) {
        maxValue = item;
      }
    }
    if  (maxValue >= len_ || minValue < 0) {
      LOGD << "Failed to get clip kps by end, invalid index. "
           << "max_idx: " << maxValue << " min_idx: " << minValue
           << " length: " << len_;
      return;
    }

    for (auto& idx : clip_idxs) {
      auto tmp_kps_value = std::static_pointer_cast<
          XStreamData<Landmarks>>(feats_->datas_[idx])->value;
      auto p_tmp_kps = std::make_shared<XStreamData<Landmarks>>();
      p_tmp_kps->value = tmp_kps_value;
      kpses->datas_.push_back(p_tmp_kps);
      boxes->datas_.push_back(boxes_->datas_[idx]);
    }
  }

  void Clean() {
    timestamps_.clear();
    feats_->datas_.clear();
    boxes_->datas_.clear();
  }

 private:
  int len_ = 0;
  int max_len_;
  std::vector<float> timestamps_;
  std::shared_ptr<BaseDataVector> feats_;
  std::shared_ptr<BaseDataVector> boxes_;
};

class ActDataPreprocess : public hobot::CSingleton<ActDataPreprocess> {
 public:
  // ActDataPreprocess();

  int Init(int num_kps, int seq_len, float stride, float max_gap,
           float kps_norm_scale, bool norm_kps_conf,
           int buffer_len = 100) {
    num_kps_ = num_kps;
    seq_len_ = seq_len;
    stride_ = stride;
    max_gap_ = max_gap;
    kps_norm_scale_ = kps_norm_scale;
    buff_len_ = buffer_len;
    norm_kps_conf_ = norm_kps_conf;
    LOGD << "num_kps: " << num_kps << ", seq_len: " << seq_len
         << ", stride: " << stride << ", max_gap: " << max_gap
         << ", buff_len: " << buffer_len << "kps_norm_scale: "
         << kps_norm_scale_;
    return 0;
  }

  void Update(std::shared_ptr<BaseData> box,
              std::shared_ptr<BaseData> kps, float timestamp) {
    std::unique_lock<std::mutex> lock(map_mutex_);
    auto p_box = std::static_pointer_cast<XStreamData<BBox>>(box);
    auto track_id = p_box->value.id;
    if (track_buffers_.find(track_id) == track_buffers_.end()) {
      FeatureSequenceBuffer buffer(buff_len_);
      track_buffers_[track_id] = buffer;
    }
    auto kps_value = std::static_pointer_cast<
        XStreamData<Landmarks>>(kps)->value;
    auto p_kps = std::make_shared<XStreamData<Landmarks>>();
    p_kps->value = kps_value;
    track_buffers_[track_id].Update(box, p_kps, timestamp);
  }

  void NormKps(std::shared_ptr<BaseDataVector> kpses,
               std::shared_ptr<BaseDataVector> boxes,
               xstream::LmkSeqOutputType type) {
    HOBOT_CHECK(kpses->datas_.size() == static_cast<uint32_t>(seq_len_));
    auto first_kps =
        std::static_pointer_cast<XStreamData<Landmarks>>(kpses->datas_[0]);
    Point center;
    if (type == xstream::LmkSeqOutputType::FALL) {
      Point left_hip = first_kps->value.values[11];
      Point right_hip = first_kps->value.values[12];
      float hip_x = left_hip.x * 0.5 + right_hip.x * 0.5;
      float hip_y = left_hip.y * 0.5 + right_hip.y * 0.5;
      center.x = hip_x;
      center.y = hip_y;
    } else if (type == xstream::LmkSeqOutputType::GESTURE) {
      center = first_kps->value.values[9];
    }
    LOGD << "center_x: " << center.x << ", center_y: " << center.y;
    max_score_ = -1;
    for (auto p_kps : kpses->datas_) {
      auto kps = std::static_pointer_cast<XStreamData<Landmarks>>(p_kps);
      for (int i = 0; i < num_kps_; ++i) {
        kps->value.values[i].x -= center.x;
        kps->value.values[i].y -= center.y;

        if (kps->value.values[i].score > max_score_) {
          max_score_ = kps->value.values[i].score;
        }
      }
    }
    LOGD << "MAX SCORE " << max_score_;
    if (max_score_ >= 2.0 || norm_kps_conf_) {
      for (auto p_kps : kpses->datas_) {
        auto kps = std::static_pointer_cast<XStreamData<Landmarks>>(p_kps);
        for (int i = 0; i < num_kps_; ++i) {
          kps->value.values[i].score /= kps_norm_scale_;
        }
      }
    }
    float max_width = -1;
    float max_height = -1;
    for (auto p_box : boxes->datas_) {
      auto box = std::static_pointer_cast<XStreamData<BBox>>(p_box);
      float cur_width = box->value.x2 - box->value.x1;
      float cur_height = box->value.y2 - box->value.y1;
      if (cur_width > max_width) {
        max_width = cur_width;
      }
      if (cur_height > max_height) {
        max_height = cur_height;
      }
    }
    LOGD << "max width: " << max_width << ", max_height: " << max_height;
    float max_border = std::max(max_width, max_height);
    for (auto p_kps : kpses->datas_) {
      auto kps = std::static_pointer_cast<XStreamData<Landmarks>>(p_kps);
      for (int i = 0; i < num_kps_; ++i) {
        kps->value.values[i].x /= max_border;
        kps->value.values[i].y /= max_border;
      }
    }
  }

  void GetClipKps(std::shared_ptr<BaseDataVector> kpses,
                  int track_id, float times_tamp,
                  xstream::LmkSeqOutputType type) {
    std::unique_lock<std::mutex> lock(map_mutex_);
    auto boxes = std::make_shared<BaseDataVector>();
    track_buffers_[track_id].GetClipFeatByEnd(
        kpses, boxes, seq_len_, stride_, max_gap_, times_tamp);
    if (kpses->datas_.size() < 1) {
      LOGD << "No clip kps get";
      return;
    }
    LOGD << "Got clip feat by end";
    NormKps(kpses, boxes, type);
  }

  void Clean(std::shared_ptr<BaseDataVector> disappeared_track_ids) {
    std::unique_lock<std::mutex> lock(map_mutex_);
    LOGD << "data preprocessor clean() called";
    for (size_t i = 0; i < disappeared_track_ids->datas_.size(); ++i) {
      auto disappeared_track_id =
          std::static_pointer_cast<XStreamData<uint32_t>>(
                  disappeared_track_ids->datas_[i])->value;
      if (track_buffers_.find(disappeared_track_id) != track_buffers_.end()) {
        track_buffers_[disappeared_track_id].Clean();
        track_buffers_.erase(disappeared_track_id);
      }
    }
  }

 private:
  std::map<int, FeatureSequenceBuffer> track_buffers_;
  int num_kps_;
  int seq_len_;
  float stride_;
  float max_gap_;
  int buff_len_;
  int rotate_degree_;
  bool norm_kps_conf_;
  float max_score_ = -1;
  float kps_norm_scale_ = 1;
  std::mutex map_mutex_;
};


#endif  // CNNMETHOD_UTIL_ACTDATAPREPROCESS_H_
