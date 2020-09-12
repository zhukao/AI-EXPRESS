/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2019-08-01 20:38:52
 * @Version: v0.0.1
 * @Brief: smartplugin impl based on xstream.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2019-09-29 05:04:11
 */
#include "rtspplugin/rtspplugin.h"

#include <fstream>
#include <functional>
#include <memory>
#include <string>

#include "hobotlog/hobotlog.hpp"
#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/message/pluginflow/msg_registry.h"
#include "xproto/plugin/xpluginasync.h"

#include "BasicUsageEnvironment.hh"
#include "hobotxsdk/xstream_sdk.h"
#include "horizon/vision/util.h"
#include "horizon/vision_type/vision_error.h"
#include "horizon/vision_type/vision_msg.h"
#include "horizon/vision_type/vision_type.h"
#include "horizon/vision_type/vision_type_util.h"
#include "liveMedia.hh"

#include "rtspplugin/rtspmessage.h"
#include "xproto_msgtype/vioplugin_data.h"

#include "unistd.h"

#include "mediapipemanager/mediapipemanager.h"
#include "mediapipemanager/meidapipeline.h"
// #include "mediapipemanager/meidapipelinetest.h"

// #define PIPE_TEST

namespace horizon {
namespace vision {
namespace xproto {
namespace rtspplugin {

using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;
using horizon::vision::xproto::basic_msgtype::VioMessage;

using hobot::vision::PymImageFrame;

// using horizon::vision::xproto::basic_msgtype::VioMessage;
using ImageFramePtr = std::shared_ptr<hobot::vision::ImageFrame>;
using XStreamImageFramePtr = xstream::XStreamData<ImageFramePtr>;

using xstream::InputDataPtr;
using xstream::OutputDataPtr;
using xstream::XStreamSDK;

XPLUGIN_REGISTER_MSG_TYPE(TYPE_DECODE_IMAGE_MESSAGE)
XPLUGIN_REGISTER_MSG_TYPE(TYPE_DECODE_DROP_MESSAGE)
XPLUGIN_REGISTER_MSG_TYPE(XPLUGIN_IMAGE_MESSAGE)
XPLUGIN_REGISTER_MSG_TYPE(XPLUGIN_DROP_MESSAGE)

int RtspPlugin::frame_count_ = 1;
std::mutex RtspPlugin::framecnt_mtx_;

void Convert(pym_buffer_t *pym_buffer, hobot::vision::PymImageFrame &pym_img) {
  if (nullptr == pym_buffer) {
    return;
  }
  pym_img.ds_pym_total_layer = DOWN_SCALE_MAX;
  pym_img.us_pym_total_layer = UP_SCALE_MAX;
  pym_img.frame_id = pym_buffer->pym_img_info.frame_id;
  pym_img.time_stamp = pym_buffer->pym_img_info.time_stamp;
  pym_img.context = static_cast<void *>(pym_buffer);
  for (int i = 0; i < DOWN_SCALE_MAX; ++i) {
    address_info_t *pym_addr = NULL;
    if (i % 4 == 0) {
      pym_addr = reinterpret_cast<address_info_t *>(&pym_buffer->pym[i / 4]);
    } else {
      pym_addr = reinterpret_cast<address_info_t *>(
          &pym_buffer->pym_roi[i / 4][i % 4 - 1]);
    }
    // std::cout << "dxd1 : " << pym_addr->width << std::endl;
    pym_img.down_scale[i].width = pym_addr->width;
    pym_img.down_scale[i].height = pym_addr->height;
    pym_img.down_scale[i].stride = pym_addr->stride_size;
    pym_img.down_scale[i].y_paddr = pym_addr->paddr[0];
    pym_img.down_scale[i].c_paddr = pym_addr->paddr[1];
    pym_img.down_scale[i].y_vaddr =
        reinterpret_cast<uint64_t>(pym_addr->addr[0]);
    pym_img.down_scale[i].c_vaddr =
        reinterpret_cast<uint64_t>(pym_addr->addr[1]);
  }
  for (int i = 0; i < UP_SCALE_MAX; ++i) {
    pym_img.up_scale[i].width = pym_buffer->us[i].width;
    pym_img.up_scale[i].height = pym_buffer->us[i].height;
    pym_img.up_scale[i].stride = pym_buffer->us[i].stride_size;
    pym_img.up_scale[i].y_paddr = pym_buffer->us[i].paddr[0];
    pym_img.up_scale[i].c_paddr = pym_buffer->us[i].paddr[1];
    pym_img.up_scale[i].y_vaddr =
        reinterpret_cast<uint64_t>(pym_buffer->us[i].addr[0]);
    pym_img.up_scale[i].c_vaddr =
        reinterpret_cast<uint64_t>(pym_buffer->us[i].addr[1]);
  }
  for (int i = 0; i < DOWN_SCALE_MAIN_MAX; ++i) {
    // std::cout << "dxd2 : " << pym_buffer->pym[i].width << std::endl;
    pym_img.down_scale_main[i].width = pym_buffer->pym[i].width;
    pym_img.down_scale_main[i].height = pym_buffer->pym[i].height;
    pym_img.down_scale_main[i].stride = pym_buffer->pym[i].stride_size;
    pym_img.down_scale_main[i].y_paddr = pym_buffer->pym[i].paddr[0];
    pym_img.down_scale_main[i].c_paddr = pym_buffer->pym[i].paddr[1];
    pym_img.down_scale_main[i].y_vaddr =
        reinterpret_cast<uint64_t>(pym_buffer->pym[i].addr[0]);
    pym_img.down_scale_main[i].c_vaddr =
        reinterpret_cast<uint64_t>(pym_buffer->pym[i].addr[1]);
  }
}

char eventLoopWatchVariable = 0;

int RtspPlugin::Init() {
  running_ = false;
  GetConfigFromFile("./video_box/configs/rtsp.json");
  MediaPipeManager::GetInstance().Init();
  for (int i = 0; i < channel_number_; ++i) {
    std::shared_ptr<horizon::vision::MediaPipeLine> pipeline =
        std::make_shared<horizon::vision::MediaPipeLine>(i, i);
    pipeline->Init();
//    pipeline->Start();
    MediaPipeManager::GetInstance().AddPipeLine(pipeline);
  }
  // std::shared_ptr<horizon::vision::MediaPipeLine> pipeline =
  // std::make_shared<horizon::vision::MediaPipeLine>(0, 0); pipeline->Init();
  // pipeline->Start();
  // MediaPipeManager::GetInstance().AddPipeLine(pipeline);
  // std::shared_ptr<horizon::vision::MediaPipeLine> pipeline1 =
  // std::make_shared<horizon::vision::MediaPipeLine>(1, 1); pipeline1->Init();
  // pipeline1->Start();
  // MediaPipeManager::GetInstance().AddPipeLine(pipeline1);
  // std::shared_ptr<horizon::vision::MediaPipeLine> pipeline2 =
  // std::make_shared<horizon::vision::MediaPipeLine>(2,2); pipeline2->Init();
  // pipeline2->Start();
  // MediaPipeManager::GetInstance().AddPipeLine(pipeline2);
  // std::shared_ptr<horizon::vision::MediaPipeLine> pipeline3 =
  // std::make_shared<horizon::vision::MediaPipeLine>(3,3); pipeline3->Init();
  // pipeline3->Start();
  // MediaPipeManager::GetInstance().AddPipeLine(pipeline3);

  return XPluginAsync::Init();
}

void RtspPlugin::GetDeocdeFrame(std::shared_ptr<MediaPipeLine> pipeline,
                                int channel) {
  pym_buffer_t *out_pym_buf = nullptr;
  int ret = 0;
  printf("Enter get decode frame thread000000\n");
  while (running_) {
//     usleep(1000*40);
    ret = pipeline->Output((void **)(&out_pym_buf));
    LOGW << "pipeline out grp:" << pipeline->GetGrpId() << " ret:" << ret;
    if (ret != 0) {
      LOGI << "Frame Drop";
      continue;
    }

#if 1
    std::vector<std::shared_ptr<PymImageFrame>> pym_images;
    auto pym_image_frame_ptr = std::make_shared<PymImageFrame>();
    Convert(out_pym_buf, *pym_image_frame_ptr);
    pym_image_frame_ptr->channel_id = 0;
    {
      std::lock_guard<std::mutex> lg(framecnt_mtx_);
      pym_image_frame_ptr->frame_id = frame_count_++;
    }
    pym_images.push_back(pym_image_frame_ptr);
    std::shared_ptr<VioMessage> input(
        new ImageVioMessage(pym_images, 1, 1, channel, pipeline, out_pym_buf),
        [&](ImageVioMessage *p) {
          if (p) {
            if (p->pipeline_ != nullptr) {
              LOGI << "image vio message destrct  grp:"
                   << p->pipeline_->GetGrpId()
                   << "  frame_id:" << pym_image_frame_ptr->frame_id;
              p->pipeline_->OutputBufferFree(p->slot_data_);
            }
            // p->FreeImage();
            // FreeBuffer();
            delete p;
            // ret = HB_VPS_ReleaseChnFrame(1, 0, &out_pym_buf);
          }
          p = nullptr;
        });

    LOGI << "image vio message construct  grp:" << pipeline->GetGrpId()
         << "  frame_id:" << pym_image_frame_ptr->frame_id;
    PushMsg(input);
#endif
    LOGI << "Get pipeline: " << channel << " output.";
  }
}

void RtspPlugin::GetDecodeFrame0() {

  static int count = 1;
  int ret = 0;
  VIDEO_FRAME_S pstFrame;
  pym_buffer_t out_pym_buf;
  hb_vio_buffer_t hb_vio_buf;
  hb_vio_buffer_t out_buf;
  int channel = 3;
  printf("Enter get decode frame thread\n");
  while (1) {
    ret = HB_VDEC_GetFrame(0, &pstFrame, 1000);
    if (ret != 0) {
      printf("HB_VDEC_GetFrame failed %d\n", ret);
      continue;
    }

    printf("[pstFrame]vir_ptr:%lld, %lld frame_size:[%d %d, %d]\n",
           (long long int)(pstFrame.stVFrame.vir_ptr[0]),
           (long long int)(pstFrame.stVFrame.vir_ptr[1]),
           pstFrame.stVFrame.width, pstFrame.stVFrame.height,
           pstFrame.stVFrame.size);

    std::ofstream outfile;
    // outfile.open("image_raw" + std::to_string(++count) + ".yuv",
    // std::ios::ate | std::ios::out | std::ios::binary);
    // outfile.write(reinterpret_cast<char *>(pstFrame.stVFrame.vir_ptr[0]),
    // pstFrame.stVFrame.width*pstFrame.stVFrame.height);
    // outfile.write(reinterpret_cast<char *>(pstFrame.stVFrame.vir_ptr[1]),
    // pstFrame.stVFrame.width*pstFrame.stVFrame.height/2); outfile.close();

    memset(&hb_vio_buf, 0, sizeof(hb_vio_buffer_t));
    hb_vio_buf.img_addr.addr[0] = pstFrame.stVFrame.vir_ptr[0];
    hb_vio_buf.img_addr.paddr[0] = pstFrame.stVFrame.phy_ptr[0];
    hb_vio_buf.img_addr.addr[1] = pstFrame.stVFrame.vir_ptr[1];
    hb_vio_buf.img_addr.paddr[1] = pstFrame.stVFrame.phy_ptr[1];
    hb_vio_buf.img_addr.width = pstFrame.stVFrame.width;
    hb_vio_buf.img_addr.height = pstFrame.stVFrame.height;
    hb_vio_buf.img_addr.stride_size = pstFrame.stVFrame.width;
    hb_vio_buf.img_info.planeCount = 2;
    hb_vio_buf.img_info.img_format = 8;
    hb_vio_buf.img_info.fd[0] = pstFrame.stVFrame.fd[0];
    hb_vio_buf.img_info.fd[1] = pstFrame.stVFrame.fd[1];
    hb_vio_buf.img_info.sensor_id = channel;
    hb_vio_buf.img_info.frame_id = channel;
    ret = HB_VPS_SendFrame(0, &hb_vio_buf, 1000);

    // memcpy(hb_vio_buf_2.img_addr.addr[0], pstFrame.stVFrame.vir_ptr[0],
    // 1920*1088); memcpy(hb_vio_buf_2.img_addr.addr[1],
    // pstFrame.stVFrame.vir_ptr[1], 1920*1088/2); ret = HB_VPS_SendFrame(0,
    // &hb_vio_buf_2, 1000);
    if (ret != 0) {
      printf("HB_VPS_SendFrame failed %d\n", ret);
    }
    ret = HB_VPS_GetChnFrame(0, 0, &out_buf, 1000);
    if (ret != 0) {
      printf("HB_VPS_GetChnFrame error!!!\n");
    }
    printf("image crop h:%d  w:%d id:%d\n", out_buf.img_addr.height,
           out_buf.img_addr.width, out_buf.img_info.pipeline_id);
    // outfile.open("image_crop" + std::to_string(count++) + ".yuv",
    // std::ios::ate | std::ios::out | std::ios::binary); printf("image crop
    // h:%d  w:%d\n", out_buf.img_addr.height, out_buf.img_addr.width);
    // outfile.write(reinterpret_cast<char *>(out_buf.img_addr.addr[0]),
    // out_buf.img_addr.height*out_buf.img_addr.width);
    // outfile.write(reinterpret_cast<char *>(out_buf.img_addr.addr[1]),
    // out_buf.img_addr.height*out_buf.img_addr.width/2); outfile.close();
    ret = HB_VDEC_ReleaseFrame(0, &pstFrame);
    if (ret != 0) {
      printf("HB_VDEC_ReleaseFrame failed %d\n", ret);
    }
    ret = HB_VPS_ReleaseChnFrame(0, 0, &out_buf);
    if (ret != 0) {
      printf("HB_VPS_ReleaseChnFrame error!!!\n");
    }
#if 1
    ret = HB_VPS_SendFrame(1, &out_buf, 1000);
    if (ret != 0) {
      printf("HB_VPS_SendFrame failed %d\n", ret);
    }
    ret = HB_VPS_GetChnFrame(1, 0, &out_pym_buf, 1000);
    if (ret != 0) {
      printf("HB_VPS_GetChnFrame error!!!\n");
    }
    printf("pym layer 4 h:%d  w:%d id:%d\n", out_pym_buf.pym[1].height,
           out_pym_buf.pym[1].width, out_pym_buf.pym_img_info.pipeline_id);
    // outfile.open("image_pym" + std::to_string(count++) + ".yuv",
    // std::ios::ate | std::ios::out | std::ios::binary); printf("pym layer 4
    // h:%d  w:%d\n", out_pym_buf.pym[1].height, out_pym_buf.pym[1].width);
    // outfile.write(reinterpret_cast<char *>(out_pym_buf.pym[1].addr[0]),
    // out_pym_buf.pym[1].height*out_pym_buf.pym[1].width);
    // outfile.write(reinterpret_cast<char *>(out_pym_buf.pym[1].addr[1]),
    // out_pym_buf.pym[1].height*out_pym_buf.pym[1].width/2); outfile.close();
    ret = HB_VPS_ReleaseChnFrame(0, 0, &out_buf);
    if (ret != 0) {
      printf("HB_VPS_ReleaseChnFrame error!!!\n");
    }

    std::vector<std::shared_ptr<PymImageFrame>> pym_images;
    // 从 image path 填充pvio image
    // auto ret = FillVIOImageByImagePath(pvio_image, image_path);
    // if (ret) {
    auto pym_image_frame_ptr = std::make_shared<PymImageFrame>();
    Convert(&out_pym_buf, *pym_image_frame_ptr);
    pym_image_frame_ptr->channel_id = 0;
    pym_image_frame_ptr->frame_id = count++;
    pym_images.push_back(pym_image_frame_ptr);
    // } else {
    //   std::free(pvio_image);
    //   LOGF << "fill vio image failed";
    // }
    std::shared_ptr<VioMessage> input(
        new ImageVioMessage(pym_images, 1, 1), [&](ImageVioMessage *p) {
          if (p) {
            // p->FreeImage();
            // FreeBuffer();
            // delete p;
            ret = HB_VPS_ReleaseChnFrame(1, 0, &out_pym_buf);
          }
          p = nullptr;
        });
    PushMsg(input);

    ret = HB_VPS_ReleaseChnFrame(1, 0, &out_pym_buf);
#endif
  }
  printf("Exit get decode frame thread\n");
}

void RtspPlugin::WaitToStart() {
  // // sleep(2);
  // std::thread t(&RtspPlugin::GetDeocdeFrame, this, nullptr, 0);
  // // const std::vector<std::shared_ptr<MediaPipeLine>>& pipelines =
  // MediaPipeManager::GetInstance().GetPipeLine();
  // // LOGE << "\n\nPipeline size : " << pipelines.size();
  // // for (uint32_t i = 0; i < pipelines.size(); ++i) {
  // //   std::thread t(&RtspPlugin::GetDeocdeFrame, this, pipelines[i], i);
  // // }
}

void RtspPlugin::Process() {
  // Begin by setting up our usage environment:
  scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  for (int i = 0; i < channel_number_; ++i) {
    ourRTSPClient *client = nullptr;
    client = openURL(*env, "RTSPClient", rtsp_url_[i].url.c_str(),
                     rtsp_url_[i].tcp_flag,
                     ("channel" + std::to_string(i) + ""));
    client->SetChannel(i);
    rtsp_clients_.push_back(client);
  }

  // ourRTSPClient * client = nullptr;
  // client = openURL(*env, "RTSPClient",
  // "rtsp://admin:admin123@10.64.32.172:554/0", ("channel" + std::to_string(0)
  // + ".h264")); rtsp_clients_.push_back(client); client = openURL(*env,
  // "RTSPClient",
  // "rtsp://admin:admin123@10.64.35.195:554/cam/realmonitor?channel=1&subtype=0",
  // ("channel" + std::to_string(1) + ".h264"));
  // rtsp_clients_.push_back(client);
  // client = openURL(*env, "RTSPClient",
  // "rtsp://admin:admin123@10.64.32.174:554/0", ("channel" + std::to_string(2)
  // + ".h264")); rtsp_clients_.push_back(client); client = openURL(*env,
  // "RTSPClient", "rtsp://admin:admin123@10.64.31.90:554/0", ("channel" +
  // std::to_string(3) + ".h264")); rtsp_clients_.push_back(client);
  const std::vector<std::shared_ptr<MediaPipeLine>> &pipelines =
      MediaPipeManager::GetInstance().GetPipeLine();
  LOGE << "\n\nPipeline size : " << pipelines.size();
  running_ = true;
  std::thread *t[channel_number_];
  for (uint32_t i = 0; i < pipelines.size(); ++i) {
    t[i] = new std::thread(&RtspPlugin::GetDeocdeFrame, this, pipelines[i], i);
  }
  // All subsequent activity takes place within the event loop:
  env->taskScheduler().doEventLoop(&eventLoopWatchVariable);
  for (int i = 0; i < channel_number_; ++i) {
    ourRTSPClient *client = rtsp_clients_[i];
    // operators cause crash if client is invalid
    if (rtsp_clients_stat_.at(i)) {
      client->sendTeardownCommand(*client->scs.session, NULL);
      Medium::close(client->scs.session);
    }
  }
  // client->sendTeardownCommand(*client->scs.session, NULL);
  // Medium::close(client->scs.session);
  env->reclaim();
  delete scheduler;
  for (uint32_t i = 0; i < pipelines.size(); ++i) {
    if (t[i]) {
      if (t[i]->joinable()) {
        t[i]->join();
      }
      delete t[i];
    }
  }

  // This function call does not return, unless, at some point in time,
  // "eventLoopWatchVariable" gets set to something non-zero.

  // If you choose to continue the application past this point (i.e., if you
  // comment out the "return 0;" statement above), and if you don't intend to do
  // anything more with the "TaskScheduler" and "UsageEnvironment" objects, then
  // you can also reclaim the (small) memory used by these objects by
  // uncommenting the following code:
  /*
    env->reclaim(); env = NULL;
    delete scheduler; scheduler = NULL;
  */
}

void RtspPlugin::CheckRtspState() {
  const std::vector<std::shared_ptr<MediaPipeLine>> &pipelines =
      MediaPipeManager::GetInstance().GetPipeLine();

  while (running_)
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t time_now = (uint64_t)tv.tv_sec;
    for (uint32_t i = 0; i < pipelines.size(); ++i)
    {
      if (time_now - pipelines[i]->GetlastReadDataTime() <= 5)
      {
        sleep(1);
        continue;
      }

      int channel = pipelines[i]->GetGrpId();
      LOGI << "RTSP channel:" << channel << " , 10 seconds no stream!";
      rtsp_clients_[i]->Stop();

      // reopen rtsp url
      ourRTSPClient *client = nullptr;
      client = openURL(*env, "RTSPClient", rtsp_url_[i].url.c_str(),
                      rtsp_url_[i].tcp_flag,
                      ("channel" + std::to_string(i) + ""));
      client->SetChannel(i);
      LOGI << "after reopen rtsp stream, channel:" << i;
      rtsp_clients_[i] = client;
    }
  }
}

int RtspPlugin::Start() {
  auto &pipelines =
          MediaPipeManager::GetInstance().GetPipeLine();
  for (auto& pipeline : pipelines) {
    pipeline->Start();
  }

  process_thread_ = std::make_shared<std::thread>(&RtspPlugin::Process, this);

  for (size_t idx = 0; idx < pipelines.size(); idx++) {
    auto stat = pipelines.at(idx)->CheckStat();
    if (stat < 0) {
      LOGE << "check stat fail, grp:" << idx
           << "  stat:" << stat
           << "  url:" << rtsp_url_[idx].url;
      rtsp_clients_stat_[idx] = false;
      return stat;
    } else {
      LOGW << "check stat success, grp:" << idx
           << "  stat:" << stat;
      rtsp_clients_stat_[idx] = true;
    }
  }

  check_thread_ = std::make_shared<std::thread>(
    &RtspPlugin::CheckRtspState, this);
  return 0;
}

int RtspPlugin::Stop() {
  LOGW << "RtspPlugin Stop";
  running_ = false;
  LOGW << "process_thread_ Stop";
  const std::vector<std::shared_ptr<MediaPipeLine>> &pipelines =
      MediaPipeManager::GetInstance().GetPipeLine();
  LOGW << "pipe line size: " << pipelines.size();
  for (uint32_t i = 0; i < pipelines.size(); ++i) {
    pipelines[i]->Stop();
  }

  eventLoopWatchVariable = 1;
  check_thread_->join();
  process_thread_->join();
  return 0;
}

int RtspPlugin::DecodeInit() {
  int ret = 0;
  ret = HB_SYS_Init();
  if (ret != 0) {
    printf("HB_SYS_Init failed %x\n", ret);
    return ret;
  }

  VP_CONFIG_S struVpConf;
  memset(&struVpConf, 0x00, sizeof(VP_CONFIG_S));
  struVpConf.u32MaxPoolCnt = 32;
  ret = HB_VP_SetConfig(&struVpConf);
  if (ret != 0) {
    printf("HB_VP_SetConfig failed %x\n", ret);
    return ret;
  }
  ret = HB_VP_Init();
  if (ret != 0) {
    printf("vp_init fail s32Ret = %d !\n", ret);
  }
  HB_VDEC_Module_Init();
  VDEC_CHN_ATTR_S vdec_attr;
  // VdecChnAttrInit(&vdec_attr, hb_CodecType, 1920, 1080);
  vdec_attr.enType = PT_H264;
  vdec_attr.enMode = VIDEO_MODE_FRAME;
  vdec_attr.enPixelFormat = HB_PIXEL_FORMAT_NV12;
  vdec_attr.u32FrameBufCnt = 10;
  vdec_attr.u32StreamBufCnt = 10;
  // vdec_attr.u32StreamBufSize = image_height_ * image_width_ * 3 / 2;
  vdec_attr.u32StreamBufSize = 1920 * 1088 * 3 / 2;
  vdec_attr.bExternalBitStreamBuf = HB_TRUE;
  vdec_attr.stAttrH264.bandwidth_Opt = HB_TRUE;
  vdec_attr.stAttrH264.enDecMode = VIDEO_DEC_MODE_NORMAL;
  vdec_attr.stAttrH264.enOutputOrder = VIDEO_OUTPUT_ORDER_DISP;

  VPS_GRP_ATTR_S grp_attr;
  VPS_CHN_ATTR_S chn_attr;
  memset(&grp_attr, 0, sizeof(VPS_GRP_ATTR_S));
  grp_attr.maxW = 1920;
  grp_attr.maxH = 1088;
  // VPS Init
  ret = HB_VPS_CreateGrp(0, &grp_attr);
  if (ret) {
    printf("HB_VPS_CreateGrp error!!!\n");
  } else {
    printf("created a group ok:GrpId = %d\n", 0);
  }
  // ret = HB_SYS_SetVINVPSMode(0, VIN_OFFLINE_VPS_OFFINE);
  // if (ret < 0) {
  //   printf("HB_SYS_SetVINVPSMode%d error!\n", VIN_OFFLINE_VPS_OFFINE);
  //   return ret;
  // }
  grp_attr.maxW = 1920;
  grp_attr.maxH = 1080;
  ret = HB_VPS_CreateGrp(1, &grp_attr);
  if (ret) {
    printf("HB_VPS_CreateGrp error!!!\n");
  } else {
    printf("created a group ok:GrpId = %d\n", 1);
  }
  // ret = HB_SYS_SetVINVPSMode(1, VIN_OFFLINE_VPS_OFFINE);
  // if (ret < 0) {
  //   printf("HB_SYS_SetVINVPSMode%d error!\n", VIN_OFFLINE_VPS_OFFINE);
  //   return ret;
  // }

  memset(&chn_attr, 0, sizeof(VPS_CHN_ATTR_S));
  chn_attr.enScale = 1;
  chn_attr.width = 1920;
  chn_attr.height = 1080;
  chn_attr.frameDepth = 8;

  VPS_CROP_INFO_S chn_crop_info;
  memset(&chn_crop_info, 0, sizeof(VPS_CROP_INFO_S));
  chn_crop_info.en = 1;
  chn_crop_info.cropRect.x = 0;
  chn_crop_info.cropRect.y = 0;
  chn_crop_info.cropRect.width = 1920;
  chn_crop_info.cropRect.height = 1080;

  VPS_PYM_CHN_ATTR_S pym_chn_attr;
  memset(&pym_chn_attr, 0, sizeof(VPS_PYM_CHN_ATTR_S));
  pym_chn_attr.timeout = 2000;
  pym_chn_attr.ds_layer_en = 23;
  pym_chn_attr.us_layer_en = 0;
  pym_chn_attr.frame_id = 0;
  pym_chn_attr.frameDepth = 8;
  pym_chn_attr.ds_info[5].factor = 32;
  pym_chn_attr.ds_info[5].roi_x = 0;
  pym_chn_attr.ds_info[5].roi_y = 0;
  pym_chn_attr.ds_info[5].roi_width = 900;
  pym_chn_attr.ds_info[5].roi_height = 540;
  pym_chn_attr.ds_info[6].factor = 32;
  pym_chn_attr.ds_info[6].roi_x = 0;
  pym_chn_attr.ds_info[6].roi_y = 0;
  pym_chn_attr.ds_info[6].roi_width = 960;
  pym_chn_attr.ds_info[6].roi_height = 540;

  int i = 0;
  // for (int i = 0; i < 4; ++i) {
  ret = HB_VDEC_CreateChn(i, &vdec_attr);
  if (ret != 0) {
    printf("HB_VDEC_CreateChn failed %x\n", ret);
    return ret;
  }

  // EXPECT_EQ(HB_VDEC_GetChnAttr(hb_VDEC_Chn, &hb_VencChnAttr), 0);

  ret = HB_VDEC_SetChnAttr(i, &vdec_attr); // config
  if (ret != 0) {
    printf("HB_VDEC_SetChnAttr failed %x\n", ret);
    return ret;
  }

  ret = HB_VDEC_StartRecvStream(i);
  if (ret != 0) {
    printf("HB_VDEC_StartRecvStream failed %x\n", ret);
    return ret;
  }

  ret = HB_VPS_SetChnAttr(0, i, &chn_attr);
  if (ret) {
    printf("HB_VPS_SetChnAttr error!!!\n");
  } else {
    printf("set chn Attr ok: GrpId = %d, chn_id = %d\n", 0, i);
  }
  ret = HB_VPS_SetChnCrop(0, i, &chn_crop_info);
  if (ret) {
    printf("HB_VPS_SetChnCropAttr error!!!\n");
  } else {
    printf("set chn Crop Attr ok: GrpId = %d, chn_id = %d\n", 0, i);
  }
  ret = HB_VPS_SetChnAttr(1, i, &chn_attr);
  if (ret) {
    printf("HB_VPS_SetChnAttr error!!!\n");
  } else {
    printf("set chn Attr ok: GrpId = %d, chn_id = %d\n", i, 0);
  }
  ret = HB_VPS_SetPymChnAttr(1, i, &pym_chn_attr);
  if (ret) {
    printf("HB_VPS_SetPymChnAttr error!!!\n");
  } else {
    printf("HB_VPS_SetPymChnAttr ok: grp_id = %d g_pym_chn = %d\n", 1, i);
  }
  HB_VPS_EnableChn(1, i);

  HB_VPS_EnableChn(0, i);
  // }
  ret = HB_VPS_StartGrp(0);
  if (ret) {
    printf("HB_VPS_StartGrp error!!!\n");
  } else {
    printf("start grp ok: grp_id = %d\n", 0);
  }
  ret = HB_VPS_StartGrp(1);
  if (ret) {
    printf("HB_VPS_StartGrp error!!!\n");
  } else {
    printf("start grp ok: grp_id = %d\n", 1);
  }
  return ret;
}

void RtspPlugin::GetConfigFromFile(const std::string &path) {
  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    LOGE << "Open config file " << path << " failed";
  }
  ifs >> config_;
  ifs.close();

  auto value_js = config_["channel_num"];
  if (value_js.isNull()) {
    LOGE << "Can not find key: channel_num";
  }
  LOGW << value_js;
  channel_number_ = value_js.asInt();
  for (int i = 0; i < channel_number_; ++i) {
    std::string channel("channel" + std::to_string(i));
    std::string rtsp_url = config_[channel.c_str()]["rtsp_link"].asString();
    // bool use_tcp = config_[channel.c_str()]["tcp"].asBool();
    LOGW << "channel: " << channel << " rtsp url: " << rtsp_url;
    Rtspinfo info;
    info.url = rtsp_url;
    info.tcp_flag = config_[channel.c_str()]["tcp"].asBool();
    LOGW << "channel: " << channel << " protocol tcp flag: " << info.tcp_flag;
    rtsp_url_.push_back(info);
  }

  rtsp_clients_stat_.resize(channel_number_);
}

}  // namespace rtspplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
