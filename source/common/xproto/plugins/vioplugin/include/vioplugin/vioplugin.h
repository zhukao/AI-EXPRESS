/*
 * @Description: implement of vioplugin
 * @Author: fei.cheng@horizon.ai
 * @Date: 2019-08-26 16:17:25
 * @Author: songshan.gong@horizon.ai
 * @Date: 2019-09-26 16:17:25
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-10-15 11:14:07
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */
#ifndef INCLUDE_VIOPLUGIN_VIOPLUGIN_H_
#define INCLUDE_VIOPLUGIN_VIOPLUGIN_H_

#include <string>
#include <unordered_map>
#include "json/json.h"
#include "vioplugin/vioproduce.h"
#include "xproto/plugin/xpluginasync.h"

#include "horizon/vision_type/vision_type.hpp"
#include "hobot_vision/blocking_queue.hpp"
#ifdef PYAPI
#include "pybind11/pybind11.h"
#endif

#define VIO_CAMERA "vio_camera"
#define VIO_FEEDBACK "vio_feedback"

namespace horizon {
namespace vision {
namespace xproto {
namespace vioplugin {

using box_t = hobot::vision::BBox_<uint32_t>;

class VioPlugin : public xproto::XPluginAsync {
 public:
  VioPlugin() = delete;
  explicit VioPlugin(const std::string &path);
  ~VioPlugin() override;
  int Init() override;
  int Start() override;
  int Stop() override;
  std::string desc() const { return "VioPlugin"; }
  bool IsInited() { return is_inited_; }
  XProtoMessagePtr GetImage();
  void ClearAllQueue();
  int SetMode(bool is_sync_mode) {
    is_sync_mode_ = is_sync_mode;
    return 0;
  }
#ifdef PYAPI
  int AddMsgCB(const std::string msg_type, pybind11::function callback);
#endif

 private:
  VioConfig *GetConfigFromFile(const std::string &path);
  int OnGetHbipcResult(const XProtoMessagePtr msg);

 private:
  VioConfig *config_;
  std::shared_ptr<VioProduce> VioProduceHandle_;
  std::vector<box_t> Shields_;
  bool is_inited_ = false;
  bool is_sync_mode_ = false;
  hobot::vision::BlockingQueue<XProtoMessagePtr> img_msg_queue_;
  hobot::vision::BlockingQueue<XProtoMessagePtr> drop_msg_queue_;
  std::unordered_map<std::string, XProtoMessageFunc> message_cb_;
};

}  // namespace vioplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
#endif
