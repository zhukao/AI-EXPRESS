/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2020-01-19 04:59:48
 * @Version: v0.0.1
 * @Brief: CNN Predictore Output datastructure.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2020-01-19 05:33:28
 */
#ifndef CNNMETHOD_REFACTOR_PREDICTOR_CNNPREDICTOROUTPUTDATA_H_
#define CNNMETHOD_REFACTOR_PREDICTOR_CNNPREDICTOROUTPUTDATA_H_

#include <vector>
#include <string>
#include <memory>
#include "hobotxsdk/xstream_data.h"
#include "bpu_predict/bpu_predict.h"
#include "bpu_predict/bpu_io.h"
#include "horizon/vision_type/vision_type.hpp"

namespace xstream {
namespace CnnProc {

class ModelInfo;
class ModelOutputBuffer;

enum class InputType {
  PYRAMID,
  FAKEIMAGE
};

struct CNNPredictorOutputData : public BaseData {
  const ModelInfo *md_info;
  std::string model_name_;
  // 每帧的target的数量。
  std::vector<int> target_nums;
  std::vector<std::vector<BaseDataPtr>> input;
  // 每帧的合法目标index,
  // 第一维表示frame，　目前恒等于１.
  // 第二个维度valid target数量，
  // 取值范围为[0, target_num - 1],
  // target_num为该次cnn总共的输入目标数，
  // 比如输入2个track,　第一个track有３个人脸抠图,
  // 第二个track有2个人脸抠图，target_num = 3+2=5，
  // 如果有一个track非法（比如第一个track的第２个抠图非法）,
  // 第二个维度就为[0,2,3,4].
  std::vector<std::vector<int>> valid_targets;

  // BPU & Model info
  BPUHandle bpu_handle_;
  // bpu　pred　handle，　每个BPUModelHandle表示对BPU一次任务请求；
  // 第一维为frame, 目前恒等于１;
  // ModelOutputBuffer表示每次pred返回的结果，
  // bpu　pred有接口支持batch模式，
  // 也就是ModleOutputBuffer是个数组，可能同时返回多个目标的结果.
  // 要求每帧:
  // (BPUModelHandle num) * (ModelOutputBuffer size) = (valid_targets size).
  std::vector<std::vector<BPUModelHandle>> model_handle_;
  std::vector<std::vector<ModelOutputBuffer>> out_bufs_;

  InputType input_type{InputType::PYRAMID};
  // 如果输入类型为FAKEIMAGE，　以下两个字段才有意义.
  BPUFakeImageHandle bpu_fakeimage_handle_;
  // 要求: (BPUFakeInmage size) = (BPUModelHandle　size);
  std::vector<std::vector<BPUFakeImage>> fake_image_ptr_;

  // 该数据表示每个target的自定义数据，后处理有可能使用到，
  // 比如前处理如果做了norm外扩，可以通过该数据结构把外扩后的框
  // 传递到后处理;
  // 第一维为frame, 目前恒等于１;
  // 第二维size = target_num,如果某目标invalid，　BaseDataPtr=nullptr.
  std::vector<std::vector<BaseDataPtr>> targets_data;
};

class ModelInfo {
 public:
  void Init(BPUHandle bpuHandler,
            std::string model_name,
            BPUModelInfo *input_model,
            BPUModelInfo *output_model);
  friend std::ostream &operator<<(std::ostream &o, const ModelInfo &info);

 public:
  std::string model_name_;
  std::string model_file_path_;

  // output info
  std::vector<int> output_layer_size_;               // include sizeof(ele)
  std::vector<int> mxnet_output_layer_size_;         // include sizeof(ele)
  std::vector<std::vector<uint32_t>> aligned_nhwc_;  // bpu output nhwc
  std::vector<std::vector<uint32_t>> real_nhwc_;     // mxnet output nhwc
  std::vector<uint32_t> elem_size_;                  // ConvertOutputToMXNet使用
  std::vector<std::vector<uint32_t>> all_shift_;     // ConvertOutputToMXNet使用
  // input info
  std::vector<int> input_nhwc_;
};

class ModelOutputBuffer {
 public:
  ModelOutputBuffer() = default;
  ModelOutputBuffer(const ModelInfo &model_info, int target_num) {
    for (int i = 0; i < target_num; ++i) {
      for (std::size_t j = 0; j < model_info.output_layer_size_.size(); ++j) {
        BPU_Buffer_Handle out_handle = BPU_createEmptyBPUBuffer();
        out_bufs_.push_back(out_handle);
      }
    }
  }
  ~ModelOutputBuffer() {
    for (auto &out_buf : out_bufs_) {
      BPU_freeBPUBuffer(out_buf);
    }
    out_bufs_.clear();
  }

  ModelOutputBuffer& operator=(const ModelOutputBuffer& src) {
    this->out_bufs_.assign(src.out_bufs_.begin(), src.out_bufs_.end());
    return *this;
  }
 public:
  std::vector<BPU_Buffer_Handle> out_bufs_;
};

}  // namespace CnnProc
}  // namespace xstream
#endif  // CNNMETHOD_REFACTOR_PREDICTOR_CNNPREDICTOROUTPUTDATA_H_
