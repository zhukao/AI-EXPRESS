# gdcplugin
## Intro

GdcPlugin负责将原图转换为IPM图并发送到总线。目前仅支持1280\*720的原图输入，及256\*512的IPM图输出。

## Detail

GdcPlugin监听MultiVioPlugin发来的消息，通过调用内部封装的转换库，将原图转换为IPM图。  
之后向总线推送IpmImageMessage消息，供其他plugin使用。  
生成IPM图时，复用原图的frame_id和timestamp，channel_id为原图的channel_id + 总通道数。

### Dependency
- stitch_image.h
- libstitch_image.so

## Usage

### 使用说明
**默认配置文件：** gdcplugin_config.json

**配置文件说明：**
```json
{
  "all_in_one_vio": true,
  "data_source_num": 4,
  "config_files": ["ch0.json", "ch1.json", "ch2.json", "ch3.json"],
  "channel2direction": [2, 1, 0, 3],
  "concate": []
}
all_in_one_vio: 多路图像是否是在一个viomessage中
config_files: sensor配置文件, 按通道顺序填写, 如果只写了一个则认为配置文件相同
channel2direction: 输入通道和朝向的对应关系, 按通道顺序填写, 0: Left, 1: Right, 2: Rear, 3: Front
concate: 需要拼接的通道, 空: 不拼接, 目前暂不支持拼接
```