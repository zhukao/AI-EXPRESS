# cnnmethod

## Intro

cnn预测Method。目前支持人脸特征、3dpose、lmk、双目活体、人脸质量、年龄、性别、车型、车颜色、车牌号、摔倒等模型。

## Build
```shell
# sh cicd/scripts/build_linux.sh, 默认编译release版，sh cicd/scripts/build_linux.sh debug 编译debug版
# 打包example程序与依赖的库：sh cicd/scripts/copy_runtime.sh 然后将打包好的release文件夹拷贝到开发板上就可以运行了。
```

## Usage
### Example
example的编译和运行：

```shell
# sh cicd/build_linux.sh
# scp -r build/bin username@x2_pad_ip:/run/path
# ssh username@x2_pad_ip
# cd /run/path
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./
# ./CNNMethod_example get_model_info config/models/PanelBoard.hbm config/configs/bpu_config.json
# ./CNNMethod_example do_fb_det_cnn pose_lmk config/det_cnn_pose_lmk.json config/vio_config/vio_onsemi0230_fb.json data/det_cnn/det_cnn_list.txt data/det_cnn/det_cnn_out.txt
# ./CNNMethod_example do_fb_rect_cnn anti_spf  config/anti_spf.json config/vio_config/vio_onsemi0230_fb.json data/rect_cnn/anti_spf/img_lst.txt data/rect_cnn/anti_spf/anti_spf_out.txt
```

### 输入/输出

#### 输入

车牌识别

| slot | 内容    | 备注               |
| ---- | ------- | ------------------ |
| 0    | rois    | 需要做预测的检测框 |
| 1    | pyramid | 图像的金字塔数据   |
| 2    | match_rois| 车辆车牌融合后的车牌框|

摔倒

| slot | 内容    | 备注               |
| ---- | ------- | ------------------ |
| 0    | rois    | 需要做预测的检测框 |
| 1    | kpses   | 需要做预测的关键点   |
| 2    | disappeared_track_ids | 消失的track ids |

其他

| slot | 内容    | 备注               |
| ---- | ------- | ------------------ |
| 0    | rois    | 需要做预测的检测框 |
| 1    | pyramid | 图像的金字塔数据   |

#### 输出

车牌识别模型

| slot | 内容     | 备注                       |
| ---- | -------- | -------------------------- |
| 0    | plate_num| 车牌号，类型为vector<int>|

车型车颜色识别模型

| slot | 内容     | 备注                       |
| ---- | -------- | -------------------------- |
| 0    | feature| 车型或车颜色的识别结果，类型为int|

活体模型

| slot | 内容     | 备注                       |
| ---- | -------- | -------------------------- |
| 0    | anti_spf | 活体值，包括value和score   |
| 1    | norm_roi | 经过norm_method处理后的roi |

人脸特征值

| slot | 内容         | 备注       |
| ---- | ------------ | ---------- |
| 0    | face_feature | 人脸特征值 |

人脸质量

| slot | 内容              | 备注           |
| ---- | ----------------- | -------------- |
| 0    | Blur              | 清晰度         |
| 1    | Brightness        | 亮度           |
| 2    | Eye_Abnormalities | 眼睛表情       |
| 3    | Mouth_Abnormal    | 嘴部表情       |
| 4    | Left_Eye          | 左眼可见区域   |
| 5    | Right_Eye         | 右眼可见区域   |
| 6    | Left_Brow         | 左眉毛可见区域 |
| 7    | Right_Brow        | 右眉毛可见区域 |
| 8    | ForeHead          | 额头可见区域   |
| 9    | Left_Cheek        | 左脸颊可见区域 |
| 10   | Right_Cheek       | 右脸颊可见区域 |
| 11   | Nose              | 鼻子可见区域   |
| 12   | Mouth             | 嘴部可见区域   |
| 13   | Jaw               | 下巴可见区域   |

pose+lmk

| slot | 内容 | 备注 |
| ---- | ---- | ---- |
| 0    | lmk  |      |
| 1    | pose |      |

摔倒

| slot | 内容 | 备注 |
| ---- | ---- | ---- |
| 0    | fall_list  | 摔倒值，包括value和score |

### 配置文件

```json
{
  "model_name": "faceAntiSpfRGB",
  "model_version": "x2.1.0.11",
  "model_file_path": "../models/PanelBoard.hbm",
  "bpu_config_path": "../bpu_config/bpu_config.json",
  "in_msg_type": "img",
  "norm_method": "norm_by_lside_square",
  "filter_method": "no_filter",
  "expand_scale": 1.5,
  "post_fn": "antispoofing",
  "threshold": 0.1,
  "max_handle_num": -1,
  "output_size": 2
}
```

| 配置名          | 说明                                 | 备注                                                         |
| --------------- | ------------------------------------ | ------------------------------------------------------------ |
| model_name      | 编译模型时指定的模型名字             |                                                              |
| model_version   | 模型版本号                           |                                                              |
| model_file_path | 模型文件地址                         |                                                              |
| bpu_config_path | bpu的配置文件地址                    |                                                              |
| in_msg_type     | 模型的处理方式（resizer或者pyramid或者ddr） | rect/img(resizer/pyramid)/lmk_seq                             |
| norm_method     | pyramid方式必填                      | norm_by_width_length<br />norm_by_width_ratio<br />norm_by_height_rario<br />norm_by_lside_ratio<br />norm_by_height_length<br />norm_by_lside_length<br />norm_by_lside_square<br />norm_by_diagonal_square<br />norm_by_nothing |
| filter_method   | pyramid方式必填                      | out_of_range<br />no_filter                                  |
| expand_scale    | pyramid方式必填                      | 外扩系数                                                     |
| post_fn         | 后处理方式                           | face_feature<br />antispoofing<br />lmk_pose<br />age_gender<br />face_quality<br />act_det |
| threshold       | 阈值                                 |                                                              |
| input_shift     | 输入转浮点时参数                      |                                                              |
| seq_len         | 输入序列长度                          |                                                              |
| stride          | 序列步长                             |  |
| max_gap         | 步长允许误差范围                      |  |
| buf_len         | 缓存buffer长度                       |                                                              |
| norm_kps_conf   | 是否强制进行关键点置信度归一化       |                                                              |
| kps_norm_scale  | 关键点置信度归一化参数                |                                                              |
| merge_groups    | 类别融合                             | 字符串，需要保证格式为"[group1_idx1,group1_idx2];[group2_idx1,group_idx2]"  |
| target_group_idx| 目标类别index                        |                                                              |
| max_handle_num       | 最大处理数量             |    负数表示无限制                                                 |
| output_size     | 输出槽的个数                         |                                                              |

