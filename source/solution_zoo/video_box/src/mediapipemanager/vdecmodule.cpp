/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author:
 * @Mail: @horizon.ai
 */
#include "mediapipemanager/vdecmodule.h"

#include "hb_vdec.h"
#include "hb_vio_interface.h"
#include "hobotlog/hobotlog.hpp"

namespace horizon {
namespace vision {
std::once_flag VdecModule::flag_;

VdecModule::VdecModule() : group_id_(-1), timeout_(40), frameDepth_(-1) {
  std::call_once(flag_, []() {
    int ret = HB_VDEC_Module_Init();
    if (ret != 0) {
      LOGF << "HB_VDEC_Module_Init Failed. ret = " << ret;
    }
  });
}

VdecModule::~VdecModule() {}

int VdecModule::Init(uint32_t group_id, const PipeModuleInfo *module_info) {
  int ret = 0;
  group_id_ = group_id;
  frameDepth_ = module_info->frame_depth;
  buffers_.resize(frameDepth_);
  VDEC_CHN_ATTR_S vdec_attr;
  memset(&vdec_attr, 0, sizeof(VDEC_CHN_ATTR_S));
  vdec_attr.enType = PT_H264;
  vdec_attr.enMode = VIDEO_MODE_FRAME;
  //vdec_attr.enMode = VIDEO_MODE_STREAM;
  vdec_attr.enPixelFormat = HB_PIXEL_FORMAT_NV12;
  vdec_attr.u32FrameBufCnt = frameDepth_;
  vdec_attr.u32StreamBufCnt = frameDepth_;  // 5;
  LOGI << "vdec_attr.u32StreamBufCnt:" << vdec_attr.u32StreamBufCnt;
  vdec_attr.u32StreamBufSize = 768 * 1024;  // 1920 * 1088 * 3 / 2;
  LOGI << "vdec_attr.u32StreamBufSize:" << vdec_attr.u32StreamBufSize;
  vdec_attr.bExternalBitStreamBuf = HB_TRUE;
  vdec_attr.stAttrH264.bandwidth_Opt = HB_TRUE;
  vdec_attr.stAttrH264.enDecMode = VIDEO_DEC_MODE_NORMAL;
  vdec_attr.stAttrH264.enOutputOrder = VIDEO_OUTPUT_ORDER_DISP;
  ret = HB_VDEC_CreateChn(group_id_, &vdec_attr);
  if (ret != 0) {
    LOGE << "HB_VDEC_CreateChn Failed. ret = " << ret;
    return ret;
  }
  ret = HB_VDEC_SetChnAttr(group_id_, &vdec_attr);
  if (ret != 0) {
    LOGE << "HB_VDEC_SetChnAttr Failed. ret = " << ret;
    return ret;
  }
  return ret;
}

int VdecModule::Start() {
  int ret = 0;
  ret = HB_VDEC_StartRecvStream(group_id_);
  if (ret != 0) {
    LOGE << "HB_VDEC_StartRecvStream Failed. ret = " << ret;
    return ret;
  }
  return ret;
}

int VdecModule::Input(void *data) {
  int ret = 0;
  VIDEO_STREAM_S *pstStream = (VIDEO_STREAM_S *)data;
  ret = HB_VDEC_SendStream(group_id_, pstStream, timeout_);
  if (ret != 0) {
    LOGE << "HB_VDEC_SendStream Failed. ret = " << ret;
  }
  return ret;
}

int VdecModule::Output(void **data) {
  int ret = 0;
  // hb_vio_buffer_t *hb_vio_buf = (hb_vio_buffer_t *)data;
  uint32_t index = buffer_index_ % frameDepth_;
  //ret = HB_VDEC_GetFrame(group_id_, &buffers_[index], -1);
  ret = HB_VDEC_GetFrame(group_id_, &buffers_[index], timeout_);
  if (ret != 0) {
    data = nullptr;
    LOGE << "HB_VDEC_GetFrame Failed. ret = " << ret;
    return ret;
  }
  LOGW << "HB_VDEC_GetFrame frame width: " << buffers_[index].stVFrame.width
       << " frame height: " << buffers_[index].stVFrame.height
       << " frame size: " << buffers_[index].stVFrame.size;

  *data = &buffers_[index];
  buffer_index_++;
  // LOGW << "HB_VDEC_GetFrame frame width: " << video_frame_.stVFrame.width
  //      << " frame height: " << video_frame_.stVFrame.height
  //      << " frame size: " << video_frame_.stVFrame.size;
  // memset(hb_vio_buf, 0, sizeof(hb_vio_buffer_t));
  // hb_vio_buf->img_addr.addr[0] = video_frame_.stVFrame.vir_ptr[0];
  // hb_vio_buf->img_addr.paddr[0] = video_frame_.stVFrame.phy_ptr[0];
  // hb_vio_buf->img_addr.addr[1] = video_frame_.stVFrame.vir_ptr[1];
  // hb_vio_buf->img_addr.paddr[1] = video_frame_.stVFrame.phy_ptr[1];
  // hb_vio_buf->img_addr.width = video_frame_.stVFrame.width;
  // hb_vio_buf->img_addr.height = video_frame_.stVFrame.height;
  // hb_vio_buf->img_addr.stride_size = video_frame_.stVFrame.width;
  // hb_vio_buf->img_info.planeCount = 2;
  // hb_vio_buf->img_info.img_format = 8;
  // hb_vio_buf->img_info.fd[0] = video_frame_.stVFrame.fd[0];
  // hb_vio_buf->img_info.fd[1] = video_frame_.stVFrame.fd[1];
  return ret;
}

int VdecModule::OutputBufferFree(void *data) {
  int ret = 0;
  if (data == nullptr)
    return -1;
  ret = HB_VDEC_ReleaseFrame(group_id_, (VIDEO_FRAME_S *)data);
  if (ret != 0) {
    LOGE << "HB_VDEC_ReleaseFrame Failed. ret = " << ret;
    return ret;
  }
  return ret;
}

int VdecModule::Stop() {
  int ret = 0;
  LOGE << "Vdec Stop id: " << group_id_;
  ret = HB_VDEC_StopRecvStream(group_id_);
  if (ret != 0) {
    LOGE << "HB_VDEC_StopRecvStream Failed. ret = " << ret;
    return ret;
  }
  ret = HB_VDEC_DestroyChn(group_id_);
  if (ret != 0) {
    LOGE << "HB_VENC_DestroyChn Failed. ret = " << ret;
    return ret;
  }
  return ret;
}

int VdecModule::DeInit() {
  int ret = 0;
  ret = HB_VDEC_DestroyChn(group_id_);
  if (ret != 0) {
    LOGE << "HB_VDEC_DestroyChn Failed. ret = " << ret;
    return ret;
  }
  return ret;
}

}  // namespace vision
}  // namespace horizon