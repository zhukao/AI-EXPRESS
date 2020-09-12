# Smart Plugin
Getting Started with smartplugin

## 简介
Smartplugin是基于XStream通用sdk接口开发的通用智能化sdk运行框架。
Smartplugin监听viomessage，调用XStream通用sdk接口得到智能化结果，并把智能化结果以protobuf的方式序列化之后以smartmessage的消息格式发送到总线(xproto)。

## 定制开发
定制开发需要三步：
1. 根据自己的智能化应用需要依赖的XStream method更新build.properties文件及src/xstream/method_factory.cpp;
2. 根据自己的智能化应用需要输出的智能化结果重写protobuf串行化，即重新实现SmartMessage的Serialize方法。
master分支提供了串行化输出的人脸检测框的sample程序，标准化的protobuf协议在repo xproto_msgtype/include/xproto_msgtype/protobuf/proto/x2.proto。
```c++
std::string CustomSmartMessage::Serialize() {
  // serialize smart message using defined smart protobuf.
  std::string proto_str;
  x2::FrameMessage proto_frame_message;
  auto smart_msg = proto_frame_message.mutable_smart_msg_();
  smart_msg->set_timestamp_(time_stamp);
  smart_msg->set_error_code_(0);
  // user-defined output parsing declaration.
  HobotXStream::BaseDataVector *face_boxes = nullptr;
  for (const auto &output : smart_result->datas_) {
    if (output->name_ == "face_bbox_list") {
      face_boxes = dynamic_cast<HobotXStream::BaseDataVector *>(output.get());
      for (int i = 0; i < face_boxes->datas_.size(); ++i) {
        auto face_box =
            std::static_pointer_cast<HobotXStream::XStreamData<hobot::vision::BBox>>(
                face_boxes->datas_[i]);
        LOGD << "x1: " << face_box->value.x1 << " y1: " << face_box->value.y1
             << " x2: " << face_box->value.x2 << " y2: " << face_box->value.y2
             << " track_id: " << face_box->value.id << "\n";
        auto target = smart_msg->add_targets_();
        target->set_type_("face");
        target->set_track_id_(face_box->value.id);
        auto proto_box = target->add_boxes_();
        proto_box->set_type_("face");
        auto point1 = proto_box->mutable_top_left_();
        point1->set_x_(face_box->value.x1);
        point1->set_y_(face_box->value.y1);
        auto point2 = proto_box->mutable_bottom_right_();
        point2->set_x_(face_box->value.x2);
        point2->set_y_(face_box->value.y2);
      }
    }
  }

  proto_frame_message.SerializeToString(&proto_str);
  return proto_str;
}
```

3. 将编译完成的库替换deploy/xprotocp_smart/lib/下的libsmartplugin.so。
  更新xstream workflow配置文件，默认为deploy/configs/smart_config.json
```json
  {
      "xstream_workflow_file": "configs/det_mot.json",
      "enable_profile": 0,
      "profile_log_path": "/userdata/log/profile.txt"
  }
```

- xstream_workflow_file: 指定xstream workflow配置文件;
- enable_profile: 是否使能online profile，该feature是xstream支持的feature，如果method开发中包括了profile信息可通过该开关online使能;
- profile_log_path: online profile 日志输出路径。

将xprotocp_smart部署包放在真机上，运行xproto_start.sh 即可启动智能化应用;

其中，sample/smart_main.cpp即是主程序源码，主程序启动方式为：

`smart_main  [-i/-d/-w/-f] xstream_config_file`

-i/-d/-w/-f 分别指定对应info、debug、warning、fatal日志等级。


## 智能数据解析说明
Protobuf数据格式定义见`common/xproto/msgtype/include/xproto_msgtype/protobuf/proto/x3.proto`

解析ProtoBuf智能数据FrameMessage，FrameMessage包括SmartFrameMessage、CaptureFrameMessage、StatisticsMessage、Image、Timestamp

### SmartFrameMessage
SmartFrameMessage包括Timestamp、Error_Code和Target

