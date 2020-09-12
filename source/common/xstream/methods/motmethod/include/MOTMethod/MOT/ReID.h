/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     ReID MOT header
 * @author    chao.yang
 * @email     chao01.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.18
 */

#ifndef MOTMETHOD_MOT_REID_H_
#define MOTMETHOD_MOT_REID_H_

#include <string>
#include <vector>
#include <list>
#include <memory>

#include "MOTMethod/MOTMethod.h"
#include "hobotxstream/data_types/image_frame.h"
#include "multi_obj_tracking/multi_obj_tracking.hpp"
#include "trackers/mul_obj_tracker.hpp"
#include "hobotxstream/data_types/tracklet.h"


namespace xstream {

typedef std::shared_ptr<BaseDataVector> BaseDataVectorPtr;

class ReID : public Mot {
 public:
  int MotInit(const std::string &config_file_path) override;

  int track(const std::vector<BaseDataPtr> &in,
  std::vector<BaseDataPtr> &out) override;

  void MotFinalize() override {};

 private:
  cv::Mat convertImageFrame2Mat(const ImageFrame *frame);

  int XStreamData2VisionType(const std::vector<BaseDataPtr> &in,
                    uint64_t &timestamp,
                    uint64_t &frame_count,
                    cv::Mat &src,
                    std::list<hbot::hobot_track::BBox> &face_det_list,
                    std::list<hbot::hobot_track::BBox> &rect_list,
                    std::vector<bool> &rect_status,
                    std::vector<hobot::vision::HumanSkeleton> &skeleton_list,
                    std::vector<std::vector<float>> &reid_list);

  int GetIdBBoxResult(const BaseDataVectorPtr &ids,
                      const BaseDataVectorPtr &bboxs);

  int GetTrackletsResult(const BaseDataVectorPtr &tracklets);

  int UpdateState(const uint64_t &frame_id,
                  const BaseDataVectorPtr &disappeared_idxs_msg);

  std::shared_ptr<hbot::hobot_track::MulObjTracker> mul_obj_tracker_;

  typedef std::shared_ptr<BBox> spBBox;

  typedef std::shared_ptr<Feature> spFeature;

  typedef std::shared_ptr<Landmark> spLandmark;

  static int GetTailFrameId(const spTrackLet &tracklet, uint64_t &frame_id);

  static int GetTailBBox(const spTrackLet &tracklet, spBBox &tail_bbox);

  static int GetTailReidFeature(const spTrackLet &tracklet,
  spFeature &reid_feature);

  static int GetTailOrientation(const spTrackLet &tracklet,
  Orientation &orientation);

  static int GetTailOcclusion(const spTrackLet &tracklet, bool &occlusion);

  static int GetTailSkeleton(const spTrackLet &tracklet,
  spLandmark &skeleton);
};

class ReID_Tracklet : public Tracklet {
 public:
  int SetHobotTracklet(spTrackLet tracklet);

  int SetTailFrameId();

  int SetTailBBox();

  int SetTailReidFeature();

  int SetTailOrientation();

  int SetTailOcclusion();

  int SetTailSkeleton();

  uint64_t GetTailFrameId() override;

  std::shared_ptr<BBox> GetTailBBox() override;

  std::shared_ptr<Feature> GetTailReidFeature() override;

  Orientation GetTailOrientation() override;

  bool GetTailOcclusion() override;

  std::shared_ptr<Landmark> GetTailSkeleton() override;


  /// Warning: This interface is not thread safe now
  /// and only used for debugging and evaluation.
  int GetBBoxOfTimeStamp(uint64_t time_stamp, BBox& bbox) override;

  /// Warning: This interface is not thread safe now
  /// and only used for debugging and evaluation.
  int GetReidFeatureOfTimeStamp(uint64_t time_stamp,
  Feature &reid) override;

  /// Warning: This interface is not thread safe now
  /// and only used for debugging and evaluation.
  int GetOrientationOfTimeStamp(uint64_t time_stamp,
  Orientation &orientation) override;

  /// Warning: This interface is not thread safe now
  /// and only used for debugging and evaluation.
  int GetOcclusionOfTimeStamp(uint64_t time_stamp, bool &is_occlusion) override;

  /// Warning: This interface is not thread safe now
  /// and only used for debugging and evaluation.
  int GetSkeletonOfTimeStamp(uint64_t time_stamp,
  Landmark &human_skeleton) override;

 private:
  spTrackLet hobot_tracklet_;

  uint64_t tail_frame_id_;

  std::shared_ptr<BBox> tail_bbox_;

  std::shared_ptr<Feature> tail_reid_feature_;

  Orientation tail_orientation_;

  bool tail_occlusion_;

  std::shared_ptr<Landmark> tail_skeleton_;
};

}  // namespace xstream

#endif  // MOTMETHOD_MOT_REID_H_
