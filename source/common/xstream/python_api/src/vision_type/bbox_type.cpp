/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      bbox_type.cpp
 * @brief     main function
 * @author    Zhuoran Rong (zhuoran.rong@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-02-26
 */

#include "pybind11/pybind11.h"

#include <cassert>  // NOLINT
#include <iostream>  // NOLINT
#include "method/bbox_filter/b_box.h"
#include "method/bbox_filter/b_box_filter.h"
#include "hobotxsdk/xstream_data.h"

#include "base_data_warp.h"  // NOLINT

namespace xstream {

namespace py = pybind11;

BaseDataWrapperPtr bbox_builder(float x1, float y1, float x2, float y2) {
    BaseDataVector *data(new BaseDataVector);
    xstream::BBox *bbox(
        new xstream::BBox(hobot::vision::BBox(x1, y1, x2, y2)));
    bbox->type_ = "BBox";
    data->datas_.push_back(BaseDataPtr(bbox));

    auto tmp = new BaseDataWrapper(BaseDataPtr(data));
    // std::cout << "base data warpper raw ptr:" << tmp << std::endl;

    return BaseDataWrapperPtr(tmp);
}

// 导出bbox数据结构
void bbox_dump(BaseDataWrapper *bbox) {
    BaseDataPtr data = bbox->base_data_;
    if (data->error_code_ < 0) {
        std::cout << "data error: " << data->error_code_ << std::endl;
        return;
    }
    std::cout << "data type_name : " << data->type_ << " " << data->name_
            << std::endl;

    BaseDataVector *pdata = reinterpret_cast<BaseDataVector *>(data.get());
    std::cout << "pdata size: " << pdata->datas_.size() << std::endl;
    std::cout << "Output BBox " << pdata->name_ << ":" << std::endl;
    for (size_t i = 0; i < pdata->datas_.size(); ++i) {
      auto xroc_box =
          std::static_pointer_cast<xstream::XStreamData<hobot::vision::BBox>>(
              pdata->datas_[i]);
      if (xroc_box->state_ == xstream::DataState::VALID) {
        std::cout << "[" << xroc_box->value.x1 << "," << xroc_box->value.y1
                  << "," << xroc_box->value.x2 << "," << xroc_box->value.y2
                  << "]" << std::endl;
        } else {
            printf("pdata->datas_[%d]: state_:%d\n",
                static_cast<int>(i),
                static_cast<int>(xroc_box->state_));
        }
    }
}

}  // namespace xstream
