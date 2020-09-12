# vio plugin

## Intro
VioPlugin负责获取、转换图像数据并控制图像数据获取速率，并将图像数据或丢帧数据推送给消息总线。

## Detail
VioPlugin主要分为两部分，一部分为图像数据的获取和速率控制，图像来源可以分为SIF、JPEG回灌图片、NV12回灌图片，根据smart智能帧产生的速率改变Vio图像信息生成的速率， 由smartplugin通过收到的VIoMessage的共享指针控制每个图像数据帧的释放工作，从而保证smartplugin中处于每帧处理过程中间，图像数据不会过早释放，VioPlugin中用于速率控制的可用buffer，可以通过JSON文件进行配置，单输入图像源的情况下理论最多可达7个，当智能帧产生速率过慢，buffer耗尽时，VioPlugin产生主动丢帧数据；一部分为消息的产生和推送，消息分为图像数据消息与主动丢帧数据消息，有第一部分产生后推送至消息总线。

## Build
运行命令：
`sh cicd/scripts/build_aarch.sh`

### Dependency
- build.properties
- build.properties.local

### 编译环境设置
- build.gradle
- hobot_util.cmake

### 编译选项设置
- CMakeLists.txt

## Usage
### 使用说明
**默认配置文件：** hbipc_config.json

**配置文件说明：**
```json
{
  // 镜头类型，单目或双目
  "cam_type": "mono",
  // 输入源类型:
  // ipc_camera：ipc等后接场景，输入通常为bt1120
  // panel_camera: 面板机等前接场景，输入通常为mipi
  // jpeg_image_list:jpeg格式的回灌图片，支持多路
  // nv12_image_list:nv12格式回灌图片，支持多路
  // cached_image_list:jpeg格式的单张循环回灌，预加载到内存，支持多路
  "data_source": "ipc_camera",
  // 控制Vio送数上限，最大缓存数量
  "max_vio_buffer": 3,
  // vio时间戳类型：
  // input_coded: 通过y图的前16个字节的编码获得时间戳，通常用于ipc等后接场景
  // frame_id: 读取vio数据结构的frame_id字段作为时间戳，96board等使用该配置
  // raw_ts: 读取vio数据结构中的timestamp字段作为时间戳，面板机standalone方案使用该类型
  "ts_type": "input_coded",
  // 回灌name.list，多路的情况下为 ["name1.list", "name2.list"]
  // name1.list中的图片作为source id 0；
  // name2.list中的图片作为source id 1，以此类推
  "file_path": "name.list",
  // cached_image_list专用，将这组图片依次预加载到内存，每个图片代表一路
  // VioPlgin每次输出的VioMessage消息都引用预加载到内存中的图片数据
  "image_list": ["image0.jpeg", "image1.jpge"],
  // cached_image_list专用，输出VioMessage的间隔事件，单位毫秒
  "interval": 20,
  // jpeg回灌时图片对齐参数
  "pad_width": 1920,
  "pad_height": 1080,
  // 对应每种输入源的详细配置文件
  "vio_cfg_file": {
    "ipc_camera": "configs/vio/hb_vio.json",
    "panel_camera": "configs/vio/panel_camera.json",
    "jpeg_image_list": "configs/vio/vio_onsemi0230_fb.json",
    "nv12_image_list": "configs/vio/vio_onsemi0230_fb.json",
    "image": "configs/vio/vio_onsemi0230_fb.json",
    "cached_image_list": "configs/vio/vio_onsemi0230_fb.json"
  }
}

```

**接口调用顺序：**
```c++
vioplugin = std::make_shared<VioPlugin>("config/vio_config.json");
  if (vioplugin == NULL) {
    std::cout << "vioplugin instance create failed" << std::endl;
    return;
  }
  ret = vioplugin->Init();
  ret = vioplugin->Start();
  ret = vioplugin->Stop();
```
**如果创建对象时没有传入配置文件，就会报错，必须传入配置文件**

### 模块消息
**模块消息定义：**
```c++
class VioMessage : public xproto::XProtoMessage {
 public:
  VioMessage() = delete;
  explicit VioMessage(HorizonVisionImageFrame **image_frame, uint32_t img_num,
                      bool is_valid = true, mult_img_info_t *info = nullptr);
  explicit VioMessage(uint64_t timestamp, uint64_t seq_id);
  ~VioMessage(){};


  // image frames
  HorizonVisionImageFrame **image_ = nullptr;
  // image frames number
  uint32_t num_ = 0;
  // sequence id, would increment automatically
  uint64_t sequence_id_ = 0;
  // time stamp
  uint64_t time_stamp_ = 0;
  // is valid uri
  bool is_valid_uri_ = true;
  // free source image
  void FreeImage();
  // serialize proto
  std::string Serialize() override;
  // multi
  mult_img_info_t *multi_info_ = nullptr;
};
```
**字段说明：**

- Serialize()：框架层面的串行化接口，对于VioPlugin来说，Drop帧有串行化数据，Image帧由SmartPlugin进行处理，没有串行化功能需要。
- image_：表示为vision_type中的视频数据类型；
- sequence_id_ ：每帧视频数据产生消息的序列号；
- time_stamp_ ：每帧视频数据产生消息的时间戳；
- is_valid_uri_ ：表示每帧视频数据产生的消息是否为可用状态；
- FreeImage（）：负责vision_type数据结构与系统软件金字塔图像数据的释放工作，在VioMessage共享指针析构时自动调用；
- multi_info_：多路视频数据类型，暂未使用；


