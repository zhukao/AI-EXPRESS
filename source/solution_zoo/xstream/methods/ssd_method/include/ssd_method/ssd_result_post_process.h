//
// Copyright 2019 lijun.
//

#ifndef INCLUDE_SSDMETHOD_SSD_RESULT_POSTPROCESS_H
#define INCLUDE_SSDMETHOD_SSD_RESULT_POSTPROCESS_H

#include <sstream>
#include <string>
#include <vector>

#include "bpu_predict/bpu_predict_extension.h"
#include "ssd_method/detection_common.h"

namespace xstream {
struct SsdConfig {
  std::vector<float> stds;
  std::vector<float> means;
  std::vector<float> offset;
  std::vector<int> steps;
  std::vector<std::vector<float>> sizes;
  std::vector<std::vector<float>> ratios;
  int cls_num;
};

int LoadConfig(const char *config_file, SsdConfig *ssd_conf, size_t layer_num);

int SsdAnchorsGenerator(std::vector<std::vector<Anchor>> &anchors_table,
                        const BPU_MODEL_NODE_S *output_info,
                        const SsdConfig &ssd_conf, uint32_t num_outs,
                        bool is_tf);
int SSDAux(BPU_MODEL_S bpu_handle, BPU_TENSOR_S const *output_base_vaddr,
           uint32_t layer_num, std::vector<std::vector<Anchor>> &anchors_table,
           std::vector<Detection> &result, const SsdConfig &ssd_conf,
           float score_threshold, float nms_threshold,
           const std::vector<bool> output_layer_endian, bool is_tf,
           bool is_performance);
}  // namespace xstream
#endif  // INCLUDE_SSDMETHOD_SSD_RESULT_POSTPROCESS_H
