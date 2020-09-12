/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      wrapper.cpp
 * @brief     native xproto wrapper
 * @author    Shiyu Fu (shiyu.fu@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-02-26
 */

#include "horizon/vision_type/vision_type.hpp"
#include "xproto/plugin/xplugin.h"
#include "xproto/plugin/xpluginasync.h"
#include "xproto/message/pluginflow/flowmsg.h"
// #include "xproto_msgtype/smartplugin_data.h"
#include "xproto_msgtype/vioplugin_data.h"
#include "vioplugin/vioplugin.h"
// #include "hbipcplugin/hbipcplugin.h"
#include "smartplugin/smartplugin.h"
#include "horizon/vision/util.h"
#include "smart_helper.h"     // NOLINT
#include "pybind11/pybind11.h"
#include "pybind11/functional.h"

using ImageFramePtr = std::shared_ptr<hobot::vision::ImageFrame>;
using XStreamImageFramePtr = xstream::XStreamData<ImageFramePtr>;
using xstream::BaseData;
using xstream::BaseDataPtr;
using xstream::DataState;
using xstream::OutputDataPtr;
using xstream::BaseDataWrapper;

namespace horizon {
namespace vision {
namespace xproto {

namespace py = pybind11;

class XPluginHelper : public XPlugin {
 public:
  using XPlugin::PushMsg;
};

PYBIND11_MODULE(native_xproto, m) {
  py::class_<XPlugin, std::shared_ptr<XPlugin>>(m, "XPlugin")
      .def("push_msg", &XPluginHelper::PushMsg);

  py::class_<XProtoMessage,
            std::shared_ptr<XProtoMessage>>(m, "XProtoMsg")
      .def_readwrite("type_", &XProtoMessage::type_)
      .def_readwrite("param_", &XProtoMessage::param_)
      .def("type", &XProtoMessage::type);

  py::class_<XPluginAsync, std::shared_ptr<XPluginAsync>>(m, "XPlgAsync")
      .def(py::init<>())
      .def(py::init<int>())
      .def("init", &XPluginAsync::Init)
      .def("register_msg", &XPluginAsync::RegMsg)
      .def("push_msg", &XPluginHelper::PushMsg);

  py::class_<vioplugin::VioPlugin,
            std::shared_ptr<vioplugin::VioPlugin>>(m, "NativeVioPlg")
      .def(py::init<const std::string &>())
      .def("init", &vioplugin::VioPlugin::Init)
      .def("deinit", &vioplugin::VioPlugin::DeInit)
      .def("start", &vioplugin::VioPlugin::Start)
      // release GIL in long-run C++ method
      .def("stop", &vioplugin::VioPlugin::Stop,
          py::call_guard<py::gil_scoped_release>())
      .def("is_inited", &vioplugin::VioPlugin::IsInited)
      .def("get_image", &vioplugin::VioPlugin::GetImage)
      .def("set_mode", &vioplugin::VioPlugin::SetMode)
      .def("add_msg_cb", &vioplugin::VioPlugin::AddMsgCB);

  // py::class_<hbipcplugin::HbipcPlugin,
  //           std::shared_ptr<hbipcplugin::HbipcPlugin>>(m, "NativeHbipcPlg")
  //     .def(py::init<const std::string &>())
  //     .def("init", &hbipcplugin::HbipcPlugin::Init)
  //     .def("deinit", &hbipcplugin::HbipcPlugin::Deinit)
  //     .def("start", &hbipcplugin::HbipcPlugin::Start)
  //     .def("stop", &hbipcplugin::HbipcPlugin::Stop);

  // py::class_<smartplugin::SmartPlugin,
  //           std::shared_ptr<smartplugin::SmartPlugin>>(m, "SmartPlg")
  //     .def(py::init<const std::string &>())
  //     .def("init", &smartplugin::SmartPlugin::Init);

  py::class_<SmartPlgHelper>(m, "SmartHelper")
      .def(py::init<>())
      .def("to_xstream_data", &SmartPlgHelper::ToXStreamData)
      .def("to_native_msg", &SmartPlgHelper::ToNativeFlowMsg)
      .def("set_mode", &SmartPlgHelper::SetMode);
}

}   // namespace xproto
}   // namespace vision
}   // namespace horizon
