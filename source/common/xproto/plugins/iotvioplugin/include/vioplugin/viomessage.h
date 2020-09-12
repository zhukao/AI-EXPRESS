/*
 * @Description: implement of vioplugin
 * @Author: fei.cheng@horizon.ai
 * @Date: 2019-08-26 16:17:25
 * @Author: songshan.gong@horizon.ai
 * @Date: 2019-09-26 16:17:25
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-10-16 16:27:39
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */
#ifndef INCLUDE_VIOMESSAGE_VIOMESSAGE_H_
#define INCLUDE_VIOMESSAGE_VIOMESSAGE_H_

#include <memory>
#include <vector>
#include <string>
#if defined(X3_X2_VIO) || defined(X3_IOT_VIO)
#include "./hb_cam_interface.h"
#include "./hb_vio_interface.h"
#include "./x3_vio_patch.h"
#endif
#if defined(X3_IOT_VIO)
#include "./hb_vio_interface.h"
#include "./iot_vio_api.h"
#endif

#include "hobot_vision/blocking_queue.hpp"

#include "xproto_msgtype/vioplugin_data.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace vioplugin {
using hobot::vision::PymImageFrame;
using horizon::vision::xproto::basic_msgtype::VioMessage;

struct ImageVioMessage : VioMessage {
 public:
  ImageVioMessage() = delete;
  explicit ImageVioMessage(
      std::vector<std::shared_ptr<PymImageFrame>> &image_frame,
      uint32_t img_num, bool is_valid = true);
  ~ImageVioMessage();

  // serialize proto
  std::string Serialize() { return "No need serialize"; }

  void FreeImage();
  void FreeImage(int tmp);  // 用于释放x3临时回灌功能的接口
};

struct DropVioMessage : VioMessage {
 public:
  DropVioMessage() = delete;
  explicit DropVioMessage(uint64_t timestamp, uint64_t seq_id);
  ~DropVioMessage(){};

  // serialize proto
  std::string Serialize() override;
};

struct DropImageVioMessage : VioMessage {
 public:
  DropImageVioMessage() = delete;
  explicit DropImageVioMessage(
      std::vector<std::shared_ptr<PymImageFrame>> &image_frame,
      uint32_t img_num, bool is_valid = true);
  ~DropImageVioMessage();

  // serialize proto
  std::string Serialize() { return "No need serialize"; }

  void FreeImage();
};

}  // namespace vioplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon

#endif
