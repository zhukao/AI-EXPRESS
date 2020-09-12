/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     dump helper header
 * @author    chao.yang
 * @email     chao01.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2019.05.23
 */

#ifndef TEST_SUPPORT_HPP_
#define TEST_SUPPORT_HPP_

#include <memory>
#include <string>

#include "hobotxsdk/xstream_sdk.h"
#include "horizon/vision_type/vision_type.hpp"
#include "opencv2/opencv.hpp"

typedef std::shared_ptr<hobot::vision::ImageFrame> ImageFramePtr;
typedef hobot::vision::SnapshotInfo<xstream::BaseDataPtr>
    SnapshotInfoXStreamBaseData;
typedef std::shared_ptr<SnapshotInfoXStreamBaseData>
    SnapshotInfoXStreamBaseDataPtr;
typedef xstream::XStreamData<SnapshotInfoXStreamBaseDataPtr>
    XStreamSnapshotInfo;
typedef std::shared_ptr<XStreamSnapshotInfo> XStreamSnapshotInfoPtr;

typedef xstream::XStreamData<ImageFramePtr> XStreamImageFrame;
typedef std::shared_ptr<XStreamImageFrame> XStreamImageFramePtr;
typedef std::shared_ptr<hobot::vision::CVImageFrame> CVImageFramePtr;
typedef xstream::XStreamData<hobot::vision::BBox> XStreamBBox;
typedef xstream::XStreamData<uint32_t> XStreamUint32;
typedef xstream::XStreamData<float> XStreamFloat;

int WriteLog(const XStreamSnapshotInfoPtr &snapshot_info);

// static int SaveImg(const ImageFramePtr &img_ptr, const std::string &path);

int DumpSnap(const XStreamSnapshotInfoPtr &snapshot_info,
             std::string dir = ".");

int ConstructInput(const std::string &smart_frame,
                   const std::string &video_path, xstream::InputDataPtr &input,
                   const std::string &img_format, bool filter);

int ConstructInputInvalidUserdata(const std::string &smart_frame,
                                  const std::string &video_path,
                                  xstream::InputDataPtr &input,
                                  const std::string &img_format, bool filter);

#endif  //  TEST_SUPPORT_HPP_
