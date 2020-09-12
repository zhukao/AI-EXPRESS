# MOTMethod

## Intro
多目标跟踪Method，用于检测框的跟踪、ID分配。

## Build
```shell
# sh cicd/scripts/build_linux.sh, 默认编译release版，sh cicd/scripts/build_linux.sh debug 编译debug版
# 打包example程序与依赖的库：sh cicd/scripts/copy_runtime.sh 然后将打包好的release文件夹拷贝到开发板上就可以运行了。
```
## Usage
### Example
//todo
### 输入/输出
#### 输入

|Slot |内容 |备注 |
|:---:|:---------------:|:--------------:|
0 | XStreamBBox_list | 必要项

#### 输出

|Slot |内容 |备注 |
|:---:|:--------------------:|:---------------------------:|
0 | XStreamBBox_list | 带track_id
1 | XStreamUint32_List | disappeared_track_id_list

### 配置文件

|字段      |描述     |默认值     |
|:-----------------------:|-----------------------------------------------------|:------:|
tracker_type|MOT工作模式，目前仅支持IOU based MOT|IOU
device|设备名称，若设置为X1，最大track_id为255，用以比对一致性|X2
update_no_target_predict|无检测框输入更新状态机track预测框，设置为true对主体连续运动且有遮挡的场景效果好，设置为false对主体不移动且有遮挡的场景效果好|false
support_hungarian|匈牙利匹配开关，打开匈牙利匹配id召回多，准确率下降；关闭则id召回少，准确率提升|false
need_check_merge|每组输入框IOU大于一定阈值做融合，该操作会影响输出数量，检测框融合多在检测模块完成，一般情况置为false|false
original_bbox|是否使用卡尔曼滤波器预测框，true为不使用，输出原始框坐标|true
max_track_target_num|状态机保存最大track数|512
max_det_target_num|输入框最大计算数|512
vanish_frame_count|消失帧数|30
time_gap|帧间隔时间|40

### 补充说明
+ 内部有状态机来存储每个tracklet
+ 该Method支持workflow多实例，method_info.is_thread_safe_ = false，method_info.is_need_reorder = true。

