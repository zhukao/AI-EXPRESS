/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xstream framework C interface
 * @author    shuhuan.sun
 * @email     shuhuan.sun@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.4
 */

#ifndef HOBOTXSDK_XSTREAM_CAPI_TYPE_HELPER_H_
#define HOBOTXSDK_XSTREAM_CAPI_TYPE_HELPER_H_

#include <functional>
#include <memory>
#include <string>

#include "hobotxsdk/xstream_capi_type.h"
#include "hobotxsdk/xstream_data.h"

/**
 * @brief 下面的宏定义在结构体实现源文件中使用
 *
 */
#define XSTREAM_DEFINE_DATA_FREE_PARENT(type)              \
  inline void HobotXStreamCapi##type##FreeParent(          \
      HobotXStreamCapiData** data) {                       \
    HobotXStreamCapi##type##Free(                          \
        reinterpret_cast<HobotXStreamCapi##type**>(data)); \
  }

/**
 * @brief 此宏在cpp构造函数中使用
 *
 */
#define XSTREAM_BASE_CPP_CONTEXT_INIT(type, context) \
  auto& ccontext = context;                          \
  ccontext->cpp2c_method_ = xstream::type##Cpp2CBaseData;

/**
 * @brief 在c结构体alloc函数中使用
 *
 */
