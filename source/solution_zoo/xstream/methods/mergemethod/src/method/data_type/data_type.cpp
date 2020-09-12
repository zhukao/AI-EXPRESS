/*
 * @Description: implement of data_type
 * @Author: chao.yang@horizon.ai
 * @Date: 2019-11-15 17:49:26
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-11-20 11:55:37
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */

#include "MergeMethod/data_type/data_type.h"

namespace xstream {

int MergeParam::UpdateParameter(const JsonReaderPtr &reader) {
  LOGD << "MergeParam update config: " << this;
  if (reader) {
    reader_ = reader;
    SET_SNAPSHOT_METHOD_PARAM(reader_->GetRawJson(), Double, match_threshold);
    LOGD << "match_threshold: " << match_threshold;
    SET_SNAPSHOT_METHOD_PARAM(reader_->GetRawJson(), Int,
                              filtered_box_state_type);
    LOGD << "filtered_box_state_type: " << filtered_box_state_type;
    return kHorizonVisionSuccess;
  } else {
    LOGE << "The reader is null";
    return kHorizonVisionErrorParam;
  }
}

std::string MergeParam::Format() { return reader_->GetJson(); }

}  // namespace xstream
