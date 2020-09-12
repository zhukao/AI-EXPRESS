/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: Predictor.h
 * @Brief: declaration of the Predictor
 * @Author: zhe.sun
 * @Date: 2020-01-10 11:26:50
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-01-10 11:26:50
 */

#ifndef INCLUDE_PREDICTOR_PREDICTOR_H_
#define INCLUDE_PREDICTOR_PREDICTOR_H_

#include <string>
#include <vector>
#include <memory>
#include "BPUModel_Manager.h"
#include "CNNConst.h"
#include "CNNMethodCreator.h"
#include "CNNMethodConfig.h"
#include "CNNPredictorOutputData.h"
#include "hobotxsdk/xstream_data.h"
#include "hobotxstream/method.h"
#include "bpu_predict/bpu_predict.h"
#include "bpu_predict/bpu_io.h"

namespace xstream {
namespace CnnProc {
bool CNNPredictorQuery(const std::string& method_name);
MethodPtr CNNPredictorCreate(const std::string& method_name);

struct NormParams {
  NormMethod norm_type;
  float expand_scale;
  float aspect_ratio;
};

class CNNPredictor : public Method {
 public:
  CNNPredictor() {}
  virtual ~CNNPredictor() {}

  virtual int Init(const std::string &cfg_path);
  virtual void Finalize();

  virtual std::vector<std::vector<BaseDataPtr> >
  DoProcess(const std::vector<std::vector<BaseDataPtr> > &input,
            const std::vector<xstream::InputParamPtr> &param) = 0;

  virtual int UpdateParameter(xstream::InputParamPtr ptr);
  virtual InputParamPtr GetParameter() const;
  virtual std::string GetVersion() const;
  virtual void OnProfilerChanged(bool on) {}

  int RunModelFromResizer(hobot::vision::PymImageFrame &pym_image,
                          BPUBBox *box, int box_num, int *resizable_cnt,
                          BPU_Buffer_Handle *output_buf, int output_size,
                          BPUModelHandle *model_handle);

  int RunModelFromImage(uint8_t *data,
                        int data_size,
                        BPU_Buffer_Handle *output_buf,
                        int output_size,
                        BPUModelHandle *model_handle,
                        BPUFakeImage *bpu_fake_image);

 protected:
  int NormalizeRoi(hobot::vision::BBox *src, hobot::vision::BBox *dst,
                   NormParams norm_params,
                   uint32_t total_w,
                   uint32_t total_h,
                   FilterMethod filter_method = FilterMethod::OUT_OF_RANGE);

 private:
  int FilterRoi(hobot::vision::BBox *src,
                         hobot::vision::BBox *dst,
                         int src_w,
                         int src_h,
                         FilterMethod filter_method);

 protected:
  std::string model_name_;
  std::string model_version_;
  std::string model_path_;
  std::string input_type_;
  std::shared_ptr<CNNMethodConfig> config_;

  ModelInfo model_info_;
  BPUFakeImageHandle fake_img_handle_ = nullptr;
  BPUHandle bpu_handle_ = nullptr;
  std::vector<std::vector<int8_t>> feature_bufs_;

  int32_t max_handle_num_ = -1;  // Less than 0 means unlimited

  static std::mutex init_mutex_;
};
}  // namespace CnnProc
}  // namespace xstream
#endif  // INCLUDE_PREDICTOR_PREDICTOR_H_
