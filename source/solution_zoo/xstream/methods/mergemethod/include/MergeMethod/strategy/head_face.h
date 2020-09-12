/*
 * @Description: implement of data_type
 * @Author: ruoting.ding@horizon.ai
 * @Date: 2019-11-15 17:49:26
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-11-20 11:54:01
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */

#ifndef INCLUDE_MERGEMETHOD_STRATEGY_HEAD_FACE_H_
#define INCLUDE_MERGEMETHOD_STRATEGY_HEAD_FACE_H_

#include <iostream>
#include <memory>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

#include "MergeMethod/data_type/data_type.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_data.h"
#include "horizon/vision_type/vision_error.h"
#include "horizon/vision_type/vision_type.hpp"

namespace xstream {

struct HeadFaceParam : MergeParam {
 public:
  HeadFaceParam() {}
  int UpdateParameter(const JsonReaderPtr &reader) override;
};

class HeadFaceStrategy : public MergeStrategy {
 public:
  int Init(std::shared_ptr<MergeParam> config) override;

  std::vector<BaseDataPtr> ProcessFrame(const std::vector<BaseDataPtr> &in,
                                        const InputParamPtr &param) override;

  void Finalize() override;

  int UpdateParameter(const JsonReaderPtr &reader) override;

 private:
  std::shared_ptr<HeadFaceParam> GetConfig();

 protected:
  std::set<int> GetIdSet(const BaseDataPtr &boxPtr);

  void GetMainId(const BaseDataPtr &bboxPtr,
                 const std::unordered_map<int, int> &merged_info,
                 const std::shared_ptr<BaseDataVector> &mergedBBoxPtr);

  int FindConflictTrackIdFromCurrentFrame(
      std::unordered_map<int, int> face_id2main_id,
      const std::set<int> &cur_face_id_set, const int main_id);

  int FindConflictTrackIDFromAllTrack(
      const std::unordered_map<int, int> &obj_id2main_id, const int main_id);

  // 这两个函数合并到ProcessFrame中
  void RunSingleFrame(const std::vector<BaseDataPtr> &frame_input,
                      std::vector<BaseDataPtr> *frame_output);

  void PassThroughSingleFrame(const std::vector<BaseDataPtr> &frame_input,
                              std::vector<BaseDataPtr> *frame_output);

  void ProduceOutput(const BaseDataPtr &facePtr, const BaseDataPtr &headPtr,
                     const BaseDataVectorPtr &faceResPtr,
                     const BaseDataVectorPtr &headResPtr);

  void MergeFaceHeadTrackID(const BaseDataPtr &face_bbox,
                            const BaseDataPtr &head_bbox,
                            const std::set<int> &face_id_set,
                            const std::set<int> &head_id_set,
                            IDRelationInfo *id_merge_info);

  void RemoveDisappearedMotId(
      const Type &type, const BaseDataPtr &idPtr,
      const std::shared_ptr<BaseDataVector> &disappereadTrackIdPtr,
      IDRelationInfo *id_relation_info);

  void RemoveDisappearedTrack(const BaseDataPtr &disappearedFacePtr,
                              const BaseDataPtr &disappearedHeadPtr,
                              const BaseDataVectorPtr &DisappereadTrackIdPtr);

  std::vector<std::pair<int, int>> GetMatchedPairs(const BaseDataPtr &face_bbox,
                                                   const BaseDataPtr &head_bbox,
                                                   const float &threshold);

  IDRelationInfo id_relation_info_;

  BBoxOffset box_offset_;

  std::vector<int> conflict_ids_;
};
}  // namespace xstream

#endif  // INCLUDE_MERGEMETHOD_STRATEGY_HEAD_FACE_H_
