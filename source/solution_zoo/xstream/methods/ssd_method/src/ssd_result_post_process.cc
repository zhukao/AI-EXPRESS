//
// Created by xudong.du on 2020.04.
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#include "ssd_method/ssd_result_post_process.h"

#include <assert.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>

#include "ssd_method/config.h"
#include "ssd_method/exptable.h"

/*
 * The following constant array could by passed by model.json in desc section.
 */
namespace xstream {
#define SCORE_LINE_BUFFER_SIZE 512
#define BOX_LINE_BUFFER_SIZE 96
#define MAX_ANCHOR_NUM_PER_PIXEL 6

static std::vector<float> stds_default = {0.1, 0.1, 0.2, 0.2};
static std::vector<float> means_default = {0, 0, 0, 0};
static std::vector<float> offset_default = {0.5, 0.5};
static std::vector<int> steps_default = {16, 32, 64, 128, 256, 512};
static std::vector<std::vector<float>> sizes_default = {
  {51.2,  72.4077344},
  {102.4, 139.2643529},
  {189.4, 228.8015734},
  {276.4, 316.9809584},
  {363.52, 404.7247361},
  {450.6, 470.8451975}
};
static std::vector<std::vector<float>>  ratios_default = {
  {1, 2, 0.5, 0, 0},
  {1, 2, 0.5, 3, 1.0 / 3},
  {1, 2, 0.5, 3, 1.0 / 3},
  {1, 2, 0.5, 3, 1.0 / 3},
  {1, 2, 0.5, 0, 0},
  {1, 2, 0.5, 0, 0}
};
static int ssd_cls_num_default = 21;
static SsdConfig DEFAULT_SSD_CONFIG = {
  stds_default,
  means_default,
  offset_default,
  steps_default,
  sizes_default,
  ratios_default,
  ssd_cls_num_default
};

int LoadConfig(const char *config_file, SsdConfig *ssd_conf,
                size_t layer_num) {
  std::cout << "config file " << config_file << std::endl;
  FR_Config cfg_jv;
  std::ifstream infile(config_file);
  infile >> cfg_jv;
  xstream::Config config(cfg_jv);

  auto config_ = config.GetSubConfig("model_params");
  if (nullptr == config_) {
    return -1;
  }

  auto mean_array = config_->GetFloatArray("mean");
  if (!mean_array.empty()) {
    if (mean_array.size() != 4) {
      return -1;
    }
    for (size_t i = 0; i < mean_array.size(); ++i) {
      (ssd_conf->means).push_back(mean_array[i]);
    }
  } else {
    ssd_conf->means = means_default;
  }

  auto std_array = config_->GetFloatArray("std");
  if (!std_array.empty()) {
    if (std_array.size() != 4) {
      return -1;
    }
    for (size_t i = 0; i < std_array.size(); ++i) {
      (ssd_conf->stds).push_back(std_array[i]);
    }
  } else {
    ssd_conf->stds = stds_default;
  }

  auto offset_array = config_->GetFloatArray("offset");
  if (!offset_array.empty()) {
    if (offset_array.size() != 2) {
      return -1;
    }
    for (size_t i = 0; i < offset_array.size(); ++i) {
      (ssd_conf->offset).push_back(offset_array[i]);
    }
  } else {
    ssd_conf->offset = offset_default;
  }
  auto step_array = config_->GetFloatArray("step");
  if (!step_array.empty()) {
    if (step_array.size() != layer_num) {
      return -1;
    }
    for (size_t i = 0; i < step_array.size(); ++i) {
      (ssd_conf->steps).push_back(step_array[i]);
    }
  } else {
    ssd_conf->steps = steps_default;
  }
  auto size_array = config_->GetFloatTwoDArray("anchor_size");
  if (!size_array.empty()) {
    if (size_array.size() != layer_num) {
      return -1;
    }
    for (size_t i = 0; i < size_array.size(); ++i) {
      auto size_vec = size_array[i];
      std::vector<float> tmp;
      for (size_t j = 0; j < size_vec.size(); ++j) {
        tmp.push_back(size_vec[j]);
      }
      (ssd_conf->sizes).push_back(tmp);
    }
  } else {
    ssd_conf->sizes = sizes_default;
  }
  auto ratio_array = config_->GetFloatTwoDArray("anchor_ratio");
  if (!ratio_array.empty()) {
    if (ratio_array.size() != layer_num) {
      return -1;
    }
    for (size_t i = 0; i < ratio_array.size(); ++i) {
      auto ratio_vec = ratio_array[i];
      std::vector<float> tmp;
      for (size_t j = 0; j < ratio_vec.size(); ++j) {
        tmp.push_back(ratio_vec[j]);
      }
      (ssd_conf->ratios).push_back(tmp);
    }
  } else {
    ssd_conf->ratios = ratios_default;
  }
  ssd_conf->cls_num = config_->GetIntValue("class_num", -1);
  if (-1 == ssd_conf->cls_num) {
    ssd_conf->cls_num = ssd_cls_num_default;
  }
  return 0;
}

int SsdAnchors(std::vector<Anchor> &anchors, int layer,
               std::vector<std::pair<uint32_t, uint32_t>> &shapes,
               const SsdConfig &ssd_conf,
               bool is_tf) {
    int step = (ssd_conf.steps)[layer];
    auto &offset = ssd_conf.offset;
    std::pair<uint32_t, uint32_t> shape = shapes[layer];
    auto size = (ssd_conf.sizes)[layer];
    auto ratio = (ssd_conf.ratios)[layer];
    for (uint32_t i = 0; i < shape.first; i++) {
      for (uint32_t j = 0; j < shape.second; j++) {
        float cy = (i + offset[0]) * step;
        float cx = (j + offset[1]) * step;
        float max_size = size[1];
        // the order of anchor table must same as ssd traning order
        if (is_tf) {
          anchors.push_back(Anchor(cx, cy, max_size, max_size));
          anchors.push_back(Anchor(cx, cy, size[0], size[0]));
        } else {
          anchors.push_back(Anchor(cx, cy, size[0], size[0]));
          anchors.push_back(Anchor(cx, cy, max_size, max_size));
        }
        for (int z = 1; z < 5; z++) {
          if (ratio[z] == 0) continue;
          float sr = std::sqrt(ratio[z]);
          float w = size[0] * sr;
          float h = size[0] / sr;
          anchors.push_back(Anchor(cx, cy, w, h));
        }
      }
    }
    return 0;
}

int GenShape(std::vector<std::pair<uint32_t, uint32_t>> &shapes,
             const BPU_MODEL_NODE_S* output_info, int num_layers) {
  for (int i = 0; i < num_layers; i++) {
    int32_t height, width;
    HB_BPU_getHW(output_info[i].data_type,
                 &output_info[i].shape, &height, &width);
    shapes.push_back({height, width});
  }
  return 0;
}

int SsdAnchorsGenerator(std::vector<std::vector<Anchor>> &anchors_table,
                        const BPU_MODEL_NODE_S *output_info,
                        const SsdConfig &ssd_conf, uint32_t num_outs,
                        bool is_tf = false) {
  uint32_t num_layers = num_outs / 2;
  std::vector<std::pair<uint32_t, uint32_t>> shapes;

  if (num_layers > 6) {
    return -1;
  }

  GenShape(shapes, output_info, num_layers);
  for (uint32_t i = 0; i < num_layers; i++) {
    std::vector<Anchor> anchors;
    SsdAnchors(anchors, i, shapes, ssd_conf, is_tf);
    anchors_table.push_back(anchors);
  }
  return 0;
}
float SsdFastExp(float x) {
    // return std::exp(x);
    union {
        uint32_t i;
        float f;
    } v;
    v.i = (12102203.1616540672f * x + 1064807160.56887296f);
    return v.f;
}

#define BSWAP_32(x) \
    static_cast<int32_t>(__builtin_bswap32(x))

#define r_int32(x, big_endian) \
    (big_endian) ? BSWAP_32((x)) : static_cast<int32_t>((x))

int GetBboxFromRawData(BPU_MODEL_S bpu_handle, void *raw_box_data,
                       int layer_num, std::vector<Anchor> &anchors,
                       std::vector<Detection> &dets, double score_threshold,
                       double *cls_table, const SsdConfig &ssd_conf,
                       const std::vector<bool> output_layer_endian,
                       bool is_tf = false) {
  int class_num = ssd_conf.cls_num;
  auto &stds = ssd_conf.stds;
  auto &means = ssd_conf.means;
  int *aligned_shape = bpu_handle.outputs[layer_num].aligned_shape.d;
  int *valid_shape = bpu_handle.outputs[layer_num].shape.d;
  int vecSize = output_layer_endian.size();
  bool big_endian = layer_num >= vecSize
                        ? false
                        : output_layer_endian[layer_num];  // layer_num从0开始
  int h_idx, w_idx, c_idx;
  HB_BPU_getHWCIndex(bpu_handle.outputs[layer_num].data_type,
                     &bpu_handle.outputs[layer_num].aligned_shape.layout,
                     &h_idx, &w_idx, &c_idx);
  uint8_t *shift_value = bpu_handle.outputs[layer_num].shifts;
  uint32_t nhwc[4];
  nhwc[0] = 1;
  nhwc[1] = aligned_shape[0] * aligned_shape[h_idx];
  nhwc[2] = aligned_shape[w_idx];
  nhwc[3] = aligned_shape[c_idx];
  uint32_t real_hnum = valid_shape[h_idx];
  uint32_t real_wnum = valid_shape[w_idx];
  uint32_t real_cnum = valid_shape[c_idx];

  uint32_t w_dest_stride = real_cnum / 4;
  uint32_t h_dest_stride = real_wnum * w_dest_stride;
  uint32_t anchor_num_per_pixel = real_cnum / 4;
  for (uint32_t hh = 0; hh < nhwc[1]; hh++) {
    uint32_t cur_h = hh % aligned_shape[1];
    if (cur_h >= real_hnum) continue;
    uint32_t res_id_cur_hh = hh * h_dest_stride;
    for (uint32_t ww = 0; ww < real_wnum; ww++) {
      uint32_t res_id_cur_ww = res_id_cur_hh + ww * w_dest_stride;
      bool is_valid_block = false;
      for (uint32_t t = 0; t < anchor_num_per_pixel; t++) {
        if (cls_table[(res_id_cur_ww + t) * class_num] < 0) continue;
        is_valid_block = true;
        break;
      }

      if (!is_valid_block) continue;

      int offset = hh * nhwc[2] * nhwc[3] + ww * nhwc[3];
      auto box_line_buffer =
          reinterpret_cast<uint32_t *>(raw_box_data) + offset;
      for (uint32_t anchor_id = 0; anchor_id < anchor_num_per_pixel;
           anchor_id++) {
        uint32_t res_id_cur_anchor = res_id_cur_ww + anchor_id;
        uint32_t res_id_cur_anchor_score =
            (res_id_cur_ww + anchor_id) * class_num;
        if (cls_table[res_id_cur_anchor_score] < 0) continue;
        // bool skip_flag = true;
        // int classes_begin = res_id_cur_anchor * 21  + 1;
        // int classes_end = classes_begin  + 20;
        // for (int w = classes_begin; w < classes_end; w++) {
        //     if (cls_table[w] > 0.01) {
        //         skip_flag = false;
        //     }
        // }
        // if (skip_flag) {
        //   box_output[res_id_cur_anchor].xmin = 0.0;
        //   box_output[res_id_cur_anchor].ymin = 0.0;
        //   box_output[res_id_cur_anchor].xmax = 0.0;
        //   box_output[res_id_cur_anchor].ymax = 0.0;
        //   continue;
        // }

        uint32_t cur_c = 4 * anchor_id;
        auto box = box_line_buffer + cur_c;
        float box_0 = static_cast<float>(r_int32(box[0], big_endian)) /
                      static_cast<float>(1 << shift_value[cur_c]);
        float box_1 = static_cast<float>(r_int32(box[1], big_endian)) /
                      static_cast<float>(1 << shift_value[cur_c + 1]);
        float box_2 = static_cast<float>(r_int32(box[2], big_endian)) /
                      static_cast<float>(1 << shift_value[cur_c + 2]);
        float box_3 = static_cast<float>(r_int32(box[3], big_endian)) /
                      static_cast<float>(1 << shift_value[cur_c + 3]);
        if (is_tf) {
          // reorder box to cy cx th tw
          std::swap(box_0, box_1);
          std::swap(box_2, box_3);
        }
        float ox =
            ((box_0 * stds[0] + means[0]) * anchors[res_id_cur_anchor].w) +
            anchors[res_id_cur_anchor].cx;
        float oy =
            ((box_1 * stds[1] + means[1]) * anchors[res_id_cur_anchor].h) +
            anchors[res_id_cur_anchor].cy;
        float tw, th;
        tw = std::exp(box_2 * stds[2] + means[2]);
        th = std::exp(box_3 * stds[3] + means[3]);
        float ow = (tw * anchors[res_id_cur_anchor].w) / 2.0;
        float oh = (th * anchors[res_id_cur_anchor].h) / 2.0;
        Bbox b(ox - ow, oy - oh, ox + ow, oy + oh);
        double cur_score;
        for (int t = 1; t < class_num; t++) {
          cur_score = cls_table[res_id_cur_anchor_score + t];
          if (cur_score > score_threshold) {
            dets.push_back(Detection(t - 1, cur_score, b));
          }
        }
      }
    }
  }
  return 0;
}
int SoftmaxFromRawScore(BPU_MODEL_S bpu_handle, void *raw_cls_data,
                        int layer_num, int class_num, double *cls_dest,
                        float cut_off_threshold, float filter_threshold,
                        const std::vector<bool> output_layer_endian) {
  int *aligned_shape = bpu_handle.outputs[layer_num].aligned_shape.d;
  int *valid_shape = bpu_handle.outputs[layer_num].shape.d;
  int vecSize = output_layer_endian.size();
  bool big_endian = layer_num >= vecSize
                        ? false
                        : output_layer_endian[layer_num];  // layer_num从0开始
  uint8_t *shift_value = bpu_handle.outputs[layer_num].shifts;
  int h_idx, w_idx, c_idx;
  HB_BPU_getHWCIndex(bpu_handle.outputs[layer_num].data_type,
                     &bpu_handle.outputs[layer_num].aligned_shape.layout,
                     &h_idx, &w_idx, &c_idx);
  uint32_t nhwc[4];
  nhwc[0] = 1;
  nhwc[1] = aligned_shape[0] * aligned_shape[h_idx];
  nhwc[2] = aligned_shape[w_idx];
  nhwc[3] = aligned_shape[c_idx];
  uint32_t real_hnum = valid_shape[h_idx];
  uint32_t real_wnum = valid_shape[w_idx];
  uint32_t real_cnum = valid_shape[c_idx];

  uint32_t w_dest_stride = real_cnum;
  uint32_t h_dest_stride = real_wnum * w_dest_stride;
  uint32_t anchor_num_per_pixel = real_cnum / class_num;
  int elem_type = bpu_handle.outputs[layer_num].data_type;
  int elem_size = (elem_type == BPU_TYPE_TENSOR_S32) ? 4 : 1;
  for (uint32_t hh = 0; hh < nhwc[1]; hh++) {
    uint32_t cur_h = hh % aligned_shape[h_idx];
    if (cur_h >= real_hnum) continue;
    uint32_t res_id_cur_hh = hh * h_dest_stride;
    for (uint32_t ww = 0; ww < real_wnum; ww++) {
      int offset = hh * nhwc[2] * nhwc[3] + ww * nhwc[3];
      auto score_line_buffer =
          reinterpret_cast<int8_t *>(raw_cls_data) + offset * elem_size;
      uint32_t res_id_cur_ww = res_id_cur_hh + ww * w_dest_stride;
      for (uint32_t anchor_id = 0; anchor_id < anchor_num_per_pixel;
           anchor_id++) {
        uint32_t res_id_cur_anchor = res_id_cur_ww + anchor_id * class_num;
        uint32_t cls_cur_c = class_num * anchor_id;
        double *temp_cls_dest = cls_dest + res_id_cur_anchor;
        double sum = 0;
        for (int cls = 0; cls < class_num; ++cls) {
          uint32_t cur_c = cls_cur_c + cls;
          if (elem_size == 4) {
            auto raw_conv_res =
                (reinterpret_cast<uint32_t *>(score_line_buffer))[cls];
            float conv_res_f =
                static_cast<float>(r_int32(raw_conv_res, big_endian)) /
                static_cast<float>(1 << shift_value[cur_c]);
            temp_cls_dest[cls] = std::exp(conv_res_f);
          } else {
            assert(elem_size == 1);
            int raw_conv_res = static_cast<int>(
                *(static_cast<int8_t *>(score_line_buffer) + cur_c));
            int exp_table_idx = (raw_conv_res + 128);
            temp_cls_dest[cls] = exptable[exp_table_idx];
          }
          sum += temp_cls_dest[cls];
        }

        double sum_threshold = filter_threshold * sum;
        temp_cls_dest[0] = -1.0;
        for (int cls = 1; cls < class_num; ++cls) {
          if (temp_cls_dest[cls] < sum_threshold) {
            temp_cls_dest[cls] = 0;
            continue;
          }
          temp_cls_dest[0] = 1;
          temp_cls_dest[cls] = temp_cls_dest[cls] / sum;
        }
      }
    }
  }
  return 0;
}
int Nms(std::vector<Detection> &dets, float thresh, const int topk,
        std::vector<Detection> &result, float score_threshold,
        bool suppress = false) {
  std::stable_sort(dets.begin(), dets.end(), Detection::greater);
  if (dets.size() > 400) {
    dets.erase(dets.begin() + 400, dets.end());
  }
  uint32_t det_num = dets.size();
  if (det_num == 0) {
    return -1;
  }

  std::vector<bool> skip(det_num, false);
  std::vector<float> areas(det_num);
  for (uint32_t i = 0; i < det_num; i++) {
    areas[i] = (dets[i].bbox.xmax - dets[i].bbox.xmin) *
               (dets[i].bbox.ymax - dets[i].bbox.ymin);
    if (dets[i].score < score_threshold) skip[i] = true;
  }
  int count = 0;

  for (size_t i = 0; count < topk && i < dets.size(); i++) {
    if (skip[i]) {
      continue;
    }
    skip[i] = true;
    ++count;

    for (size_t j = i + 1; j < dets.size(); ++j) {
      if (skip[j]) {
        continue;
      }

      if (suppress == false) {
        if (dets[i].id != dets[j].id) {
          continue;
        }
      }
      float xx1 = std::max(dets[i].bbox.xmin, dets[j].bbox.xmin);
      float yy1 = std::max(dets[i].bbox.ymin, dets[j].bbox.ymin);

      float xx2 = std::min(dets[i].bbox.xmax, dets[j].bbox.xmax);
      float yy2 = std::min(dets[i].bbox.ymax, dets[j].bbox.ymax);

      float area_intersection = (xx2 - xx1) * (yy2 - yy1);
      bool area_intersection_valid = (area_intersection > 0) && (xx2 - xx1 > 0);

      if (area_intersection_valid) {
        float o = area_intersection / (areas[j] + areas[i] - area_intersection);
        if (o > thresh) {
          skip[j] = true;
        }
      }
    }
    result.push_back(dets[i]);
  }

  return 0;
}

int SSDAux(BPU_MODEL_S bpu_handle, BPU_TENSOR_S const *output_base_vaddr,
           uint32_t layer_num, std::vector<std::vector<Anchor>> &anchors_table,
           std::vector<Detection> &result, const SsdConfig &ssd_conf,
           float score_threshold, float nms_threshold,
           const std::vector<bool> output_layer_endian, bool is_tf = false,
           bool is_performance = false) {
  uint32_t anchors_layers = anchors_table.size();
  if (anchors_layers != layer_num) {
    return -1;
  }

  std::vector<Detection> dets;
  for (uint32_t i = 0; i < layer_num; i++) {
    std::vector<Anchor> &anchors = anchors_table[i];
    int anchors_num = anchors.size();
    if (HB_SYS_isMemCachable(&(output_base_vaddr[i].data))) {
      HB_SYS_flushMemCache(&(output_base_vaddr[i].data),
                           HB_SYS_MEM_CACHE_INVALIDATE);
    }
    if (HB_SYS_isMemCachable(&(output_base_vaddr[i + layer_num].data))) {
      HB_SYS_flushMemCache(&(output_base_vaddr[i + layer_num].data),
                           HB_SYS_MEM_CACHE_INVALIDATE);
    }
    auto box_vaddr =
        reinterpret_cast<void *>(output_base_vaddr[i + layer_num].data.virAddr);
    auto cls_vaddr =
        reinterpret_cast<void *>(output_base_vaddr[i].data.virAddr);

    //        auto bboxes = (Bbox *) malloc(anchors_num * sizeof(Bbox));
    auto cls_table = reinterpret_cast<double *>(
        malloc(ssd_conf.cls_num * anchors_num * sizeof(double)));

    SoftmaxFromRawScore(bpu_handle, cls_vaddr, i,
                        ssd_conf.cls_num, cls_table, 0.01, score_threshold,
                        output_layer_endian);

    GetBboxFromRawData(bpu_handle, box_vaddr, i + layer_num, anchors, dets,
                       score_threshold, cls_table, ssd_conf,
                       output_layer_endian, is_tf);

    //        for (int j = 0; j < anchors_num; j++) {
    //            Bbox bbox = bboxes[j];
    //            auto cls_all = cls_table + j * SSD_CLASS_NUM_P1;
    //            for (int w = 1; w < SSD_CLASS_NUM_P1; w++) {
    //                if (cls_all[w] > score_threshold) {
    //                    dets.push_back(Detection(w - 1, cls_all[w], bbox));
    //                }
    //            }
    //        }
    free(cls_table);
    //        free(bboxes);
  }
  Nms(dets, nms_threshold, 200, result, score_threshold, false);

  return 0;
}
}  // namespace xstream
