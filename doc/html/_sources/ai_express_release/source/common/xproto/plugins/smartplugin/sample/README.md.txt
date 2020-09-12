# smart plugin sample

## 背景

smart_main.cpp基于smartplugin和vioplugin运行workflow，验证和展示method能力。目前vio有两种输入方式，一种是通过sensor从真实场景获取图像，另一种是在cp端采用回灌方式输入用户预先准备的图片。


## 回灌的意义

回灌就是阻断从sensor获取图片的路径，改为输入用户预先准备的图片。这样保证了每次的输入都是相同的，便于算法效果验证。另外，回灌也是人脸识别应用场景中本地图片注册入库的输入方法。

## 如何回灌

在smart_main.cpp程序中会读取vio配置vio_config.json，根据配置选择输入方式。当前配置内容如下：

```
{
 // 镜头类型，单目或双目
  "cam_type": "mono",
  // 输入源类型:
  // ipc_camera：ipc等后接场景，输入通常为bt1120
  // panel_camera: 面板机等前接场景，输入通常为mipi
  // jpeg_image_list:jpeg格式的回灌图片
  // nv12_image_list:nv12格式回灌图片
  "data_source": "ipc_camera",
  // 控制Vio送数上限，最大缓存数量
  "max_vio_buffer": 3,
  // vio时间戳类型：
  // input_coded: 通过y图的前16个字节的编码获得时间戳，通常用于ipc等后接场景
  // frame_id: 读取vio数据结构的frame_id字段作为时间戳，96board等使用该配置
  // raw_ts: 读取vio数据结构中的timestamp字段作为时间戳，面板机standalone方案使用该类型
  "ts_type": "input_coded",
  // 回灌name.list
  "file_path": "name.list",
  // jpeg回灌时图片对齐参数
  "pad_width": 1920,
  "pad_height": 1080,
  // 对应每种输入源的详细配置文件
  "vio_cfg_file": {
    "ipc_camera": "configs/vio/hb_vio.json",
    "panel_camera": "configs/vio/panel_camera.json",
    "jpeg_image_list": "configs/vio/vio_onsemi0230_fb.json",
    "nv12_image_list": "configs/vio/vio_onsemi0230_fb.json",
    "image": "configs/vio/vio_onsemi0230_fb.json"
  }
}
```

举一例说明如何进行回灌测试，如果用户需要回灌3张jpeg图片，图片在./configs/picture下,如图所示：


![picture](./doc/images/picture_dir.png)

* 1）首先到./configs/picture下生成图片列表文件，执行以下命令：
ls | sed "s:^:`pwd`/:" > ../name.list
将图片路径保存在./configs/name.list,结果如图:

![namelist](./doc/images/namelist.png)

* 2）修改vio_config.json文件
vio_coonfig.json配置文件修改如下

```
{
  "cam_type": "mono",
  "data_source": "jpeg_image_list",
  "max_vio_buffer": 3,
  "ts_type": "frame_id",
  "file_path": "configs/name.list",
  "pad_width": 1920,
  "pad_height": 1080,
  "vio_cfg_file": {
    "jpeg_image_list": "configs/vio/vio_onsemi0230_fb.json"
  }
}
```

* 3）执行./smart_main ./config/smart_config.json即可进行回灌测试。

