/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     Method interface of xsoul framework
 * @author    chao.yang
 * @email     chao01.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.12.15
 */

#include <unistd.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <cassert>
#include <string>
#include <fstream>

#include "hobotxsdk/xstream_sdk.h"
#include "hobotxsdk/xstream_data.h"
#include "horizon/vision_type/vision_type.hpp"
#include "hobotlog/hobotlog.hpp"


typedef xstream::XStreamData<hobot::vision::BBox> XStreamBBox;

typedef xstream::XStreamData<uint32_t> XStreamUint32;

typedef std::shared_ptr<hobot::vision::ImageFrame> ImageFramePtr;
typedef xstream::XStreamData<ImageFramePtr> XStreamImageFramePtr;


struct inputContext {
  std::ofstream* pofs;
  std::string imgname;
};

class Callback {
 public:
  Callback() = default;

  ~Callback() = default;

  void OnCallback(const xstream::OutputDataPtr &output) {
    using xstream::BaseDataVector;
    std::cout << "======================" << std::endl;
    std::cout << "seq: " << output->sequence_id_ << std::endl;
    std::cout << "error_code: " << output->error_code_ << std::endl;
    std::cout << "error_detail_: " << output->error_detail_ << std::endl;
    std::cout << "datas_ size: " << output->datas_.size() << std::endl;

    assert(output->datas_.size() >= 2);
    auto &bbox_list = output->datas_[0];
  //  auto &disappeared_track_id_list = output->datas_[1];
    if (bbox_list->error_code_ < 0) {
      std::cout << "bbox_list data error: "
                << bbox_list->error_code_ << std::endl;
    }
    std::cout << "bbox_list data type_name : " << bbox_list->type_
              << " " << bbox_list->name_ << std::endl;
    auto bbox_data = std::static_pointer_cast<BaseDataVector>(bbox_list);
    std::cout << "bbox_list data data size: "
              << bbox_data->datas_.size() << std::endl;

    const inputContext* context =
            reinterpret_cast<const inputContext*>(output->context_);
    std::ofstream* ofs = context->pofs;
    *ofs << context->imgname;
    for (auto & pbbox : bbox_data->datas_) {
      assert("BBox" == pbbox->type_);
      auto bbox = std::static_pointer_cast<XStreamBBox>(pbbox);
      *ofs << " " << bbox->value.x1
           << " " << bbox->value.y1
           << " " << bbox->value.x2
           << " " << bbox->value.y2
           << " " << bbox->value.id;
    }

    *ofs << std::endl;

    delete context;
  }
};

class MotMethodParam : public xstream::InputParam {
 public:
  MotMethodParam(const std::string &method_name,
      const std::string &json_config) : InputParam(method_name) {
    content_ = json_config;
    is_json_format_ = true;
  }
  std::string Format() override { return content_; };
 private:
  std::string content_ = "";
};

struct MyImageFrame : public hobot::vision::ImageFrame {
  MyImageFrame() { type = "MyImageFrame"; }
  /// \~Chinese 图像数据
  uint64_t Data() override { return 0; }
  /// \~Chinese UV分量数据
  uint64_t DataUV() override { return 0; }
  /// \~Chinese 图片大小
  uint32_t DataSize() override { return 0; }
  /// \~Chinese UV分量大小
  uint32_t DataUVSize() override { return 0; }
  /// \~Chinese 宽度
  uint32_t Width() override { return 1920; }
  /// \~Chinese 高度
  uint32_t Height() override { return 1080; }
  /// \~Chinese 长度
  uint32_t Stride() override { return 0; }
  /// \~Chinese uv长度
  uint32_t StrideUV() override { return 0; }
};


int ConstructPvMsgFromIpcStr(xstream::BaseDataVector *boxes,
                             const std::string &pv_str,
                             const std::vector<hobot::vision::BBox>
                                     &black_area,
                             std::string* img_name, float det_thres,
                             float overlap_ratio) {
  std::istringstream ss(pv_str);
  ss >> *img_name;

  float x1 = 0, y1 = 0, x2 = 0, y2 = 0, score = 0;

  while (ss >> x1) {
    ss >> y1;
    ss >> x2;
    ss >> y2;
    ss >> score;
    // if (score >= det_thres) {
      std::shared_ptr<XStreamBBox> bbox(new XStreamBBox());
      bbox->type_ = "BBox";
      bbox->value.x1 = x1;
      bbox->value.y1 = y1;
      bbox->value.x2 = x2;
      bbox->value.y2 = y2;
      bbox->value.score = score;
      boxes->datas_.push_back(xstream::BaseDataPtr(bbox));
    // }
  }
  return 0;
}

