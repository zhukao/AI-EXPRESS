/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @File: LmkInputPredictor.cpp
 * @Brief: declaration of the LmkInputPredictor
 * @Author: zhe.sun
 * @Date: 2020-01-20 11:26:50
 * @Last Modified by: zhe.sun
 * @Last Modified time: 2020-01-20 11:26:50
 */

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include "CNNPredictor/LmkInputPredictor.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/profiler.h"
#include "horizon/vision_type/vision_type.hpp"
#include "horizon/vision_type/vision_type_common.h"
#include "opencv2/opencv.hpp"
#include "hobotxstream/image_tools.h"
#include "util/AlignFace.h"
#include "CNNConst.h"

using hobot::vision::CVImageFrame;
using hobot::vision::ImageFrame;
using hobot::vision::Landmarks;
using hobot::vision::Points;
using hobot::vision::SnapshotInfo;
typedef std::shared_ptr<ImageFrame> ImageFramePtr;

namespace xstream {
namespace CnnProc {
DEFINE_MethodCreator(lmk_pre, LmkInputPredictor())

std::vector<std::vector<BaseDataPtr>>
LmkInputPredictor::DoProcess(
    const std::vector<std::vector<BaseDataPtr> > &input,
    const std::vector<xstream::InputParamPtr> &param) {

  std::vector<std::vector<BaseDataPtr>> result(1);
  std::shared_ptr<CNNPredictorOutputData> run_data =
      std::make_shared<CNNPredictorOutputData>();
  result[0].push_back(run_data);

  run_data->md_info = &model_info_;
  run_data->input = input;
  run_data->input_type = InputType::FAKEIMAGE;

  std::size_t frame_size = run_data->input.size();
  run_data->target_nums.resize(frame_size);
//  run_data->targets_data.resize(frame_size);
  run_data->valid_targets.resize(frame_size);
  run_data->out_bufs_.resize(frame_size);
  run_data->bpu_handle_ = bpu_handle_;
  run_data->model_handle_.resize(frame_size);
  run_data->bpu_fakeimage_handle_ = fake_img_handle_;
  run_data->fake_image_ptr_.resize(frame_size);
  run_data->model_name_ = model_name_;

  for (size_t frame_idx = 0; frame_idx < frame_size; frame_idx++) {
    auto &input_data = (run_data->input)[frame_idx];

    // BaseDataVector<BaseDataVector<shared_ptr<XStreamData<shared_ptr<snap>>>>>
    auto snaps = dynamic_cast<BaseDataVector *>(input_data[0].get());
    HOBOT_CHECK(snaps);
    std::size_t person_num = snaps->datas_.size();

    // A person may has more than one snapshot
    uint32_t total_snap = 0;
    uint32_t handle_num = max_handle_num_ < 0
                              ? person_num
                              : std::min(max_handle_num_,
                                static_cast<int32_t>(person_num));

    for (std::size_t obj_idx = 0, effective_idx = 0;
         effective_idx < handle_num && obj_idx < person_num; obj_idx++) {
      auto one_person_snaps =
          dynamic_cast<BaseDataVector *>(snaps->datas_[obj_idx].get());
      if (!one_person_snaps) {
        continue;
      }
      effective_idx++;
      total_snap += one_person_snaps->datas_.size();
    }
    run_data->target_nums[frame_idx] = total_snap;
    run_data->out_bufs_[frame_idx].resize(total_snap);
    run_data->model_handle_[frame_idx].resize(total_snap);
    run_data->fake_image_ptr_[frame_idx].resize(total_snap);

    for (uint32_t obj_idx = 0, snap_idx = 0;
         obj_idx < person_num && snap_idx < total_snap;
         obj_idx++) {
      auto one_person_snaps =
          dynamic_cast<BaseDataVector *>(snaps->datas_[obj_idx].get());
      if (!one_person_snaps) {
        continue;
      }
      auto snap_num = one_person_snaps->datas_.size();
      for (uint32_t s_idx = 0; s_idx < snap_num; s_idx++) {
        auto snap = std::static_pointer_cast<
            XStreamData<std::shared_ptr<SnapshotInfo<BaseDataPtr>>>>(
            one_person_snaps->datas_[s_idx]);
        auto lmk = std::static_pointer_cast<XStreamData<Landmarks>>(
            snap->value->userdata[1]);
        // check lmk's status
        if (lmk->value.values.size() == 0) {
          LOGD << "invalid lmk";
          snap_idx++;
          continue;
        }

        auto snap_mat =
            std::static_pointer_cast<CVImageFrame>(snap->value->snap);
        Points points = snap->value->PointsToSnap(lmk->value);
        std::vector<float> face_lmks;
        for (auto &point : points.values) {
          HOBOT_CHECK(point.x <= snap_mat->Width())
              << "lmk point x large than width, x:" << point.x;
          HOBOT_CHECK(point.y <= snap_mat->Height())
              << "lmk point y large than height, y:" << point.y;
          face_lmks.push_back(point.x);
          face_lmks.push_back(point.y);
          LOGD << "lmk x:" << point.x << ", y:" << point.y;
        }

        ModelOutputBuffer bufs(model_info_, 1);
        {
          RUN_PROCESS_TIME_PROFILER(model_name_ + "_do_cnn")
          RUN_FPS_PROFILER(model_name_ + "_do_cnn")
          cv::Mat snap_bgr;
          {
            RUN_PROCESS_TIME_PROFILER(model_name_ + "_nv12Tobgr")
            RUN_FPS_PROFILER(model_name_ + "_nv12Tobgr")
            cv::cvtColor(snap_mat->img, snap_bgr, CV_YUV2BGR_NV12);
          }
          int height = model_info_.input_nhwc_[1];
          int width = model_info_.input_nhwc_[2];
          LOGD << "input w h:" << width << " " << height;
          cv::Mat face_patch_bgr(height, width, CV_8UC3);
          {
            RUN_PROCESS_TIME_PROFILER(model_name_ + "_alignface")
            RUN_FPS_PROFILER(model_name_ + "_alignface")
            if (!AlignFace(
                    face_lmks, snap_bgr, face_patch_bgr, 0, g_lmk_template)) {
              LOGD << "align face failed";
              snap_idx++;
              continue;
            }
          }

          int img_len = height * width * 3 / 2;
          uint8_t *output_data = nullptr;
          int output_size, output_1_stride, output_2_stride;
          {
            RUN_PROCESS_TIME_PROFILER(model_name_ + "_bgrTonv12")
            RUN_FPS_PROFILER(model_name_ + "_bgrTonv12")
            int ret = 0;
#ifdef USE_BGR2NV12
            ret = HobotXStreamConvertImage(face_patch_bgr.data,
                                        height * width * 3,
                                        width,
                                        height,
                                        width * 3,
                                        0,
                                        IMAGE_TOOLS_RAW_BGR,
                                        IMAGE_TOOLS_RAW_YUV_NV12,
                                        &output_data,
                                        &output_size,
                                        &output_1_stride,
                                        &output_2_stride);
#else
            cv::cvtColor(face_patch_bgr, face_patch_bgr, CV_BGR2YUV_I420);
            ret = HobotXStreamConvertImage(face_patch_bgr.data,
                                        img_len,
                                        width,
                                        height,
                                        width,
                                        width / 2,
                                        IMAGE_TOOLS_RAW_YUV_I420,
                                        IMAGE_TOOLS_RAW_YUV_NV12,
                                        &output_data,
                                        &output_size,
                                        &output_1_stride,
                                        &output_2_stride);
            HOBOT_CHECK(ret == 0) << "convert img failed";
            LOGD << "convert img success";
#endif
          }
          {
            BPUModelHandle model_handle;
            RUN_PROCESS_TIME_PROFILER(model_name_ + "_runmodel")
            RUN_FPS_PROFILER(model_name_ + "_runmodel")
            if (RunModelFromImage(output_data,
                                  output_size,
                                  bufs.out_bufs_.data(),
                                  model_info_.output_layer_size_.size(),
                                  &model_handle,
                &(run_data->fake_image_ptr_[frame_idx][snap_idx]))
                == -1) {
              HobotXStreamFreeImage(output_data);
              snap_idx++;
              continue;
            }
            run_data->model_handle_[frame_idx][snap_idx] = model_handle;
            run_data->valid_targets[frame_idx].push_back(snap_idx);
            LOGD << "RunModelFromImage success";
          }
          HobotXStreamFreeImage(output_data);
        }
        run_data->out_bufs_[frame_idx][snap_idx] = bufs;
        snap_idx++;
      }
    }
  }

  return result;
}
}  // namespace CnnProc
}  // namespace xstream
