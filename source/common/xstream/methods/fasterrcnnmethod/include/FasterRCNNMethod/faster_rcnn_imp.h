//
// Created by yaoyao.sun on 2019-04-23.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#ifndef INCLUDE_FASTERRCNNMETHOD_FASTER_RCNN_IMP_H_
#define INCLUDE_FASTERRCNNMETHOD_FASTER_RCNN_IMP_H_

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <mutex>
#include <unordered_map>
#include <atomic>
#include <utility>
#include "hobotxsdk/xstream_data.h"
#include "horizon/vision_type/vision_type.hpp"
#include "bpu_predict/bpu_predict.h"
#include "bpu_predict/bpu_predict_extension.h"
#include "./plat_cnn.h"
#include "result.h"
#include "config.h"
#include "hobot_vision/bpu_model_manager.hpp"

namespace faster_rcnn_method {

enum class FasterRCNNBranchOutType {
  BBOX,
  KPS,
  MASK,
  REID,
  LMKS2_LABEL,
  LMKS2_OFFSET,
  LMKS1,
  POSE_3D,
  PLATE_COLOR,
  PLATE_ROW,
  KPS_LABEL,
  KPS_OFFSET,
  LMKS,
  INVALID
};

struct FasterRCNNBranchInfo {
  FasterRCNNBranchOutType type;
  std::string name;
  std::string box_name;
  std::unordered_map<int, std::string> labels;
};

class FasterRCNNImp {
 public:
  FasterRCNNImp() {}

  virtual ~FasterRCNNImp() {}

  // return 0 for successed, -1 for failed.
  int Init(const std::string &config_file);

  // return 0 for successed,  else failed.
  int UpdateParameter(xstream::InputParamPtr ptr) {
    faster_rcnn_param_ = ptr;
    return 0;
  }

  xstream::InputParamPtr GetParameter() const { return faster_rcnn_param_; }

  std::string GetVersion() const { return model_version_; }

  void RunSingleFrame(const std::vector<xstream::BaseDataPtr> &frame_input,
                      std::vector<xstream::BaseDataPtr> &frame_output);
  // xstream framework will call this function to do something, like release
  // resources.
  void Finalize();

 private:
  void ParseConfig(const std::string &config_file);

  void GetModelInfo(const std::string &model_name);

  void GetFrameOutput(int src_img_width, int src_img_height,
                      std::vector<xstream::BaseDataPtr> &frame_output);

  void PostProcess(FasterRCNNOutMsg &det_result);

  // get face, head or body boxes from rpp.
  void GetRppRects(std::map<std::string, std::vector<hobot::vision::BBox>> &boxes,
                   int index,
                   const std::string &name,
                   const std::unordered_map<int, std::string> &labels);
  // get body skeleton.
  void GetKps(std::vector<Landmarks> &kpss,
              void* output,
              const std::vector<BBox> &body_boxes);
  // get body skeleton.
  void GetKps2(std::vector<hobot::vision::Landmarks> &kpss,
               void* kps_label_output, void* kps_offset_output,
               const std::vector<hobot::vision::BBox> &body_boxes);
  // get body segmentation
  void GetMask(std::vector<Segmentation> &masks,
               void* output,
               const std::vector<BBox> &body_boxes);
  void GetReid(std::vector<Feature> &reids,
               void* output,
               const std::vector<BBox> &body_boxes);
  // get face lmk
  void GetLMKS(std::vector<hobot::vision::Landmarks> &landmarks,
      void* output,
      const std::vector<hobot::vision::BBox> &face_boxes);
  // get face lmk2
  void GetLMKS2(std::vector<Landmarks> &landmarks,
                void* lmks2_label_output,
                void* lmks2_offset_output,
                const std::vector<BBox> &face_boxes);
  // get face lmk1, lmk1 means regression lmk, used to supplement lmk2.
  void GetLMKS1(std::vector<Landmarks> &landmarks,
                void* output,
                const std::vector<BBox> &face_boxes);

  // get face 3dpose.
  void GetPose(std::vector<Pose3D> &face_pose,
               void* output,
               const std::vector<BBox> &face_boxes);
  // get vehicle plate color.
  void GetPlateColor(std::vector<hobot::vision::Attribute<int>> *plate_color,
                     void* output,
                     const std::vector<hobot::vision::BBox> &plate_box);