1. 人体智能信息
```
Target {
  string type_;             // "person"
  uint64 track_id_;         // track_id
  repeated Box boxes_ = [{
    string type_;           //  "body"、"head" 或 "face"，分别表示人脸框、人头框、人体框
    Point top_left_;        // 框左上点坐标
    Point bottom_right_;    // 框右下点坐标
    float score;
  }];
  repeated Points points_ = [Points {
    string type_;           // "body_landmarks"，表示人体骨骼点集合
    repeated Point points_;
  }];
  repeated Attributes attributes_ = [{
    string type_;           // "age"、"gender"、"face_mask", 分别表示年龄、性别、口罩
                            // "fall"、"raise_hand"、"stand"、"squat", 分别表示摔倒、举手、站立和蹲下
                            // "action"表示体感游戏

    float value_;           // 属性对应的值
    string value_string_;   // reserved
    float score_;           // 置信度
  }];
}
```

2. 车辆信息
```
Target {
  string type_;               // "vehicle"、"vehicle_capture"、"vehicle_anomaly"，分别表示车辆信息、抓拍车辆信息、抓拍车辆违法信息
  uint64 track_id_;           // track_id
  repeated Box boxes_ = [{
    string type_;             // "vehicle_box" 表示车体框
    Point top_left_;
    Point bottom_right_;
    float score;
  }];

  repeated Image imgs_ = [{   // 抓拍图像，仅抓拍车辆、抓拍违法车辆包含该信息
    bytes buf_;               // 二进制图
    string type_;             // "jpg"，图片类型
    uint32 width_;            // 图像宽度
    uint32 height_;           // 图像高度
  }];

  repeated Attributes attributes_ = [{
    string type_;             // "vehicle_type"、"vehicle_color"、"vehicle_lane_id"、"vehicle_speed"，分别表示车辆类型、颜色、所在车道、车速
                              // "vehicle_anomalys_type"，表示车辆违法类型，仅抓拍违法车辆包含该信息           

    float value_;             // 属性对应的值
    string value_string_;     // reserved
    float score_;             // 置信度
  }];

  repeated Points points_ = [Points {
    string type_;             // "vehicle_key_points"，表示车辆关键点
                              // "vehicle_location"，表示车辆物理位置
    repeated Point points_;
  }];

  repeated FloatArray float_arrays_ = [{
    string type_;             // "vehicle_gis"，表示车辆gis信息
    repeated float value_;
  }];

  repeated Target sub_targets_ = [{
    string type_;             // "plate" 表示车牌
    repeated Box boxes_ = [{
      string type_;           // "plate_box" 表示车排框
      Point top_left_;
      Point bottom_right_;
      float score;
    }];

    repeated Attributes attributes_ = [{
      string type_;           // "is_double_plate"，表示双车牌
                              // "plate_num"，表示车牌号
                              // "plate_color"，表示车牌颜色
                              // "plate_type"，表示车牌类型

      float value_;           // 属性对应的值
      string value_string_;   // reserved
      float score_;           // 置信度
    }];

    repeated Points points_ = [Points {
      string type_;           // "plate_key_points"，表示车排关键点
      repeated Point points_;
    }];
  }];
}
```

3. 行人、非机动车信息
```
Target {
  string type_;           // "person"或"no-motor"
  uint64 track_id_;       // track_id
  repeated Box boxes_ = [{
    string type_;         // "person_box"、"no-motor_box"，分别表示人体框、非机动车框
    Point top_left_;
    Point bottom_right_;
    float score;
  }];

  repeated FloatArray float_arrays_ = [{
    string type_;           // "person_gis"、"no-motor_gis"，分别表示行人、非机动车gis信息
    repeated float value_;
  }];
}
```

### StatisticsMessage
4. 车流统计信息
```
StatisticsMessage {
   repeated Attributes attributes_ = [{
      string type_;           // "trafficflow_hour_index"，表示时段
                              // "trafficflow_cycle_count"，表示每个时段周期数
                              // "trafficflow_cycle_minute"，表示第几个周期
                              // "trafficflow_vehicle_sum"，表示总过车数
                              // "trafficflow_big_vehicle_sum"，表示大车过车数
                              // "trafficflow_small_vehicle_sum"，表示小车过车数
                              // "trafficflow_mean_speed"，表示平均过车速度

      float value_;           // 属性对应的值
      string value_string_;   // reserved
      float score_;           // 置信度
    }];
}
```