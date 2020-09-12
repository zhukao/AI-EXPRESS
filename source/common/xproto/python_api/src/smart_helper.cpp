/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      smart_helper.cpp
 * @brief     C++/Python data conversion
 * @author    Shiyu Fu (shiyu.fu@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-03-9
 */

#include <iostream>
#include "smart_helper.h"   // NOLINT
#include "horizon/vision/util.h"

using xstream::BaseData;
using xstream::BaseDataPtr;
using xstream::DataState;
using xstream::OutputDataPtr;
using xstream::BaseDataWrapper;
using ImageFramePtr = std::shared_ptr<hobot::vision::ImageFrame>;
using XStreamImageFramePtr = xstream::XStreamData<ImageFramePtr>;

namespace horizon {
namespace vision {
namespace xproto {

namespace py = pybind11;

std::string PySmartMsg::Serialize() {
  if (py_serialize != nullptr) {
    auto smart_rets = new py::tuple(smart_result->datas_.size());
    for (size_t idx = 0; idx < smart_result->datas_.size(); ++idx) {
      auto data = smart_result->datas_[idx];
      BaseDataWrapper data_warpper =
          BaseDataWrapper(data);
      (*smart_rets)[idx] = data;
    }
    std::string ret = py_serialize((*smart_rets));
    return ret;
  }
  return "Python serialization not given";
}

SmartPlgHelper::SmartPlgHelper() {
  monitor_.reset(new smartplugin::RuntimeMonitor());
}

py::dict SmartPlgHelper::ToXStreamData(py::object msg) {
  XProtoMessagePtr flow_msg = msg.cast<XProtoMessagePtr>();
  auto vio_msg =
    std::static_pointer_cast<basic_msgtype::VioMessage>(flow_msg);
  auto vio_msg_ptr = vio_msg.get();
  if (!is_sync_mode_) {
    smartplugin::SmartInput *input_wrapper = new smartplugin::SmartInput();
    input_wrapper->frame_info = vio_msg;
    input_wrapper->context = input_wrapper;
    monitor_->PushFrame(input_wrapper);
  }
  // build xstream data
  auto xstream_inputs_dict = new py::dict();
  for (uint32_t idx = 0; idx < 1; ++idx) {
    BaseDataPtr xstream_input;
    if (vio_msg_ptr->num_ > idx) {
      auto xstream_img =
        horizon::vision::util::ImageFrameConversion(vio_msg_ptr->image_[idx]);
      xstream_input = xstream::BaseDataPtr(xstream_img);
      std::cout << "ToXStreamData: Input Frame ID = "
                << xstream_img->value->frame_id
                << ", Timestamp = " << xstream_img->value->time_stamp
                << std::endl;
    } else {
      xstream_input = std::make_shared<BaseData>();
      xstream_input->state_ = DataState::INVALID;
    }

    if (idx == uint32_t{0}) {
      if (vio_msg->num_ == 1) {
        xstream_input->name_ = "image";
      } else {
        xstream_input->name_ = "rgb_image";
      }
    } else {
      xstream_input->name_ = "nir_image";
    }
    (*xstream_inputs_dict)[py::str(xstream_input->name_)] =
      new BaseDataWrapper(xstream_input);
  }
  return *xstream_inputs_dict;
}

XProtoMessagePtr SmartPlgHelper::ToNativeFlowMsg(py::args smart_rets) {
  OutputDataPtr outputdata(new xstream::OutputData());
  XStreamImageFramePtr *rgb_image = nullptr;
  // xstream BaseDataWrapper to OutputData
  for (auto smart_ret : smart_rets) {
    BaseDataWrapper *bd_warpper = smart_ret.cast<BaseDataWrapper *>();
    BaseDataPtr basedata = bd_warpper->base_data_;
    if (basedata->name_ == "rgb_image" || basedata->name_ == "image") {
      rgb_image = dynamic_cast<XStreamImageFramePtr *>(basedata.get());
    }
    outputdata->datas_.push_back(basedata);
  }
  // create custome smart message
  auto smart_msg =
    std::make_shared<smartplugin::CustomSmartMessage>(outputdata);
  // smart_msg->py_serialize = serialize;
  if (rgb_image != nullptr && rgb_image->value != nullptr) {
    smart_msg->time_stamp = rgb_image->value->time_stamp;
    smart_msg->frame_id = rgb_image->value->frame_id;
    // free vio message
    auto input = monitor_->PopFrame(smart_msg->frame_id);
    delete static_cast<smartplugin::SmartInput *>(input.context);
    std::cout << "SmartHelper: delete SmartInput with frame_id: "
              << smart_msg->frame_id << std::endl;
  }
  return smart_msg;
}

}   // namespace xproto
}   // namespace vision
}   // namespace horizon
