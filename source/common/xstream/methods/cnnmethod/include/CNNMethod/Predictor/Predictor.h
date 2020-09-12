/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: Predictor.h
 * @Brief: declaration of the Predictor
 * @Author: zhengzheng.ge
 * @Email: zhengzheng.ge@horizon.ai
 * @Date: 2019-07-16 14:52:31
 * @Last Modified by: zhengzheng.ge
 * @Last Modified time: 2019-07-16 16:22:44
 */

#ifndef INCLUDE_CNNMETHOD_PREDICTOR_PREDICTOR_H_
#define INCLUDE_CNNMETHOD_PREDICTOR_PREDICTOR_H_

#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "CNNMethod/CNNConst.h"
#include "CNNMethod/util/CNNMethodConfig.h"
#include "CNNMethod/util/CNNMethodData.h"
#include "CNNMethod/util/ModelInfo.h"
#include "bpu_predict/bpu_io.h"
#include "bpu_predict/bpu_predict.h"
#include "bpu_predict/bpu_predict_extension.h"
#include "hobot_vision/bpu_model_manager.hpp"
#include "horizon/vision_type/vision_type.hpp"
#include "horizon/vision_type/vision_type_common.h"

namespace xstream {

struct NormParams {
  NormMethod norm_type;
  float expand_scale;
  float aspect_ratio;
};

class Predictor {
 public:
  Predictor() {}
  virtual ~Predictor() {
    Finalize();
  }

  virtual int32_t Init(std::shared_ptr<CNNMethodConfig> config);
  virtual void Finalize();
  virtual void Do(CNNMethodRunData *run_data) = 0;
  virtual void UpdateParam(std::shared_ptr<CNNMethodConfig> config);
  virtual std::string GetVersion() const {
    return model_version_;
  }

 protected:
  int RunModelWithBBox(hobot::vision::PymImageFrame &pym_image,
                       BPU_BBOX *box, int box_num, int *resizable_cnt);

  int NormalizeRoi(hobot::vision::BBox *src, hobot::vision::BBox *dst,
                   NormParams norm_params, uint32_t total_w, uint32_t total_h,
                   FilterMethod filter_method = FilterMethod::OUT_OF_RANGE);

  void PrepareInputTensor(uint8_t *img_data,
                          int data_length,
                          BPU_DATA_TYPE_E data_type,
                          bool is_padding);

  void PrepareOutputTensor();
  void PrepareOutputTensorBatch(int nBox);

  void ReleaseInputTensor();

  void ReleaseOutputTensor();

  int RunModel(uint8_t *img_data,
               int data_length,
               BPU_DATA_TYPE_E data_type,
               bool is_padding = false);

  void ConvertOutputToMXNet(void *src_ptr, void *dest_ptr, int out_index);

 protected:
  std::string model_name_;
  std::string model_version_;

  BPU_MODEL_S* bpu_model_ = nullptr;
  ModelInfo model_info_;
  int data_type_ = -1;
  std::vector<BPU_TENSOR_S> input_tensors_;
  bool input_tensors_alloced_ = false;
  std::vector<BPU_TENSOR_S> output_tensors_;
  bool output_tensors_alloced_ = false;
  int batch_size_ = 0;

  int is_plate_num_;
  std::string model_path_;
  int32_t max_handle_num_ = -1;  // Less than 0 means unlimited

//  float aspect_ratio_;
  NormParams norm_params_;
  std::string post_fn_;
  int core_id_ = 2;

 private:
  int FilterRoi(hobot::vision::BBox *src, hobot::vision::BBox *dst, int src_w,
                int src_h, FilterMethod filter_method);
};
}  // namespace xstream
#endif  // INCLUDE_CNNMETHOD_PREDICTOR_PREDICTOR_H_
