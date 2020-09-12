# uvc plugin

## Detail
UvcPlugin监听VioPlugin、SmartPlugin，获取视频帧、智能帧。
其中智能数据经过Proto封装之后，通过HID发送给ap侧。

## Usage
### 使用说明
**默认配置文件：** uvcplugin.json

**配置文件说明：**
```
{
  "hid_file": "/dev/hidg0",      #hid系统文件，默认"/dev/hidg0"
  "smart_type": 1                #智能数据类型，0-face, 1-body, 2-vehicle；默认1
}
```

**接口调用顺序：**
```
uvclplugin = std::make_shared<uvcplugin>("uvcplugin.json");
  if (uvcplugin == NULL) {
    std::cout << "uvcplugin instance create failed" << std::endl;
    return;
  }
  ret = uvcplugin->Init();
  ret = uvcplugin->Start();
  ret = uvcplugin->Stop();
  ret = uvcplugin->Deinit();
```
**如果创建对象时没有传入配置文件，就会报错，必须传入配置文件**
