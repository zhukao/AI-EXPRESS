# AntiSpfMergeMethod
活体融合Method，用于对抓拍信息中的活体结果融合。主要为了串track id反攻击逻辑设计，即：
1. 滑动窗内判断为活体的帧超过投票阈值。
2. 当前帧判断为活体。具体参见：05 人脸串Track ID攻击策略。

当1、2同时满足时，判断为活体。其他所有状态均不判断为活体。

## 输入

|Slot |内容 |备注 |
|:---:|:---------------:|:--------------:|
0 | XStreamSnapshotInfo_List | 必要项
1 | XStreamAntiSpoofing_list | 必要项
2 | uint32_t_list | 必要项track_id_list

## 输出

|Slot |内容 |备注 |
|:---:|:--------------------:|:---------------------------:|
0 | XStreamSnapshotInfo_List | anti_spf_snapshot_list

## 补充说明
+ 内部无状态机
+ 该Method支持workflow多实例，method_info.is_thread_safe_ = true，method_info.is_need_reorder = false。

## Update History

|Date      | Ver. |Change Log |
|:-------:|:-----:|:----------:|
20191104 |N/A    | 初始版本 |

## 配置文件参数

|字段      |描述     |默认值     |
|:-----------------------:|:-----------------------------------------------------:|:------:|
anti_spf_strategy|活体融合Method工作模式，支持and和or。and：RGB和NIR取过滤条件较严格的活体值（例如：活体、非活体，合并结果为非活体）；or：RGB和NIR取过滤条件较宽松的活体值（例如：活体、非活体，合并结果为活体）输出|and
