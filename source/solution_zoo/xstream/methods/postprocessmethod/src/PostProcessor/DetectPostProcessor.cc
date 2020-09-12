/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: DetectPostProcessor.cc
 * @Brief: definition of the DetectPostProcessor
 * @Author: zhe.sun
 * @Email: zhe.sun@horizon.ai
 * @Date: 2020-08-28 14:28:17
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-08-28 16:19:08
 */

#include "PostProcessMethod/PostProcessor/DetectPostProcessor.h"
#include <vector>
#include <memory>
#include <set>
#include "common/util.h"
#include "common/apa_data.h"
#include "PostProcessMethod/PostProcessor/DetectConst.h"
#include "bpu_predict/bpu_predict_extension.h"
#include "bpu_predict/bpu_predict.h"
#include "bpu_predict/bpu_parse_utils.h"
#include "bpu_predict/bpu_parse_utils_extension.h"
#include "hobotxstream/profiler.h"
#include "hobotxsdk/xstream_data.h"
#include "hobotlog/hobotlog.hpp"
#include "horizon/vision_type/vision_type.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

namespace xstream {

int DetectPostProcessor::Init(const std::string &cfg) {
  PostProcessor::Init(cfg);
  auto net_info = config_->GetSubConfig("net_info");
  std::vector<std::shared_ptr<Config>> model_out_sequence =
      net_info->GetSubConfigArray("model_out_sequence");

  for (size_t i = 0; i < model_out_sequence.size(); ++i) {
    std::string type_str = model_out_sequence[i]->GetSTDStringValue("type");
    HOBOT_CHECK(!type_str.empty());
    if (str2detect_out_type.find(type_str) ==
        str2detect_out_type.end()) {
      out_level2branch_info_[i].type = DetectBranchOutType::INVALID;
    } else {
      out_level2branch_info_[i].type = str2detect_out_type[type_str];
      out_level2branch_info_[i].name =
          model_out_sequence[i]->GetSTDStringValue("name");
      out_level2branch_info_[i].box_name =
          model_out_sequence[i]->GetSTDStringValue("box_name");
      out_level2branch_info_[i].labels =
          model_out_sequence[i]->GetLabelsMap("labels");
    }
  }

  method_outs_ = config_->GetSTDStringArray("method_outs");
  iou_threshold_ = config_->GetFloatValue("iou_threshold", iou_threshold_);

  return 0;
}

void DetectPostProcessor::GetModelInfo() {
  HOBOT_CHECK(bpu_model_ != nullptr);

  uint32_t output_layer_num = bpu_model_->output_num;
  for (size_t i = 0; i < output_layer_num; ++i) {
    auto &branch_info = out_level2branch_info_[i];
    // get shifts
    const uint8_t *shift_value = bpu_model_->outputs[i].shifts;
    // dim of per shape = 4
    int shape_dim = bpu_model_->outputs[i].aligned_shape.ndim;
    HOBOT_CHECK(shape_dim == 4)
        << "shape_dim = " << shape_dim;
    std::vector<int> aligned_dim(shape_dim);
    std::vector<int> real_dim(shape_dim);
    for (int dim = 0; dim < shape_dim; dim++) {
      aligned_dim[dim] = bpu_model_->outputs[i].aligned_shape.d[dim];
      real_dim[dim] = bpu_model_->outputs[i].shape.d[dim];
    }
    branch_info.shift = shift_value[0];
    branch_info.aligned_nhwc = aligned_dim;
    branch_info.real_nhwc = real_dim;

    // element_type_bytes
    branch_info.element_type_bytes = 1;
    switch (bpu_model_->outputs[i].data_type) {
      case BPU_TYPE_TENSOR_F32:
      case BPU_TYPE_TENSOR_S32:
      case BPU_TYPE_TENSOR_U32:
        branch_info.element_type_bytes = 4;
      default:
        break;
    }
    // valid_output_size
    branch_info.valid_output_size = 1;
    for (int dim = 0; dim < bpu_model_->outputs[i].shape.ndim; dim++) {
      branch_info.valid_output_size *= bpu_model_->outputs[i].shape.d[dim];
    }
    branch_info.valid_output_size *= branch_info.element_type_bytes;
    // aligned_output_size
    branch_info.aligned_output_size = 1;
    for (int dim = 0; dim < bpu_model_->outputs[i].aligned_shape.ndim; dim++) {
      branch_info.aligned_output_size *=
          bpu_model_->outputs[i].aligned_shape.d[dim];
    }
    branch_info.aligned_output_size *= branch_info.element_type_bytes;
    // output_off
    if (i == 0) {
      branch_info.output_off = 0;
    } else {
      branch_info.output_off = out_level2branch_info_[i-1].output_off +
        out_level2branch_info_[i-1].aligned_output_size;
    }
  }
}

std::vector<std::vector<BaseDataPtr>> DetectPostProcessor::Do(
    const std::vector<std::vector<BaseDataPtr>> &input,
    const std::vector<xstream::InputParamPtr> &param) {
  std::vector<std::vector<BaseDataPtr>> output;
  output.resize(input.size());
  for (size_t i = 0; i < input.size(); i++) {
    const auto &frame_input = input[i];
    auto &frame_output = output[i];
    RunSingleFrame(frame_input, frame_output);
  }
  return output;
}

void DetectPostProcessor::RunSingleFrame(
    const std::vector<BaseDataPtr> &frame_input,
    std::vector<BaseDataPtr> &frame_output) {
  LOGD << "DetectPostProcessor RunSingleFrame";
  HOBOT_CHECK(frame_input.size() == 1);

  for (size_t out_index = 0; out_index < method_outs_.size(); ++out_index) {
    frame_output.push_back(std::make_shared<xstream::BaseDataVector>());
  }

  auto async_data = std::static_pointer_cast<XStreamData<
      std::shared_ptr<hobot::vision::AsyncData>>>(frame_input[0]);
  if (async_data->value->bpu_model == nullptr ||
      async_data->value->task_handle == nullptr ||
      async_data->value->output_tensors == nullptr ||
      async_data->value->input_tensors == nullptr) {
    LOGE << "Invalid AsyncData";
    return;
  }
  bpu_model_ = std::static_pointer_cast<BPU_MODEL_S>(
      async_data->value->bpu_model);
  task_handle_ = std::static_pointer_cast<BPU_TASK_HANDLE>(
      async_data->value->task_handle);
  input_tensors_ = std::static_pointer_cast<std::vector<BPU_TENSOR_S>>(
      async_data->value->input_tensors);
  output_tensors_ = std::static_pointer_cast<std::vector<BPU_TENSOR_S>>(
      async_data->value->output_tensors);
  src_image_width_ = async_data->value->src_image_width;
  src_image_height_ = async_data->value->src_image_height;
  model_input_height_ = async_data->value->model_input_height;
  model_input_width_ = async_data->value->model_input_width;

  // get model info
  if (!is_init_) {
    GetModelInfo();
    is_init_ = true;
  }

  {
    RUN_PROCESS_TIME_PROFILER("DetectPostRunModel");
    RUN_FPS_PROFILER("DetectPostRunModel");

    HB_BPU_waitModelDone(task_handle_.get());
    // release input
    ReleaseTensor(input_tensors_);
    // release BPU_TASK_HANDLE
    HB_BPU_releaseTask(task_handle_.get());
  }

  {
    RUN_PROCESS_TIME_PROFILER("DetectPostProcess");
    RUN_FPS_PROFILER("DetectPostProcess");

    DetectOutMsg det_result;
    PostProcess(det_result);

    // release output_tensors
    ReleaseTensor(output_tensors_);

    CoordinateTransOutMsg(det_result, src_image_width_, src_image_height_,
                          model_input_width_, model_input_height_);
    for (auto &boxes : det_result.boxes) {
      LOGD << boxes.first << ", num: " << boxes.second.size();
      for (auto &box : boxes.second) {
        LOGD << box;
      }
    }

    // convert DetectOutMsg to xstream data structure
    std::map<std::string, std::shared_ptr<BaseDataVector>> xstream_det_result;
    GetResultMsg(det_result, xstream_det_result);

    for (size_t out_index = 0; out_index < method_outs_.size(); ++out_index) {
      if (xstream_det_result[method_outs_[out_index]]) {
        frame_output[out_index] = xstream_det_result[method_outs_[out_index]];
      }
    }
  }
}

void DetectPostProcessor::PostProcess(DetectOutMsg &det_result) {
  for (size_t out_level = 0; out_level < output_tensors_->size(); ++out_level) {
    const auto &branch_info = out_level2branch_info_[out_level];
    auto out_type = branch_info.type;
    HB_SYS_flushMemCache(&(output_tensors_->at(out_level).data),
                         HB_SYS_MEM_CACHE_INVALIDATE);
    switch (out_type) {
      case DetectBranchOutType::INVALID:
      {
        #if 0
        // test dump raw output
        int8_t* result = reinterpret_cast<int8_t*>
          ((output_tensors_->at(out_level).data.virAddr);
        std::fstream outfile;
        outfile.open("output.txt", std::ios_base::out|std::ios_base::trunc);
        for (int i = 0; i < 64*32*16; i++) {
          outfile << +(*(result+i)) << " ";
          if ((i+1)%16 == 0) {
            outfile << std::endl;
          }
        }
        outfile.close();
        #endif
        break;
      }
      case DetectBranchOutType::APABBOX:
        ParseAPADetectionBox(
            output_tensors_->at(out_level).data.virAddr,
            out_level, det_result.boxes[branch_info.name]);
        break;
      case DetectBranchOutType::BBOX:
        // need next layer data, need flush
        HB_SYS_flushMemCache(&(output_tensors_->at(out_level+1).data),
                             HB_SYS_MEM_CACHE_INVALIDATE);
        ParseDetectionBox(
            output_tensors_->at(out_level).data.virAddr, out_level,
            output_tensors_->at(out_level+1).data.virAddr,
            det_result.boxes[branch_info.name]);
        break;
      case DetectBranchOutType::MASK:
        ParseDetectionMask(
          output_tensors_->at(out_level).data.virAddr,
          out_level, det_result.segmentations[branch_info.name]);
        break;
      default:
        break;
    }
  }
}

// convert DetectOutMsg to xstream data structure
void DetectPostProcessor::GetResultMsg(
    DetectOutMsg &det_result,
    std::map<std::string, std::shared_ptr<BaseDataVector>>
      &xstream_det_result) {
  // box
  for (const auto &boxes : det_result.boxes) {
    xstream_det_result[boxes.first] =
        std::make_shared<xstream::BaseDataVector>();
    xstream_det_result[boxes.first]->name_ = "rcnn_" + boxes.first;
    for (uint i = 0; i < boxes.second.size(); ++i) {
      const auto &box = boxes.second[i];
      auto xstream_box = std::make_shared<XStreamData<BBox>>();
      xstream_box->value = std::move(box);
      xstream_det_result[boxes.first]->datas_.push_back(xstream_box);
    }
  }

  // segmentations
  for (const auto &segmentation_vec : det_result.segmentations) {
    xstream_det_result[segmentation_vec.first] =
        std::make_shared<xstream::BaseDataVector>();
    xstream_det_result[segmentation_vec.first]->name_ =
        "rcnn_" + segmentation_vec.first;
    for (auto &segmentation : segmentation_vec.second) {
      auto xstream_segmentation = std::make_shared<XStreamData<Segmentation>>();
      xstream_segmentation->value = std::move(segmentation);
      xstream_det_result[segmentation_vec.first]->datas_.push_back(
          xstream_segmentation);
    }
  }
}

void DetectPostProcessor::CoordinateTransOutMsg(
    DetectOutMsg &det_result,
    int src_image_width, int src_image_height,
    int model_input_width, int model_input_hight) {
  for (auto &boxes : det_result.boxes) {
    for (auto &box : boxes.second) {
      CoordinateTransform(box.x1, box.y1,
                          src_image_width, src_image_height,
                          model_input_width, model_input_hight);
      CoordinateTransform(box.x2, box.y2,
                          src_image_width, src_image_height,
                          model_input_width, model_input_hight);
    }
  }

  for (auto &landmarks : det_result.landmarks) {
    for (auto &landmark : landmarks.second) {
      for (auto &point : landmark.values) {
        CoordinateTransform(point.x, point.y,
                            src_image_width, src_image_height,
                            model_input_width, model_input_hight);
      }
    }
  }
}

// branch_num: layer
void DetectPostProcessor::ParseDetectionBox(
    void* result, int branch_num, void* anchor,
    std::vector<BBox> &boxes) {
  LOGD << "Into ParseDetectionBox";
  uint8_t *det_result = reinterpret_cast<uint8_t *>(result);
  uint8_t *det_anchor = reinterpret_cast<uint8_t *>(anchor);

  int32_t each_result_bytes = 16;
  int32_t layer_base = 0;
  const int32_t max_dpp_branch = 5;   // dpp constraint

  struct BPUDetNMSResultRaw {
    int16_t xmin;
    int16_t ymin;
    int16_t xmax;
    int16_t ymax;
    int8_t score;
    uint8_t class_id;
    int16_t padding1;
    int16_t padding2;
    int16_t padding3;
  };
  struct SortStruct {
    BPUDetNMSResultRaw *pos;
    int16_t h, w;          // anchor中心点
    uint8_t detout_layer;  // 输出层索引
  };

  std::vector<std::vector<SortStruct>> layer_bbox(max_dpp_branch);

  // 偏移量是该层对齐大小
  int8_t *raw_result = reinterpret_cast<int8_t *>(det_result);
  int8_t *raw_anchor = reinterpret_cast<int8_t *>(det_anchor);

  uint16_t valid_len = *(reinterpret_cast<uint16_t *>(raw_result));
  int32_t valid_result_cnt = valid_len / each_result_bytes;
  LOGD << "valid_len: " << valid_len;
  LOGD << "valid_result_cnt: " << valid_result_cnt;
  BPUDetNMSResultRaw *curr_layer_result =
    reinterpret_cast<BPUDetNMSResultRaw *>(raw_result + 16);
  BPUDetNMSResultRaw *curr_layer_anchor =
    reinterpret_cast<BPUDetNMSResultRaw *>(raw_anchor + 16);

  // clear cached data
  for (int32_t k = layer_base; k < layer_base + max_dpp_branch; k++) {
    layer_bbox[k].clear();
  }

  for (int32_t k = 0; k < valid_result_cnt; k++) {
    BPUDetNMSResultRaw *curr_result = &curr_layer_result[k];
    BPUDetNMSResultRaw *curr_anchor = &curr_layer_anchor[k];

    SortStruct data;
    data.pos = curr_result;
    data.detout_layer = static_cast<uint8_t>(branch_num);
    data.h = curr_anchor->ymin +
      (curr_anchor->ymax - curr_anchor->ymin) / 2;
    data.w = curr_anchor->xmin +
      (curr_anchor->xmax - curr_anchor->xmin) / 2;
    int8_t branch_id =
      static_cast<int8_t>(curr_anchor->xmax - curr_anchor->xmin) / 2;

    if (branch_id < 0 || branch_id >= static_cast<int>(layer_bbox.size())) {
      LOGD << static_cast<int>(branch_id)
        << " " << static_cast<int>(curr_anchor->xmax)
        << " " << static_cast<int>(curr_anchor->xmin);
      continue;
    }
    layer_bbox[branch_id].push_back(data);
  }

  std::vector<BBox> vect_bbox;
  vect_bbox.clear();

  LOGD << "========== orig dump begin ============";
  for (size_t i = 0; i < layer_bbox.size(); i++) {
    if (layer_bbox[i].empty()) {
      continue;
    }
    std::stable_sort(layer_bbox[i].begin(), layer_bbox[i].end(),
      [](const SortStruct &s1, const SortStruct & s2) {
      if (s1.h == s2.h) {
        return (s1.w < s2.w);
      } else {
        return (s1.h < s2.h);
      }
    });

    for (size_t j = 0; j < layer_bbox[i].size(); j++) {
      const BPUDetNMSResultRaw &curr_result = *layer_bbox[i][j].pos;

      int16_t xmin_i = curr_result.xmin >> 2;
      float xmin_f = static_cast<float>(curr_result.xmin & 0x3) / 4.0f;
      float xmin = static_cast<float>(xmin_i) + xmin_f;
      int16_t ymin_i = curr_result.ymin >> 2;
      float ymin_f = static_cast<float>(curr_result.ymin & 0x3) / 4.0f;
      float ymin = static_cast<float>(ymin_i) + ymin_f;
      int16_t xmax_i = curr_result.xmax >> 2;
      float xmax_f = static_cast<float>(curr_result.xmax & 0x3) / 4.0f;
      float xmax = static_cast<float>(xmax_i) + xmax_f;
      int16_t ymax_i = curr_result.ymax >> 2;
      float ymax_f = static_cast<float>(curr_result.ymax & 0x3) / 4.0f;
      float ymax = static_cast<float>(ymax_i) + ymax_f;

      BBox bbox;
      bbox.x1 = xmin;
      bbox.y1 = ymin;
      bbox.x2 = xmax;
      bbox.y2 = ymax;
      bbox.id = curr_result.class_id;
      bbox.score = curr_result.score;  // conf_scale
      vect_bbox.push_back(bbox);
    }
  }

  LOGD << "========== nms dump begin ============";
  std::vector<BBox> merged_local;
  LocalIOU(vect_bbox, merged_local,
           iou_threshold_, vect_bbox.size(), false);

  for (size_t m = 0; m < merged_local.size(); m++) {
    LOGD << merged_local[m].id << merged_local[m];
    // 存入boxes
    boxes.push_back(merged_local[m]);
  }
}

void DetectPostProcessor::ParseDetectionMask(
    void* result, int branch_num, std::vector<Segmentation> &masks) {
  uint8_t* mask_feature = reinterpret_cast<uint8_t *>(result);

  Segmentation mask;
  mask.height = out_level2branch_info_[branch_num].aligned_nhwc[1];
  mask.width = out_level2branch_info_[branch_num].aligned_nhwc[2];
  int feature_size = mask.height * mask.width;
  float score = 0;
  for (int i = 0; i < feature_size; ++i) {
    uint8_t fp_mask = *(mask_feature + 2*i);
    mask.values.push_back(fp_mask);
    uint8_t fp_score = *(mask_feature + 2*i + 1);
    mask.pixel_score.push_back(fp_score);
    score += fp_score;
  }

  #if 0
  // test mask
  cv::Mat mask_cv(mask.height, mask.width, CV_8UC1);
  for (int h = 0; h < mask.height; ++h) {
    uchar *p_gray = mask_cv.ptr<uchar>(h);
    for (int w = 0; w < mask.width; ++w) {
      p_gray[w] = mask.values[h*mask.width+w];
    }
  }
  cv::imwrite("mask.jpg", mask_cv);
  #endif

  score = score / feature_size;
  mask.score = score;
  masks.push_back(mask);
}

void DetectPostProcessor::ParseAPADetectionBox(
    void* result, int branch_num,
    std::vector<BBox> &boxes) {
  LOGD << "Into ParseAPADetectionBox";
  int8_t *anchor_result = reinterpret_cast<int8_t *>(result);
  std::vector<BPUParkingAnchorResult> cached_result;

  std::vector<float> anchor_scale(
      out_level2branch_info_[branch_num].aligned_nhwc[3],
      32);

  int dim_h = out_level2branch_info_[branch_num].aligned_nhwc[1];
  int dim_w = out_level2branch_info_[branch_num].aligned_nhwc[2];
  int dim_c = out_level2branch_info_[branch_num].aligned_nhwc[3];
  int line_step = dim_w * dim_c;
  // 1. find max conf point and get anchor id and type
  {
    for (int h = 0; h < dim_h; ++h) {
      int8_t * conf_index = anchor_result + h * line_step;
      for (int w = 0; w < dim_w; w++) {
        for (int channel_id = 0; channel_id < 2; channel_id++) {
          int conf = *(conf_index + w * dim_c + channel_id * 6);
          // conf 阈值
          if (conf >= -0.8473 * anchor_scale[channel_id]) {
            BPUParkingAnchorResult result;
            result.h = h;
            result.w = w;
            result.anchor = channel_id;
            result.conf = conf;
            result.c = channel_id;
            cached_result.push_back(result);
          }
        }
      }
    }
  }

  // 2 get anchor
  {
    for (size_t i = 0; i < cached_result.size(); ++i) {
      BPUParkingAnchorResult &result =
          cached_result[i];
      int anchor_id = result.anchor;
      int8_t *base_addr =
        anchor_result + result.h * line_step +
        result.w * dim_c + anchor_id * 6 + 1;    // dx

      result.box.data.x = *base_addr;
      result.box.data.y = *(base_addr + 1);
      result.box.data.z = 0;
      result.box.data.l = *(base_addr + 2);
      result.box.data.w = *(base_addr + 3);
      result.box.data.h = 0;
      result.box.data.theta = *(base_addr + 4);
    }
  }

  // 3 decode 2DBoxTo3D
  std::vector<Parking3DBBox> result_box;
  {
    float da = 0.0f, xa = 0.0f, ya = 0.0f;
    float la = 0.0f, wa = 0.0f, thetaa = 0.0f;
    float ouput_scale = 8;  // TODO(zhe.sun) need get from modelinfo
    for (size_t k = 0; k < cached_result.size(); k++) {
      BPUParkingAnchorResult &unet_obj = cached_result[k];
      int anchor_id = unet_obj.anchor;
      int c = unet_obj.c;

      std::vector<LidarAnchor> lidar_anchor {
          {0, 0, 0, 100, 50, 0, 0, 111.803},
          {0, 0, 0, 100, 50, 0, 1.57, 111.803}};
      xa = lidar_anchor[anchor_id].x + unet_obj.w * ouput_scale;
      ya = lidar_anchor[anchor_id].y + unet_obj.h * ouput_scale;
      la = lidar_anchor[anchor_id].l;
      wa = lidar_anchor[anchor_id].w;
      thetaa = lidar_anchor[anchor_id].t;
      da = lidar_anchor[anchor_id].d;
      float y_min_ = -1000.0f;
      float y_max_ = 5750.0f;
      float x_min_ = -1000.0f;
      float x_max_ = 5750.0f;
      float center_x, center_y, result_l, result_w, theta;
      center_x = (unet_obj.box.data.x / anchor_scale[0]) * da + xa;
      if (center_x > x_max_ || center_x < x_min_) {
        continue;
      }
      center_y = (unet_obj.box.data.y / anchor_scale[1]) * da + ya;
      if (center_y > y_max_ || center_y < y_min_) {
        continue;
      }
      result_l = expf(unet_obj.box.data.l / anchor_scale[3]) * la;
      result_w = expf(unet_obj.box.data.w / anchor_scale[4]) * wa;
      theta = unet_obj.box.data.theta / anchor_scale[6] + thetaa;
      theta = -theta;

      Parking3DBBox box(unet_obj.conf / anchor_scale[c],
          center_x, center_y, 0, result_l, 0,
          result_w, theta);
      result_box.push_back(box);
    }
  }

  // 4. NMS
  {
    std::vector<Parking3DBBox> merge_result;
    float nms_thresh_hold = 0.3;
    Parking_NMS_local_iou(result_box, merge_result, nms_thresh_hold,
                          result_box.size(), false);
    std::vector<cv::Mat> out_2dbox;
    Center_To_Corner_Box2d(merge_result, out_2dbox);
    LOGD << "result size: " << merge_result.size();
    for (size_t j = 0; j < merge_result.size(); ++j) {
      LOGD << "x: " << merge_result[j].x
           << ", y: " << merge_result[j].y
           << ", width: " << merge_result[j].w
           << ", length: " << merge_result[j].l
           << ", theta: " << merge_result[j].theta;
      // BBox
      BBox box;
      {
        box.x1 = merge_result[j].x - merge_result[j].l / 2;
        box.y1 = merge_result[j].y - merge_result[j].w / 2;
        box.x2 = merge_result[j].x + merge_result[j].l / 2;
        box.y2 = merge_result[j].y + merge_result[j].w / 2;
        box.score = merge_result[j].score;
        box.rotation_angle = merge_result[j].theta;
      }
      boxes.push_back(box);

      // // perception raw
      // LOGD << "x1: " << out_2dbox[j].at<float>(0, 0)
      //      << ", y1: " << out_2dbox[j].at<float>(0, 1)
      //      << ", x2: " << out_2dbox[j].at<float>(1, 0)
      //      << ", y2: " << out_2dbox[j].at<float>(1, 1)
      //      << ", x3: " << out_2dbox[j].at<float>(2, 0)
      //      << ", y3: " << out_2dbox[j].at<float>(2, 1)
      //      << ", x4: " << out_2dbox[j].at<float>(3, 0)
      //      << ", y4: " << out_2dbox[j].at<float>(3, 1);
    }
  }
}

void DetectPostProcessor::LocalIOU(
    std::vector<BBox> &candidates,
    std::vector<BBox> &result,
    const float overlap_ratio, const int top_N,
    const bool addScore) {
  if (candidates.size() == 0) {
    return;
  }
  std::vector<bool> skip(candidates.size(), false);

  auto greater = [](const BBox &a, const BBox &b) {
    return a.score > b.score;
  };
  std::stable_sort(candidates.begin(), candidates.end(),
                   greater);

  int count = 0;
  for (size_t i = 0; count < top_N && i < skip.size(); ++i) {
    if (skip[i]) {
      continue;
    }
    skip[i] = true;
    ++count;

    float area_i = candidates[i].Width() * candidates[i].Height();

    // suppress the significantly covered bbox
    for (size_t j = i + 1; j < skip.size(); ++j) {
      if (skip[j]) {
        continue;
      }
      // get intersections
      float xx1 =
          std::max(candidates[i].x1, candidates[j].x1);
      float yy1 =
          std::max(candidates[i].y1, candidates[j].y1);
      float xx2 =
          std::min(candidates[i].x2, candidates[j].x2);
      float yy2 =
          std::min(candidates[i].y2, candidates[j].y2);
      float area_intersection = (xx2 - xx1) * (yy2 - yy1);
      bool area_intersection_valid = (area_intersection > 0) && (xx2 - xx1 > 0);

      if (area_intersection_valid) {
        // compute overlap
        float area_j = candidates[j].Width() * candidates[j].Height();
        float o = area_intersection / (area_i + area_j - area_intersection);

        if (o > overlap_ratio) {
          skip[j] = true;
          if (addScore) {
            candidates[i].score += candidates[j].score;
          }
        }
      }
    }
    result.push_back(candidates[i]);
  }
  return;
}

}  // namespace xstream
