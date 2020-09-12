/*
 * @Description: filter method
 * @Author:  hangjun.yang@horizon.ai
 * @Date: 2019-1-11 10:30:32
 * @Author: chao.yang@horizon.ai
 * @Date: 2019-5-27 10:30:32
 * @LastEditors  : hao.tian@horizon.ai
 * @LastEditTime : 2020-01-07 11:41:53
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */
#ifndef FilterMethod_FilterMethod_H_
#define FilterMethod_FilterMethod_H_

#include <algorithm>
#include <cassert>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "FilterMethod/filter_data_type.hpp"
#include "hobotxstream/method.h"
#include "horizon/vision_type/vision_type.hpp"
namespace xstream {

#define FILTER_LOG(index, item) \
  LOGI << "item index:" << index << " filtered by " << item;

#define FILTER_LOG_VALUE(index, item, value)                \
  LOGI << "item index:" << index << " filtered by " << item \
       << " with the value: " << value;

struct FilterState {
  FilterState() : is_filter_(false), is_normal_(false) {}
  bool is_filter_;
  bool is_normal_;
};

typedef std::shared_ptr<FilterState> FilterStatePtr;

class FilterMethod : public Method {
 public:
  int Init(const std::string &config_file_path) override;

  std::vector<std::vector<BaseDataPtr>> DoProcess(
      const std::vector<std::vector<BaseDataPtr>> &input,
      const std::vector<xstream::InputParamPtr> &param) override;

  void Finalize() override;

  int UpdateParameter(InputParamPtr ptr) override;

  InputParamPtr GetParameter() const override;

  std::string GetVersion() const override { return "0.0.16"; }

  void OnProfilerChanged(bool on) override {}

 private:
  //  filter by stop id
  bool MatchStopId(const int32_t &id);
  //  filter by 3d pose
  bool ReachPoseThreshold(const float &pitch, const float &yaw,
                          const float &roll = 0);
  //  filter by size
  bool ReachSizeThreshold(const float &x1, const float &y1, const float &x2,
                          const float &y2, const int &size_thr);
  //  filter by upper limit
  bool BelowUpperLimit(const float &score = 0, const float &thr = 0);

  //  filter by lower limit
  bool AboveLowerLimit(const float &score = 0, const float &thr = 0);

  //  filter by rangge
  bool BetweenRange(const float &val = 0, const float &min = 0,
                    const float &max = 0);

  //  filter by age threshold
  bool ReachAgeThreshold(const int32_t &val = 0, const int32_t &min = 0,
                         const int32_t &max = 0, const float &score = 0);

  //  filter by face rect score
  bool PassPostVerification(const float &bbox_score, const float &pv_thr);

  //  filter by face landmark score
  bool LmkVerification(const std::shared_ptr<XStreamLandmarks> &lmk);

  //  if not in the snap area, return false
  bool IsWithinSnapArea(const float &x1, const float &y1, const float &x2,
                        const float &y2, const int &image_width,
                        const int &image_height);
  //  if not in the blacklist area, return false
  bool IsWithinBlackListArea(const float &x1, const float &y1, const float &x2,
                             const float &y2);
  //  if not in the whitelist area, return false
  bool IsWithinWhiteListArea(const float &x1, const float &y1, const float &x2,
                             const float &y2);
  //  filter by expand
  bool ExpandThreshold(hobot::vision::BBox *box, const float &expand_scale);

  BaseDataVectorPtr ConstructFilterOutputSlot0(const size_t &face_num,
                                               const size_t &head_num,
                                               const size_t &body_num,
                                               const size_t &hand_num);
  BaseDataVectorPtr ConstructBoundOutputSlot();

  BaseDataVectorPtr CopySlot(const BaseDataVectorPtr &attr,
                             const std::string &data_type);

 private:
  FilterParamPtr filter_param_ = nullptr;

  void Copy2Output(const std::vector<BaseDataVectorPtr> &input_slot,
                   std::vector<BaseDataVectorPtr> *p_output_slot,
                   uint32_t face_num, uint32_t head_num, uint32_t body_num,
                   uint32_t hand_num, bool pass_through);

  void LostIdProcess(const int input_size,
                     const std::vector<BaseDataVectorPtr> &input_slot,
                     const std::vector<BaseDataVectorPtr> &output_slot);

  int FaceValid(const int &input_size,
                const std::vector<BaseDataVectorPtr> &input_slot, int idx);

  int HeadValid(const int &input_size,
                const std::vector<BaseDataVectorPtr> &input_slot, int idx);

  int BodyValid(const int &input_size,
                const std::vector<BaseDataVectorPtr> &input_slot, int idx);
  int HandValid(const int &input_size,
                const std::vector<BaseDataVectorPtr> &input_slot, int idx);

  void ProcessOneBatch(const std::vector<BaseDataPtr> &batch_i,
                       std::vector<BaseDataPtr> *p_frame_output,
                       const std::shared_ptr<InputParam> &param_i);

  void FaceCountFilter(int input_size, int face_size,
                       std::vector<BaseDataVectorPtr> *p_output_slot) const;

  int GetIdByIdx(const int &input_size,
                 const std::vector<BaseDataVectorPtr> &input_slot,
                 const std::string &data_group, const int &box_idx);

  void ConstructState(const int &error_code, const int &state_flag,
                      const int &box_id);

  void CommonFilter(const int &input_size, const int &box_size,
                    const std::string &data_group,
                    const std::vector<BaseDataVectorPtr> &input_slot,
                    const std::vector<BaseDataVectorPtr> &output_slot);

  int NormalizeRoi(hobot::vision::BBox *src, float norm_ratio,
                   NormMethod norm_method, uint32_t total_w, uint32_t total_h);

  float GetUpperLimitVal(const std::string &name);

  float GetLowerLimitVal(const std::string &name);

  std::pair<float, float> GetRangeVal(const std::string &name);

  int GetErrCode(const std::string &name);

  template <typename T>
  T ValueNorm(const T &min, const T &max, const T &val) {
    if (val > max) return max;
    if (val < min) return min;
    return val;
  }
  std::map<int32_t, FilterStatePtr> filter_states_;
};
}  // namespace xstream
#endif  // FilterMethod_FilterMethod_H_
