/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      xroc.cpp
 * @brief     main function
 * @author    Zhuoran Rong (zhuoran.rong@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-02-26
 */

#include "pybind11/attr.h"
#include "pybind11/functional.h"
#include "pybind11/pybind11.h"

#include "base_data_warp.h" // NOLINT
#include "hobotxsdk/xstream_data.h"
#include "hobotxstream/xstream.h"
#include "hobotxstream/method_factory.h"

#include <iostream> // NOLINT
#include <thread>   // NOLINT

namespace xstream {

namespace py = pybind11;

using xstream::BaseData;
using xstream::BaseDataPtr;
using xstream::InputData;
using xstream::InputDataPtr;

// xroc sdk
class XStreamSDKWrapper {
 public:
  // 构造函数
  XStreamSDKWrapper() {
    std::cout << "XStreamSDKWrapper construct" << std::endl;
    if (!PyEval_ThreadsInitialized()) {
      PyEval_InitThreads();
    }
    xstream_sdk_ = XStreamSDK::CreateSDK();
  }

  ~XStreamSDKWrapper() {
    std::cout << "XStreamSDKWrapper desconstruct" << std::endl;
    delete xstream_sdk_;
  }

  // 初始化
  int SDKInit(const std::string &path) {
    int ret = 0;
    if (is_init_) {
      return 0;
    }

    xstream_sdk_->SetConfig("config_file", path);

    if ((ret = xstream_sdk_->Init()) != 0) {
      return ret;
    }

    is_init_ = true;
    return 0;
  }

  // 设置回调
  int Callback(py::function cb, const std::string &name = "") {
    // 把OutputData结构解析为一组BaseData, 返回一个BaseData的tuple
    return xstream_sdk_->SetCallback(
        [=](OutputDataPtr output) {
          // 获取 GIL 锁, 由于是异步回调,
          // pybind11没有保证异步回调函数会自动获取GIL
          // 获取GIL锁之后可以调用python接口
          py::gil_scoped_acquire acquire;
          py::tuple return_tuple(output->datas_.size());
          for (size_t i = 0; i < return_tuple.size(); ++i) {
            BaseDataWrapper item(output->datas_[i]);
            return_tuple[i] = py::cast(item);
          }

          // 回调到python 展开tuple
          cb(*return_tuple);
          // XXX 增加异常处理
        },
        name);
  }

  // async predict
  // 输入为一系列的BaseData
  // 输入顺序为workflow的inputs顺序
  int AsyncPredict(py::kwargs kwargs) {
    // 创建input data
    InputDataPtr inputdata(new InputData());

    if (kwargs.size() == 0) {
      return 0;
    }

    // 构建workflow的输入参数
    for (auto item : kwargs) {
      std::string item_name = item.first.cast<std::string>();
      BaseDataWrapper *item_data = item.second.cast<BaseDataWrapper *>();
      item_data->base_data_->name_ = item_name;
      inputdata->datas_.push_back(item_data->base_data_);
    }

    // 释放GIL锁,释放之后,C代码里不可调用任何Python的接口
    py::gil_scoped_release release;
    return static_cast<int>(xstream_sdk_->AsyncPredict(inputdata));
  }

 protected:
  class XStreamSDK *xstream_sdk_{nullptr};
  bool is_init_{false};
};

// 定义xstream internal package
PYBIND11_MODULE(xstream_internal, m) {
  // optional module docstring
  m.doc() = "xstream sdk internal implemention";

  // 数据状态
  py::enum_<DataState>(m, "DataState", py::arithmetic())
      .value("Valid", DataState::VALID)
      .value("Filtered", DataState::FILTERED)
      .value("Invisible", DataState::INVISIBLE)
      .value("Disappeared", DataState::DISAPPEARED)
      .value("Invalid", DataState::INVALID);

  // 定义基础数据类型
  py::class_<BaseDataWrapper, BaseDataWrapperPtr>(m, "BaseData")
      .def_property_readonly("state", &BaseDataWrapper::get_state);

  // 定义XStream SDK框架类
  py::class_<XStreamSDKWrapper>(m, "XStreamSDK")
      .def(py::init<>(), "Create XStream SDK instance.")
      .def("init", &XStreamSDKWrapper::SDKInit,
           "Set workflow description file.")
      .def("set_callback", &XStreamSDKWrapper::Callback, py::keep_alive<1, 2>(),
           py::keep_alive<1, 3>(), "Set workflow callback.")
      .def("async_predict", &XStreamSDKWrapper::AsyncPredict, "Async Predict");
}
}  // namespace HobotXStream
