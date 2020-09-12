# FilterMethod

过滤Method，用于检测框的过滤，未通过过滤条件的bbox会被标记为FIlTERED状态。，目前支持对以下条件进行过滤：

|过滤项 |对应字段 |备注|
|---|---------------|--------------|
|检测框大小|snap_size_thr||
|三维角度|frontal_thr|通过公式：face_frontal = pitch * pitch / (a * a) + yaw * yaw / (b * b) + roll * roll / (c * c)计算，以pitch = a，yaw = b，roll = c的椭球为边界，超出边界（face_frontal > 1）被过滤。||
|图像质量|quality_thr||
|关键点置信度|lmk_thr|所有关键点置信度大于所设阈值才能通过过滤条件
|人脸置信度|pv_thr||
|边界过滤|bound_thr_w/bound_thr_h||
|黑名单区域|black_area_iou_thr/black_area_list|黑名单IOU阈值共享|
|保留最大检测框数|max_box_counts|会选择最大的几个检测框作为输出，设置为0则输出所有检测框|
|遮挡过滤|*_occluded_thr||
|外扩系数及norm方法|expand_scale/norm_method||


## 输入

|Slot |内容 |备注 |
|:---:|:---------------:|:--------------:|
0 | XStreamDisappeared_id | 可选项
1 | XStreamBBox_list | 可选项
2 | XStreamPose3D_list | 可选项
3 | XStreamLandmarks_list | 可选项
4 | blur | 可选项
5 | brightness | 可选项
6 | eye_abnormalities | 可选项
7 | mouth_abnormal | 可选项
8 | left_eye | 可选项
9 | right_eye | 可选项
10 | left_brow | 可选项
11 | right_brow | 可选项
12 | forehead | 可选项
13 | left_cheek | 可选项
14 | right_cheek | 可选项
15 | nose | 可选项
16 | mouth | 可选项
17| jaw | 可选项


## 输出

|Slot |内容 |备注 |
|:---:|:--------------------:|:---------------------------:|
0 | filter_info | 过滤错误码
1 | XStreamDisappeared_id | 可选项
2 | XStreamBBox_list | 可选项
3 | XStreamPose3D_list | 可选项
4 | XStreamLandmarks_list | 可选项
5 | blur | 可选项
6 | brightness | 可选项
7 | eye_abnormalities | 可选项
8 | mouth_abnormal | 可选项
9 | left_eye | 可选项
10 | right_eye | 可选项
12 | right_brow | 可选项
13 | forehead | 可选项
14 | left_cheek | 可选项
15 | right_cheek | 可选项
16 | nose | 可选项
17 | mouth | 可选项
18| jaw | 可选项


## 补充说明
+ 对于不同的可选项可以自由组合，但是此method的输入必须大于等于1
+ 例如对于智能帧过滤可以只输入box相关数据，对于抓拍帧需要输入disappeared_id、box、pose、landmark等
+ 内部无状态机
+ 该Method支持workflow多实例，method_info.is_thread_safe_ = true，method_info.is_need_reorder = false。

## Update History

|Date      | Ver. |Change Log |
|:-------:|:-----:|:----------:|
20191105 |N/A    | 初始版本 |
20200106 |N/A    | 重构版本 |

## 配置文件参数

|字段      |描述     |默认值     |
|:-----------------------:|:-----------------------------------------------------:|:------:|
image_width|视频帧宽度|1920
image_height|视频帧高度|1080
face_size_thr|人脸检测框大小阈值|72
head_size_thr|人头检测框大小阈值|72
body_size_thr|人体检测框大小阈值|72
face_pv_thr|人脸置信度阈值|0.98
head_pv_thr|人头置信度阈值|0.98
body_pv_thr|人体置信度阈值|0.98
face_expand_scale|人脸外扩系数，用以过滤外扩出边界的检测框|1.0
head_expand_scale|人头外扩系数，用以过滤外扩出边界的检测框|1.0
body_expand_scale|人体外扩系数，用以过滤外扩出边界的检测框|1.0
max_box_counts|最大检测框数，设置为0不对检测框数目作过滤|0
filter_with_frontal_thr|是否使用总体阈值进行过滤|false
frontal_pitch_thr|正侧椭球pitch阈值|30
frontal_yaw_thr|正侧椭球yaw阈值|40
frontal_roll_thr|正侧椭球roll阈值|0
frontal_thr|正侧脸阈值|1000
quality_thr|清晰度阈值，越小越好|0.5
lmk_thr|人脸坐标阈值|0.5
lmk_filter_num|人脸坐标点过滤数量|0
bound_thr_w|视频帧宽边界|10
bound_thr_h|视频帧高边界|10
black_area_iou_thr|黑名单区域iou阈值|0.5
black_area_list|黑名单区域，例如可配置为[[10, 10, 30, 30], [40, 40, 50, 50]]即为在两个黑名单区域被过滤|[]
white_area_list|白名单区域|[]
brightness_min|亮度过滤最小值|0
brightness_max|亮度过滤最大值|4
abnormal_thr|行为异常遮挡阈值，越小表示行为较为正常|0.5
filter_status|过滤结果状态设定，0：有效，1：被过滤，2：不可见，3：消失，4：无效|4
age_min|年龄最小值|0
age_max|年龄最大值|100
age_thr|年龄阈值|0.5
stop_id|停止输出ID值|-1
left_eye_occluded_thr|左眼遮挡阈值，越小遮挡程度越轻|0.5
right_brow_occluded_thr|右眼遮挡阈值，越小遮挡程度越轻|0.5
forehead_occluded_thr|前额遮挡阈值，越小遮挡程度越轻|0.5
left_cheek_occluded_thr|左脸遮挡阈值，越小遮挡程度越轻|0.5
right_cheek_occluded_thr|右脸遮挡阈值，越小遮挡程度越轻|0.5
nose_occluded_thr|鼻子遮挡阈值，越小遮挡程度越轻|0.5
mouth_occluded_thr|嘴巴遮挡阈值，越小遮挡程度越轻|0.5
jaw_occluded_thr|下巴遮挡阈值，越小遮挡程度越轻|0.5
input_slot|**输入数据的信息，表明每个数据的名称、类型、分组**|已有分类：id,bbox,Pose3D,landmark等，与vision type中类型一一对应，已有分组：none,face,head,body,当需要添加新的分离与分组时，需要确定代码中是否支持
err_description|错误码描述，对应字段设置为多少filter_info输出对应的错误码|
    "snap_area": -1,
    "snap_size_thr": -2,
    "expand_thr": -3,
    "frontal_thr": -4,
    "pv_thr": -5,
    "quality_thr": -6,
    "lmk_thr": -7,
    "black_list": -8,
    "big_face": -10,
    "age": -11,
    "stop_id" : -12,
    "brightness": -21,
    "abnormal_thr": -22,
    "left_eye_occluded_thr": -23,
    "right_eye_occluded_thr": -24,
    "left_brow_occluded_thr": -25,
    "right_brow_occluded_thr": -26,
    "forehead_occluded_thr": -27,
    "left_cheek_occluded_thr": -28,
    "right_cheek_occluded_thr": -29,
    "nose_occluded_thr": -30,
    "mouth_occluded_thr": -31,
    "jaw_occluded_thr": -32
