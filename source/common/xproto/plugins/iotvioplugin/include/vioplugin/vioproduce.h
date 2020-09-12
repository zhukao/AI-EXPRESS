/*
 * @Description: implement of vioplugin
 * @Author: fei.cheng@horizon.ai
 * @Date: 2019-08-26 16:17:25
 * @Author: songshan.gong@horizon.ai
 * @Date: 2019-09-26 16:17:25
 * @LastEditors: hao.tian@horizon.ai
 * @LastEditTime: 2019-10-16 15:41:22
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */

#ifndef INCLUDE_VIOPRODUCE_VIOPRODUCE_H_
#define INCLUDE_VIOPRODUCE_VIOPRODUCE_H_
#include <atomic>
#include <cstddef>
#include <future>
#include <mutex>
#include <string>
#include <thread>
#include <memory>
#include <vector>
#include <unordered_map>

#include "json/json.h"

#include "vioplugin/viomessage.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "uvc/camera.h"
#include "hbmedia/hb_comm_vdec.h"
#include "hbmedia/hb_vp_api.h"
#include "hbmedia/hb_vdec.h"
#ifdef __cplusplus
}
#endif

#define kHorizonVisionXProtoOffset 1200

namespace horizon {
namespace vision {
namespace xproto {
namespace vioplugin {

enum HorizonVioError {
  kHorizonNoInternalFree = -kHorizonVisionXProtoOffset - 1,
  kHorizonVioErrorAlreadyStart = -kHorizonVisionXProtoOffset - 2,
  kHorizonVioErrorNotStart = -kHorizonVisionXProtoOffset - 3,
  kHorizonVioErrorModeSwitch = -kHorizonVisionXProtoOffset - 5,
  kHorizonVioErrorTooShortCharArray = -kHorizonVisionXProtoOffset - 6,
};

class VioConfig {
 public:
  VioConfig() = default;
  explicit VioConfig(const std::string &path, const Json::Value &json)
      : path_(path), json_(json) {}
  std::string GetValue(const std::string &key) const;
  Json::Value GetJson() const;
  static VioConfig *GetConfig();
  bool SetConfig(VioConfig *config);

 private:
  std::string path_;
  Json::Value json_;
  static VioConfig *config_;
  mutable std::mutex mutex_;
};

class VioProduce : public std::enable_shared_from_this<VioProduce> {
 public:
  // 根据data_source不同创建不同的图像对象，/camera/jpeg/nv12等
  static std::shared_ptr<VioProduce>
  CreateVioProduce(const std::string &data_source);
  virtual ~VioProduce() {}

  // called by vioplugin, add a run job to executor, async invoke, thread pool
  int Start();
  int Stop();
  // produce inputs, subclass must be implement
  virtual int Run() = 0;
  // finish producing inputs，common use
  virtual int Finish();
  // set VioConfig
  int SetConfig(VioConfig *config);
  // viomessage can be base class
  using Listener = std::function<int(const std::shared_ptr<VioMessage> &input)>;
  // set callback function
  int SetListener(const Listener &callback);
  // static member function
  static int PadImage(HorizonVisionImage *img, uint32_t dst_width = 1920,
                      uint32_t dst_height = 1080);
  // feedback path
  HorizonVisionImageFrame *GetImageFrame(const std::string &path);
  virtual void FreeBuffer();
  bool AllocBuffer();
  virtual void WaitUntilAllDone();

 protected:
  VioProduce() : is_running_{false} {
    auto config = VioConfig::GetConfig();
    auto json = config->GetJson();
    max_vio_buffer_ = json["max_vio_buffer"].asUInt();
    // std::atomic_init(&consumed_vio_buffers_, 0);
  }
  explicit VioProduce(int max_vio_buffer) : is_running_{false} {
    max_vio_buffer_ = max_vio_buffer;
    // std::atomic_init(&consumed_vio_buffers_, 0);
  }

 protected:
  VioConfig *config_ = nullptr;
  std::function<int(const std::shared_ptr<VioMessage> &input)> push_data_cb_ =
      nullptr;
  std::atomic<bool> is_running_;
  // std::atomic<int> consumed_vio_buffers_;
  int consumed_vio_buffers_ = 0;
  int max_vio_buffer_ = 0;
  std::mutex vio_buffer_mutex_;
  std::future<bool> task_future_;
  std::string cam_type_ = "mono";
  enum class TSTYPE {
    RAW_TS,    // 读取pvio_image->timestamp
    FRAME_ID,  // 读取pvio_image->frame_id
    INPUT_CODED  // 解析金字塔0层y图的前16个字节，其中编码了timestamp。
  };
  TSTYPE ts_type_ = TSTYPE::RAW_TS;
  static const std::unordered_map<std::string, TSTYPE> str2ts_type_;

