# PlateVoteMethod

PlateVoteMethod 是车牌投票策略的封装
 
## 输入

| slot id | info |
|  ---- | ----  |
|  slot0  | plate_boxes(车牌框，主要是用到里边的track_id信息)  |
| slot1 | disappeared_track_ids（消失的track_id,用于清理内部关于此id的资源）|
| slot2 | plate_infos (车牌号)|

## 输出

| slot id | info |
|  ---- | ----  |
| slot0 | plate_infos（车牌投票）|

### 补充说明

单实例不支持多线程访问，支持多实例。

### 配置文件描述

配置文件是config目录下的plate_vote_method.json，下面讲解配置里主要参数的意思。

max_slide_window_size 表示滑动窗口的大小，默认值为100.

### 策略简要描述

算法提供的策略wiki: http://wiki.hobot.cc/pages/viewpage.action?pageId=75011640&showComments=true

### 编译运行

bazel build -s :plate_vote_method --crosstool_top="@hr_bazel_tools//rules_toolchain/toolchain:toolchain" --cpu=x2j2-aarch64 --define cpu=x2j2-aarch64 --verbose_failures   --spawn_strategy=local







