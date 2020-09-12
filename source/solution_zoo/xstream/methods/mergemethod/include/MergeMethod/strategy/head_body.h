/*
 * @Description: implement of data_type
 * @Author: peng02.li@horizon.ai
 * @Date: 2019-11-15 17:49:26
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-12-18 17:16:50
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */

#ifndef INCLUDE_MERGEMETHOD_STRATEGY_HEAD_BODY_H_
#define INCLUDE_MERGEMETHOD_STRATEGY_HEAD_BODY_H_

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

struct HeadBodyParam : MergeParam {
 public:
  HeadBodyParam() {}
  int UpdateParameter(const JsonReaderPtr &reader) override;
  bool use_kps = true;
  bool double_thresh_flag = false;
  double kps_cnt_threshold = 0.5;
  double conflict_threshold = 0.8;
  double valid_kps_score_thresh = 0.2;
  double head_extend_ratio = 0.05;
};

class HeadBodyStrategy : public MergeStrategy {
 public:
  int Init(std::shared_ptr<MergeParam> config) override;

  std::vector<BaseDataPtr> ProcessFrame(const std::vector<BaseDataPtr> &in,
                                        const InputParamPtr &param) override;

  void Finalize() override;

  int UpdateParameter(const JsonReaderPtr &reader) override;

 private:
  std::shared_ptr<HeadBodyParam> GetConfig();

  void RunSingleFrame(const std::vector<BaseDataPtr> &frame_input,
                      std::vector<BaseDataPtr> *frame_output);

  void PassThroughSingleFrame(const std::vector<BaseDataPtr> &frame_input,
                              std::vector<BaseDataPtr> *frame_output);

  void ProduceOutput(const BaseDataPtr &faceBoxPtr,
                     const BaseDataPtr &headBoxPtr,
                     const BaseDataPtr &bodyBoxPtr,
                     const BaseDataPtr &bodyKpsPtr,
                     const BaseDataVectorPtr &faceResPtr,
                     const BaseDataVectorPtr &headResPtr,
                     const BaseDataVectorPtr &bodyResPtr);
  void Pairs2Map(const std::vector<std::pair<int, int>> &pairs,
                 std::unordered_map<int, int> *index_map);

  std::vector<std::pair<int, int>> GetHeadFacePairs(
      const BaseDataPtr &head_box_ptr, const BaseDataPtr &face_box_ptr);

  std::vector<std::pair<int, int>> GetHeadBodyPairsWithKps(
      const BaseDataPtr &head_box_ptr, const BaseDataPtr &body_box_ptr,
      const BaseDataPtr &body_kps_ptr);

  std::vector<std::pair<int, int>> GetHeadBodyPairsWithoutKps(
      const BaseDataPtr &head_box_ptr, const BaseDataPtr &body_box_ptr,
      bool double_thresh_flag = false);

  std::vector<std::pair<int, int>> GreedySearch(std::vector<float> *score_mat,
                                                int widht, int height,
                                                float thresh);
};

}  // namespace xstream

#endif  // INCLUDE_MERGEMETHOD_STRATEGY_HEAD_BODY_H_