 private:
  typedef struct {
    int p_Width = 1920;
    int p_Height = 1080;
    VDEC_CHN hb_VDEC_Chn = 0;
    VDEC_CHN_ATTR_S hb_VdecChnAttr;
    int hb_BufCnt = 10;
    char* mmz_vaddr[10] = {NULL};
    uint64_t mmz_paddr[10] = {0};
  } vdec_module_context_t;
  vdec_module_context_t vdec_module_context_;

  // vio produce define a default video decoder init,
  // which can decede 1080p-jpeg img to nv12
  // child class can use this api to dec h264, h265 through override
  virtual VDEC_CHN_ATTR_S VdecChnAttrInit();

 public:
  int InitDecModule();
  int DeInitDecModule();
  int StartDecModule();
  int StopDecModule();
  int InputDecModule(const char* buf, int size);
  int GetOutputDecModule(VIDEO_FRAME_S&);
  int ReleaseOutputDecModule(VIDEO_FRAME_S& pstFrame);
};

class VioCamera : public VioProduce {
 public:
  int Run() override;

 protected:
  VioCamera() = default;
  virtual ~VioCamera() = default;

 protected:
  uint32_t sample_freq_ = 1;

 private:
  int read_time_stamp(void *addr, uint64_t *timestamp);
};

class PanelCamera : public VioCamera {
 public:
  explicit PanelCamera(const std::string &vio_cfg_file);
  virtual ~PanelCamera();

 private:
  int camera_index_ = -1;
};

class IpcCamera : public VioCamera {
 public:
  explicit IpcCamera(const std::string &vio_cfg_file);
  virtual ~IpcCamera();
};

class RawImage : public VioProduce {
 public:
  RawImage() = delete;
  explicit RawImage(const char *vio_cfg_file);
  virtual ~RawImage();
  int Run() override;
};

class ImageList : public VioProduce {
 public:
  ImageList() = delete;
  explicit ImageList(const char *vio_cfg_file);
  // todo
  template <typename T>
  bool FillVIOImageByImagePath(T *pvio_image, const std::string &img_name);
  virtual ~ImageList();
  int Run() override;
};

class JpegImageList : public ImageList {
 public:
  JpegImageList() = delete;
  explicit JpegImageList(const char *vio_cfg_file) : ImageList(vio_cfg_file) {}
  virtual ~JpegImageList() {}
};

class Nv12ImageList : public ImageList {
 public:
  Nv12ImageList() = delete;
  explicit Nv12ImageList(const char *vio_cfg_file) : ImageList(vio_cfg_file) {}
  virtual ~Nv12ImageList() {}
};

class CachedImageList : public VioProduce {
 public:
  CachedImageList() = delete;
  explicit CachedImageList(const char *vio_cfg_file);
  virtual ~CachedImageList();
  int Run() override;
};

class VideoFeedbackProduce : public VioProduce {
 public:
  VideoFeedbackProduce() = delete;
  explicit VideoFeedbackProduce(const char *vio_cfg_file);
  virtual ~VideoFeedbackProduce();
  int Run() override;
};

class UsbCam : public VioProduce {
 public:
  UsbCam() = delete;
  explicit UsbCam(const char *vio_cfg_file);
  virtual ~UsbCam() { DeInitUvc(); }
  int Run() override;

 private:
  int InitUvc(std::string dev_name);
  int DeInitUvc();
  camera_t *cam = NULL;
  int width_ = 1920;
  int height_ = 1080;
  fcc_format fcc_ = FCC_MJPEG;

  std::shared_ptr<std::thread> sp_feed_decoder_task_ = nullptr;
  std::shared_ptr<std::thread> sp_get_decoder_task_ = nullptr;

  static void got_frame_handler(struct video_frame *frame, void *user_args);
/**
 * convert_yuy2_to_nv12 - image convert from yuy2 to nv12
 * @in_frame: input frame pointer
 * @out_frame: output frame pointer
 * @width: width of the frame
 * @height: height of the frame
 *
 * Pay attention, please prepare the in_frame & out_frame by yourself.
 * And, make sure the image size is right:
 * yuy2, width * height * 2
 * nv12, width * height * 1.5
 */
  static int convert_yuy2_to_nv12(void *in_frame, void *out_frame,
                                  unsigned int width, unsigned int height);
  // 1080p nv12 image from usb cam
  static hobot::vision::BlockingQueue<std::shared_ptr<unsigned char>>
          nv12_queue_;
  static uint64 nv12_queue_len_limit_;
  // 1080p mjpeg image from usb cam
  static hobot::vision::BlockingQueue<std::vector<unsigned char>>
          jpg_queue_;
  static uint64 jpg_queue_len_limit_;
};
}  // namespace vioplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
#endif  // XPROTO_INCLUDE_XPROTO_PLUGIN_XPLUGIN_H_
