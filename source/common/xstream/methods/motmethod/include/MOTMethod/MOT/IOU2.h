/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @brief     IOU2_MOT header
 * @author    dezhi.zeng
 * @email     dezhi.zeng@horizon.ai
 * @version   0.0.0.1
 * @date      2019.11.18
 */

#ifndef INCLUDE_MOTMETHOD_MOT_IOU2_H_
#define INCLUDE_MOTMETHOD_MOT_IOU2_H_

#include <string>
#include <vector>
#include <memory>
#include <map>

#include "MOTMethod/MOTMethod.h"
#include "MOTMethod/error_code.h"
#include "iou_based_mot/target.h"
#include "iou_based_mot/tracker.h"


namespace xstream {

struct IOU2Param;

class IOU2 : public Mot {
 public:
  int MotInit(const std::string &config_file_path) override;

  int Track(const std::vector<BaseDataPtr> &in,
            std::vector<BaseDataPtr> &out) override;

  void MotFinalize() override;

 private:
  typedef std::shared_ptr<BaseDataVector> BaseDataVectorPtr;

  int SetTrackerConfig();

  void RectMsg2Box(std::vector<hobot::iou_mot::sp_BBox> *boxes,
                        BaseDataVector *rects_msg,
                        const int &img_width, const int &img_height);

  void track_to_rects(const time_t &time_stamp,
                const std::vector<hobot::iou_mot::sp_TrackLet> &tracklet_list,
                std::vector<BaseDataPtr> *p_out_rects,
                std::vector<BaseDataPtr> *p_disappeared_ids);

  void copy_inrects_to_out(BaseDataVector *rects_msg,
                std::vector<BaseDataPtr> *p_out_rects,
                const int &img_width, const int &img_height);

  std::shared_ptr<IOU2Param> GetConfig();

  int UpdateParameter(const std::string &content) override;

 private:
  std::shared_ptr<hobot::iou_mot::Tracker> tracker_;
};
}  // namespace xstream

#endif  // INCLUDE_MOTMETHOD_MOT_IOU2_H_