#define XSTREAM_CAPI_BASE_ALLOC(type, out)                            \
  HobotXStreamCapi##type* out = static_cast<HobotXStreamCapi##type*>( \
      std::calloc(1, sizeof(HobotXStreamCapi##type)));                \
  auto& base = out->parent_;                                          \
  base.type_ = #type;                                                 \
  auto context = new xstream::CppContext();                           \
  context->c2cpp_method_ = xstream::type##C2CppBaseData;              \
  context->cfree_method_ = HobotXStreamCapi##type##FreeParent;        \
  base.context_ = context;

/**
 * @brief 在c结构体free函数中使用
 *
 */
#define XSTREAM_CAPI_BASE_FREE(type, ppdata)                          \
  auto context =                                                      \
      static_cast<xstream::CppContext*>((*ppdata)->parent_.context_); \
  delete context;

/**
 * @brief 定义transfer函数的命名规则，在transfer头文件中使用
 *  注意：在namespace HobotXStream下使用
 */
#define XSTREAM_DECLEAR_TRANSFER(type)                                   \
  HOBOT_EXPORT                                                           \
  HobotXStreamCapi##type* type##Cpp2C(std::shared_ptr<type> cpp_data);   \
  HOBOT_EXPORT                                                           \
  std::shared_ptr<type> type##C2Cpp(HobotXStreamCapi##type* c_data);     \
  inline std::shared_ptr<BaseData> type##C2CppBaseData(                  \
      HobotXStreamCapiData* c) {                                         \
    return std::static_pointer_cast<BaseData>(                           \
        type##C2Cpp(reinterpret_cast<HobotXStreamCapi##type*>(c)));      \
  }                                                                      \
  inline HobotXStreamCapiData* type##Cpp2CBaseData(                      \
      std::shared_ptr<BaseData> cpp) {                                   \
    return &(type##Cpp2C(std::static_pointer_cast<type>(cpp))->parent_); \
  }

/**
 * @brief 定义 to c 结构体转换函数的函数命名
 *
 */
#define XSTREAM_DEFINE_2C_TRANSFER_FUNC(type, in_data) \
  HobotXStreamCapi##type* type##Cpp2C(std::shared_ptr<type> in_data)

/**
 * @brief 在 to c 结构体转换函数中使用，处理 BaseData 里面的数据
 *
 */
#define XSTREAM_BASE_2C_TRANSFER_PROCESS(type, in_data, out_data, ...)      \
  HobotXStreamCapi##type* out_data =                                        \
      HobotXStreamCapi##type##Alloc(__VA_ARGS__);                           \
  auto context = reinterpret_cast<CppContext*>(out_data->parent_.context_); \
  context->cpp_data_ = in_data;                                             \
  BaseData2C(in_data.get(), &out_data->parent_);

/**
 * @brief 定义 to cpp 结构体转换函数的函数命名
 *
 */
#define XSTREAM_DEFINE_2CPP_TRANSFER_FUNC(type, in_data) \
  std::shared_ptr<type> type##C2Cpp(HobotXStreamCapi##type* in_data)

/**
 * @brief 在 to cpp 结构体转换函数中使用，处理 BaseData 里面的数据
 *
 */
#define XSTREAM_BASE_2CPP_TRANSFER_PROCESS(type, in_data, out_data, ...) \
  auto out_data = std::make_shared<type>(__VA_ARGS__);                   \
  BaseData2Cpp(&(in_data->parent_), out_data.get());

namespace xstream {

class CppContext {
 public:
  std::shared_ptr<BaseData> cpp_data_;
  std::function<std::shared_ptr<BaseData>(HobotXStreamCapiData*)> c2cpp_method_;
  std::function<void(HobotXStreamCapiData**)> cfree_method_;
};

class CContext {
 public:
  std::function<HobotXStreamCapiData*(std::shared_ptr<BaseData>)> cpp2c_method_;
};

inline void BaseData2C(BaseData* cpp, HobotXStreamCapiData* c) {
  if (cpp->type_[0])
    c->type_ = &cpp->type_[0];  // 不用赋值，因为c接口alloc时就已经赋值
  if (cpp->name_[0]) {
    c->name_ = &cpp->name_[0];
  }
  if (cpp->error_code_) {
    c->error_code_ = cpp->error_code_;
  }
  if (cpp->error_detail_[0]) {
    c->error_detail_ = &cpp->error_detail_[0];
  }
}

inline void BaseData2Cpp(HobotXStreamCapiData* c, BaseData* cpp) {
  if (c->type_)
    cpp->type_ = std::string(c->type_);  // 不用赋值，因为cpp构造函数中已经赋值
  if (c->name_) {
    cpp->name_ = std::string(c->name_);
  }
  if (c->error_code_) {
    cpp->error_code_ = c->error_code_;
  }
  if (c->error_detail_) {
    cpp->error_detail_ = std::string(c->error_detail_);
  }
}

inline InputDataPtr InputList2Cpp(const HobotXStreamCapiInputList* inputs) {
  InputDataPtr cpp(new xstream::InputData());
  cpp->datas_.resize(inputs->datas_size_);
  for (size_t i = 0; i < inputs->datas_size_; ++i) {
    auto i_c_data = inputs->datas_[i];
    auto c2cpp_func =
        reinterpret_cast<CppContext*>(i_c_data->context_)->c2cpp_method_;
    cpp->datas_[i] =
        c2cpp_func(reinterpret_cast<HobotXStreamCapiData*>(i_c_data));
  }
  cpp->context_ = inputs;
  return cpp;
}

inline HobotXStreamCapiDataList* Output2CList(OutputDataPtr outputs) {
  auto c_out = HobotXStreamCapiDataListAlloc(outputs->datas_.size());
  for (size_t i = 0; i < outputs->datas_.size(); ++i) {
    auto& i_cpp_data = outputs->datas_[i];
    auto cpp2c_func = (i_cpp_data->c_data_)->cpp2c_method_;
    c_out->datas_[i] = cpp2c_func(i_cpp_data);
  }
  return c_out;
}

inline void HobotXStreamCapiBaseDataFree(HobotXStreamCapiData** data) {
  if (data && *data) {
    auto context = reinterpret_cast<xstream::CppContext*>((*data)->context_);
    delete context;
    std::free(*data);
  }
}

inline std::shared_ptr<BaseData> BaseDataC2CppBaseData(
    HobotXStreamCapiData* c) {
  std::shared_ptr<BaseData> cpp_data(new BaseData());
  BaseData2Cpp(c, cpp_data.get());
  return cpp_data;
}

inline HobotXStreamCapiData* BaseDataCpp2CBaseData(
    std::shared_ptr<BaseData> cpp) {
  auto c_data = static_cast<HobotXStreamCapiBaseData*>(
      std::calloc(1, sizeof(HobotXStreamCapiBaseData)));
  c_data->type_ = "BaseData";
  auto context = new xstream::CppContext();
  context->c2cpp_method_ = xstream::BaseDataC2CppBaseData;
  context->cfree_method_ = HobotXStreamCapiBaseDataFree;
  c_data->context_ = context;
  BaseData2C(cpp.get(), c_data);
  return c_data;
}

XSTREAM_DECLEAR_TRANSFER(BaseDataVector);

}  // namespace xstream

#endif  // HOBOTXSDK_XSTREAM_CAPI_TYPE_HELPER_H_
