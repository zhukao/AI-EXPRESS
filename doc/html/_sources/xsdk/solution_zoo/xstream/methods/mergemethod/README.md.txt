# MergeMethod

MergeMethod主要用于完成检测框的融合，目前的实现是基于IOU的融合，即两个框的交并比满足条件，则为两个框赋予相同的track_id，MergeMethod目前主要用于人脸与人头框的融合以及rgb人脸框与nir人脸框的融合。

## 输入

### 人头、人脸策略

|Slot |内容 |备注 |
|:---:|:---------------------------:|:--------------:|
| 0 | face_box | 必要项|
| 1 | disappeared_face_id | 必要项|
| 2 | head_box | 必要项|
| 3 | disappeared_head_id | 必要项|
| 4 | rgb_lmk | 可选项|
| 5 | nir_lmk | 可选项|

从slot4开始，可输入rgb_lmk与nir_lmk，主要用于图像校正，通过CALIBRATION宏进行控制，默认关闭。

*备注*：此类策略主要用于人脸抓拍场景

### 人头、人脸、人体策略

|Slot |内容 |备注 |
|:---:|:---------------------------:|:--------------:|
| 0 | face_box | 必要项|
| 1 | head_box | 必要项|
| 2 | body_box | 必要项|
| 3 | disappeared_face_id | 必要项|
| 4 | disappeared_head_id | 必要项|
| 5 | disappeared_body_id | 必要项|
| 6 | body_kps | 可选项|

*备注*：此类策略主要用于室内跟踪场景

## 输出

### 人头、人脸

|Slot |内容 |备注 |
|:---:|:--------------------:|:---------------------------:|
| 0 | merged_face_box | 融合后ID|
| 1 | merged_head_box | 融合后ID|
| 2 | disappeared_track_id | 融合后ID|

### 人头、人脸、人体

|Slot |内容 |备注 |
|:---:|:--------------------:|:---------------------------:|
| 0 | merged_face_box | 融合后ID|
| 1 | merged_head_box | 融合后ID|
| 2 | merged_body_box | 融合后ID|
| 3 | disappeared_track_id | 融合后ID|

## 配置文件参数

### 人头、人脸

|字段      |描述     |默认值     |
|:-------:|:-----:|:----------:|
| match_threshold|匹配阈值，若分数低于该值则认为不构成匹配对|0.4|

### 可见光、红外

|字段      |描述     |默认值     |
|:-------:|:-----:|:----------:|
| camera_type|0代表横屏，1代表竖屏|

### 人头、人脸、人体

|字段      |描述     |默认值     |
|:-------:|:-----:|:----------:|
| merge_type|融合策略选择，人头人脸融合或者人头人体融合|head_body|
| match_threshold|匹配阈值，若分数低于该值则认为不构成匹配对|0.4|
| use_kps|是否使用人体关键点辅助人头人体匹配|true|
| double_thresh_flag|是否使用双阈值条件来去除容易产生矛盾的人头人体的匹配输出|false|
| valid_kps_score_thresh|kps的分数阈值，低于该值则认为该关键点是无效的|0.2|
| kps_cnt_threshold|有效kps的个数阈值，若低于该值则认为匹配无效|0.5|
| conflict_thresh|若同一人体与多个人头的匹配分数大于该阈值，则认为该人体存在矛盾的匹配|0.8|
| head_extend_ratio|人头框外扩比例|0.05|