  // get vehicle plate row.
  void GetPlateRow(std::vector<hobot::vision::Attribute<int>> *plates_row,
                   void* output,
                   const std::vector<hobot::vision::BBox> &plate_boxes);

  // run model
  int RunModel(uint8_t *img_data,
               int data_length,
               BPU_DATA_TYPE_E data_type);

  int RunModelFromPym(void* pyramid, int pym_layer,
                      BPU_DATA_TYPE_E data_type);

  int RunModelResize(uint8_t *img_data,
                     int data_length,
                     int img_height,
                     int img_width,
                     int img_channel,
                     BPU_DATA_TYPE_E pre_data_type,
                     BPU_DATA_TYPE_E model_data_type);

  // prepare input_tensor
  void PrepareInputTensor(uint8_t *img_data,
                          int data_length,
                          BPU_DATA_TYPE_E data_type);

  // prepare output_tensor
  void PrepareOutputTensor();

  // release input tensor
  void ReleaseInputTensor(std::vector<BPU_TENSOR_S> &input_tensors);

  // release output tensor
  void ReleaseOutputTensor();
  void FlushOutputTensor();
  std::vector<BPU_TENSOR_S> input_tensors_;
  std::vector<BPU_TENSOR_S> output_tensors_;
  bool output_tensors_alloced_ = false;
  BPU_MODEL_S* bpu_model_ = nullptr;

  std::string bpu_config_path_;
  std::string model_file_path_;
  std::string model_name_;
  std::string model_version_;
  int pyramid_layer_;
  std::map<int, FasterRCNNBranchInfo> out_level2rcnn_branch_info_;
  std::vector<std::string> method_outs_;

  std::shared_ptr<Config> config_;

  int model_input_width_;
  int model_input_height_;

  float kps_pos_distance_;
  int kps_feat_width_;
  int kps_feat_height_;
  int kps_points_number_;
  int kps_feat_stride_;
  float kps_anchor_param_;

  float lmk_pos_distance_;
  int lmk_feat_width_;
  int lmk_feat_height_;
  int lmk_feat_stride_;
  int lmk_points_number_;
  float lmk_anchor_param_;
  int face_pose_number_;
  float face_pv_thr_ = 0;

  int32_t plate_color_num_;
  int32_t plate_row_num_;

  // TODO(yaoyao.sun) use vector<uint32_t>
  uint32_t kps_shift_ = 0;
  uint32_t kps_label_shift_ = 0;
  uint32_t kps_offset_shift_ = 0;
  uint32_t mask_shift_ = 0;
  uint32_t reid_shift_ = 0;
  uint32_t lmks_shift_ = 0;
  uint32_t lmks2_label_shift_ = 0;
  uint32_t lmks2_offset_shift_ = 0;
  uint32_t lmks1_shift_ = 0;
  uint32_t face_pose_shift_ = 0;
  uint32_t plate_color_shift_ = 0;
  uint32_t plate_row_shift_ = 0;

  int core_id_ = 2;

  int *aligned_reid_dim = nullptr;
  int *aligned_mask_dim = nullptr;
  int *aligned_kps_dim = nullptr;
  int *aligned_kps_label_dim = nullptr;
  int *aligned_kps_offset_dim = nullptr;
  int *aligned_lmks_dim = nullptr;
  int *aligned_lmks2_label_dim = nullptr;
  int *aligned_lmks2_offset_dim = nullptr;
  int *aligned_lmks1_dim = nullptr;
  int *aligned_face_pose_dim = nullptr;
  int *aligned_plate_color_dim = nullptr;
  int *aligned_plate_row_dim = nullptr;

  int reid_element_type;
  int mask_element_type;
  int kps_element_type;
  int kps_label_element_type;
  int kps_offset_element_type;
  int lmk_element_type;
  int lmk1_element_type;
  int lmk2_label_element_type;
  int lmk2_offet_element_type;
  int face_pose_element_type;
  int plate_color_element_type;
  int plate_row_element_type;

  xstream::InputParamPtr faster_rcnn_param_;
};

}  // namespace faster_rcnn_method

#endif  // INCLUDE_FASTERRCNNMETHOD_FASTER_RCNN_IMP_H_