void LogRollBackHandler(const char* filename, std::size_t size) {
  std::stringstream ss;
  // printf("\n filename:%s \n", filename);
  ss << "cp " << filename << " /userdata/log/log_backup.log";
  system(ss.str().c_str());
}

void LogConfig() {
  // 加载配置文件
  // el::Configurations conf("../../config/hobot_log.conf");
  // el::Loggers::reconfigureAllLoggers(conf);
  // 设置标记LoggingFlag::StrictLogFileSizeCheck，使MAX_LOG_FILE_SIZE最大文件限制生效
  // AddFlag(HOBOT_MAX_FILE_SIZE_FLAG);
  // 防止Fatal级别日志中断程序
  // AddFlag(HOBOT_DISABLE_ABORT_FATAL_FLAG);
  // 选择划分级别的日志
  // AddFlag(HOBOT_HIERARCHICAL_FLAG);
  // 添加标志输出STL容器添加换行,编译需要添加宏指令 -DELPP_STL_LOGGING
  // AddFlag(HOBOT_NEW_LINE_CONTAINER_FLAG);
  // 自动添加空白，<<"123"<<"456";输出123 456
  // AddFlag(HOBOT_AUTO_SPACING_FLAG);
  // 设置级别门阀值，修改参数可以控制日志输出,输出Debug级别以上的日志
  // SetLogLevel(HOBOT_LOG_DEBUG);

  // 重要现在LOGV
  // verbose级别为1，设置级别verbose为1，LOGV的日志才会输出，默认日志级别为0
  // SetVerboseLevel(1);
  // 注册回调
  // RegisterRollBackHandler(LogRollBackHandler);
  // 记录crash原因,暂时不使用
  // AddFlag(HOBOT_CRASH_REASON_FLAG);
  // 防止CHECK宏条件不满足时，中断程序
  // AddFlag(HOBOT_DISABLE_ABORT_FATAL_FLAG);
}


int main(int argc, char const *argv[]) {
  // LogConfig();
  using xstream::BaseData;
  using xstream::BaseDataPtr;
  using xstream::BaseDataVector;
  using xstream::InputData;
  using xstream::InputDataPtr;

  // 初始化sdk
  xstream::XStreamSDK* flow = xstream::XStreamSDK::CreateSDK();
  Callback callback;
  flow->SetCallback(
      std::bind(&Callback::OnCallback, &callback, std::placeholders::_1));
  flow->SetConfig("config_file", "../../config/iou_mot_2.json");
  flow->Init();

  InputDataPtr inputdata(new InputData());

  /* pass-through mode

  xstream::InputParamPtr input_param(
      new MotMethodParam("IOU_example", "pass-through"));
  inputdata->params_.push_back(input_param);

  */
  std::string data_pre = "x2_ipc";
  std::string det_file_path =
          "./" + data_pre +
          "_det_result_beta.txt";
  std::string mot_result_path =
                        "./" + data_pre +
                        "_cpp_result.txt";

  std::ofstream ofs(mot_result_path);

  std::ifstream postv_fn(det_file_path, std::ios::in);
  if (postv_fn.fail()) {
    std::cout << "Open track_result failed" << std::endl;
    return -1;
  }

  std::vector<hobot::vision::BBox> black_areas;

  int frame_num = 0;
  int frame_rate = 40*1000;
  float det_thres = 0.91;
  float overlap_ratio = 0.8;

  std::string postv_str;
  while (getline(postv_fn, postv_str)) {
    frame_num += 1;

    std::string img_name = "";
    std::cout << "Processed " << frame_num << " images." << std::endl;

    auto img = std::make_shared<MyImageFrame>();
    img->time_stamp = frame_num * frame_rate;  //单位微妙

    auto xstream_img = new XStreamImageFramePtr();
    xstream_img->type_ = "ImageFrame";
    xstream_img->name_ = "image";
    xstream_img->value = img;
    inputdata->datas_.push_back(BaseDataPtr(xstream_img));


    std::shared_ptr<BaseDataVector> face_head_box(new BaseDataVector);
    ConstructPvMsgFromIpcStr(face_head_box.get(), postv_str, black_areas,
                &img_name, det_thres, overlap_ratio);
    face_head_box->name_ = "face_head_box_list";
    inputdata->datas_.push_back(BaseDataPtr(face_head_box));

    // async MOTMethod
    inputContext* context = new inputContext;
    context->pofs = &ofs;
    context->imgname = img_name;
    inputdata->context_ = context;
    flow->AsyncPredict(inputdata);
    // sleep(1);

    // sync MOTMethod
    // auto out = flow->SyncPredict(inputdata);
    // callback.OnCallback(out);
  }

  sleep(2);
  ofs.close();
  delete flow;
  return 0;
}
