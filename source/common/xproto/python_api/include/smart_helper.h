/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      smart_helper.h
 * @author    Shiyu Fu (shiyu.fu@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-02-26
 */

#include <string>
#include <memory>
#include "base_data_warp.h"
#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto_msgtype/vioplugin_data.h"
#include "xproto_msgtype/smartplugin_data.h"
#include "smartplugin/smartplugin.h"
#include "smartplugin/runtime_monitor.h"
#include "pybind11/pybind11.h"

namespace horizon {
namespace vision {
namespace xproto {

namespace py = pybind11;

struct PySmartMsg : basic_msgtype::SmartMessage {
  explicit PySmartMsg(
    xstream::OutputDataPtr out) : smart_result(out) {
    type_ = TYPE_SMART_MESSAGE;
  }
  std::string Serialize() override;

 private:
  xstream::OutputDataPtr smart_result;
  std::function<py::str(py::args)> py_serialize;
};

class SmartPlgHelper {
 public:
  SmartPlgHelper();
  py::dict ToXStreamData(py::object msg);
  XProtoMessagePtr ToNativeFlowMsg(py::args smart_rets);
  // XProtoMessagePtr ToNativeFlowMsg(py::args smart_rets,
  //               std::function<py::str(py::args)> serialize);
  void SetMode(bool sync_mode) { is_sync_mode_ = sync_mode; }

 private:
  std::shared_ptr<smartplugin::RuntimeMonitor> monitor_;
  bool is_sync_mode_ = false;
};

}   // namespace xproto
}   // namespace vision
}   // namespace horizon
