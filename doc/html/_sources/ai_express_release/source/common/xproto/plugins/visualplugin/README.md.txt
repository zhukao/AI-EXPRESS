# vio plugin

## Intro
> VisualPlugin

## Detail
VisualPlugin监听VioPlugin、SmartPlugin，获取视频帧、智能帧，进行编码（h264/jpeg）、Proto封装之后，推到live555服务。供客户端可以拉取数据进行播放或者存储。
RTSP形式："rtsp://10.31.43.96:556/horizonMeta?MediaType=2"

## Build

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
**默认配置文件：** visualplugin.json

**配置文件说明：**
```
{
  "auth_mode": 0,       #鉴权模式，0不鉴权，1用户密码鉴权
  "user": "admin",      #auth_mode为1时起效
  "password": "123456",
  "smart_type": 2,      #智能数据类型，会影响客户端的解码 0-face, 1-body, 2-vehicle
  "video_type": 2,      #视频帧类型 0-yuv, 1-h264, 2-jpg，目前只支持jpg
  "image_width": 1920,  #视频帧最大宽高
  "image_height": 1080,
  "layer": 4,           #使用的图层
  "data_buf_size": 3110400, #live555缓存区大小
  "packet_size": 102400,    #live555分包大小
  "jpeg_quality": 50        #JPG压缩质量
}

```

**接口调用顺序：**
```
visualplugin = std::make_shared<visualplugin>("visualplugin.json");
  if (visualplugin == NULL) {
    std::cout << "visualplugin instance create failed" << std::endl;
    return;
  }
  ret = visualplugin->Init();
  ret = visualplugin->Start();
  ret = visualplugin->Stop();
  ret = visualplugin->Deinit();
```
**如果创建对象时没有传入配置文件，就会报错，必须传入配置文件**

### 模块消息
**模块消息定义：**
本模块增加消息类型。
  
### 性能开销

### 维护人员

