/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      base_data_warp.h
 * @brief     main function
 * @author    Zhuoran Rong (zhuoran.rong@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-02-26
 */

#include "hobotxsdk/xstream_data.h"

#include <iostream>  // NOLINT
#include <memory>  // NOLINT

namespace xstream {
// 定义XStreamData包装类
class BaseDataWrapper : public std::enable_shared_from_this<BaseDataWrapper> {
 public:
  explicit BaseDataWrapper(BaseDataPtr data) : base_data_(data) {}
  ~BaseDataWrapper() {
    // std::cout << "BaseDataWapper Destoryed!" << std::endl;
  }

  // 获得当前basedata的状态
  DataState get_state(void) {
    if (base_data_ == nullptr) {
      return DataState::INVALID;
    } else {
      return base_data_->state_;
    }
  }

  BaseDataPtr base_data_{nullptr};
};

typedef std::shared_ptr<BaseDataWrapper> BaseDataWrapperPtr;
}  // namespace xstream
