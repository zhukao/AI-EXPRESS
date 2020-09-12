/* @Description: declaration of ipm image message
 * @Author: shiyu.fu@horizon.ai
 * @Date: 2020-09-08 15:27:06
 * @Last Modified by: shiyu.fu@horizon.ai
 * @Last Modified time: 2020-09-08 15:54:13
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 * */

#ifndef XPROTO_MSGTYPE_GDCPLUGIN_DATA_H_
#define XPROTO_MSGTYPE_GDCPLUGIN_DATA_H_

#include <memory>
#include <vector>
#include "horizon/vision_type/vision_error.h"
#include "horizon/vision_type/vision_msg.h"
#include "horizon/vision_type/vision_type.h"
#include "horizon/vision_type/vision_type.hpp"
#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/utils/profile.h"
#include "xproto_msgtype/vioplugin_data.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace basic_msgtype {

#define TYPE_IPM_MESSAGE "XPLUGIN_IPM_MESSAGE"

using hobot::vision::CVImageFrame;
using horizon::vision::xproto::basic_msgtype::VioMessage;

struct IpmImageMessage : VioMessage {
  explicit IpmImageMessage(std::vector<std::shared_ptr<CVImageFrame>> &imgs,
                           uint32_t img_num, std::vector<uint64_t> ttses,
                           std::vector<uint64_t> frame_ids,
                           std::vector<int> chn_ids,
                           bool is_valid = true) {
    HOBOT_CHECK(imgs.size() == ttses.size() &&
                ttses.size() == frame_ids.size() &&
                frame_ids.size() && chn_ids.size())
                << "size mismatch, img_num: " << img_num
                << ", imgs.size(): " << imgs.size()
                << ", ttses.size(): " << ttses.size()
                << ", frame_ids.size(): " << frame_ids.size()
                << ", chn_ids.size(): " << chn_ids.size();
    type_ = TYPE_IPM_MESSAGE;
    num_ = img_num;
    is_valid_uri_ = is_valid;
    ipm_imgs_.resize(img_num);
    channel_ids_.resize(img_num);
    frame_ids_.resize(img_num);
    time_stamps_.resize(img_num);
    for (size_t i = 0; i < img_num; ++i) {
      ipm_imgs_[i] = imgs[i];
      ipm_imgs_[i]->channel_id = chn_ids[i];
      ipm_imgs_[i]->time_stamp = ttses[i];
      ipm_imgs_[i]->frame_id = frame_ids[i];
      channel_ids_[i] = chn_ids[i];
      frame_ids_[i] = frame_ids[i];
      time_stamps_[i] = ttses[i];
    }
  }

  std::vector<std::shared_ptr<CVImageFrame>> ipm_imgs_;
  std::vector<int> channel_ids_;
  std::vector<uint64_t> frame_ids_;
  std::vector<uint64_t> time_stamps_;
};

}  // namespace basic_msgtype
}  // namespace xproto
}  // namespace vision
}  // namespace horizon

#endif  // XPROTO_MSGTYPE_GDCPLUGIN_DATA_H_
