/*
 * @Description: implement of  vio data header
 * @Author: fei.cheng@horizon.ai
 * @Date: 2019-10-14 16:35:21
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-10-16 16:11:58
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */

#ifndef XPROTO_MSGTYPE_RTSPPLUGIN_DATA_H_
#define XPROTO_MSGTYPE_RTSPPLUGIN_DATA_H_

#include <memory>
#include <vector>
#ifdef X2
#include "hb_vio_interface.h"
#endif
#include "horizon/vision_type/vision_error.h"
#include "horizon/vision_type/vision_msg.h"
#include "horizon/vision_type/vision_type.h"
#include "horizon/vision_type/vision_type.hpp"
#include "xproto/message/pluginflow/flowmsg.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace basic_msgtype {

#define TYPE_DECODE_IMAGE_MESSAGE "XPLUGIN_DECODE_IMAGE_MESSAGE"
#define TYPE_DECODE_DROP_MESSAGE "XPLUGIN_DECODE_DROP_MESSAGE"

struct RtspMessage : public XProtoMessage {
 public:
  RtspMessage() { type_ = TYPE_DECODE_IMAGE_MESSAGE; }
  virtual ~RtspMessage() = default;

  // channel number
  uint32_t chanel = 0;
  // image frames number
  uint32_t num_ = 0;
  // sequence id, would increment automatically
  uint64_t sequence_id_ = 0;
  // time stamp
  uint64_t time_stamp_ = 0;
  // is valid uri
  bool is_valid_uri_ = true;
  // free source image
  void FreeImage();
  // serialize proto
  std::string Serialize() override { return "Default vio message"; };
#ifdef X2
  // image frames
  HorizonVisionImageFrame **image_ = nullptr;
  // multi
  mult_img_info_t *multi_info_ = nullptr;
#endif

#ifdef X3
  std::vector<std::shared_ptr<hobot::vision::PymImageFrame>> image_;
#endif
};

}  // namespace basic_msgtype
}  // namespace xproto
}  // namespace vision
}  // namespace horizon

#endif  // XPROTO_MSGTYPE_VIOPLUGIN_DATA_H_
