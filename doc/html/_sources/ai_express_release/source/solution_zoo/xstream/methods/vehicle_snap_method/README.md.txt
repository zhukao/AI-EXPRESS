车辆抓拍及车辆与车牌关联模块

# Intro

本部分包含两个功能模块，即车辆与车牌的关联，车辆的抓拍。车辆的抓拍支持优选和过线两种抓拍模式。

# Build&Deploy

 ```
bash build.sh
 ```
```
bash deploy.sh
 ```

# Dependency
vision_type
jsoncpp
如果需要编译测试代码，则还依赖 opencv

# Usage

## 车牌与车辆的关联
API 头文件是 `include/vehicle_match_strategy/pairs_match_api.hpp`。
使用该模块时，可使用静态方法 `NewPairsMatchAPI(const std::string &config_file)` 通过
```c++
auto match_api = NewPairsMatchAPI(config_file)；
```
来新建一个处理车牌与车辆关联任务的实体。其中 `config_file` 是配置文件。样例配置文件在 `config/config_match.json`。

其中，各参数意义如下：

`image_width`: 视频的宽度

`image_height`: 视频的高度

`min_statis_frames`: 形成统计矩阵所需要的最小帧数

`work_frames`: 一个统计矩阵的工作寿命，即多少帧更新一次统计矩阵。

`overlap_threshold`: 车牌框在车辆框中的比率大于多少时，认为它们可能是同一辆车的

`nums_cells_img`: 形成统计矩阵时，图像被分为 `nums_cells_img x nums_cells_img` 个格子

`max_pair_dis`: 可能匹配的车牌与车辆之间的最小距离

`min_stat_value`: 统计矩阵有意义的最小值

构建完 API 实体，可以使用 `process` 方法处理车牌与车辆的关联。具体实例可以参考 `test/test_pair_match.cpp`。

## 车辆抓拍
API 头文件是 `include/vehicle_snap_strategy/vehicle_snap_strategy_api.hpp`。可以使用静态方法 `NewVEhicleSnapStrategyAPI(const std:: &config_file_name, const CropImage &crop_image)` 通过
```c++
auto snap_api = NewPairsMatchAPI(config_file_name, crop_image)；
```
来构建一个 API 实体。其中 `crop_image` 是一个函数指针，指向一个抠图函数，需要调用该 API 的程序根据自己的图像类型来实现，其类型定义也在该 API 头文件中。`config_file_name` 是配置文件名，样例配置在 `config/config_snap.json` 中。

其中个参数意义如下：

### `factors`
该项下定义了计算优选抓拍各项分数所需要的参数。

`norm`: {

  `min`: 归一化时所用的最小值

  `max`: 归一化时所用的最大值

}

`vehicle_szie`: {
计算车辆大小分数所需的各项参数

`weight`: 权重

`norm_fun`: 归一化函数

`valid_min`: 有效的车辆 size 最小值

`norm_factor`: 归一化因子

}

`plate_szie`: {
计算车牌大小分数所需的各项参数

`weight`: 权重

`norm_fun`: 归一化函数

`valid_min`: 有效的车牌 size 最小值

`norm_factor`: 归一化因子

}

`vehicle_det_score`: {
计算车辆检测置信度分数所需的各项参数

`weight`: 权重

`norm_fun`: 归一化函数

`min`: 检测置信度最小值

`max`: 检测置信度最大值

}

`plate_det_score`: {
计算车牌检测置信度分数所需的各项参数

`weight`: 权重

`norm_fun`: 归一化函数

`min`: 检测置信度最小值

`max`: 检测置信度最大值

}

`vehicle_visibility`: {
  计算车辆可见度分数所需的各项参数

`weight`: 权重

`norm_fun` 归一化函数

}

`vehicle_integrity`: {
计算车辆完整度所需的各项分数

`weight`: 权重

`dis_th`: 与边缘的距离小于多少像素时判定为不完整

`norm_fun` 归一化函数

}

#### `snap_contorl`
该项下定义了控制何时输出抓拍信息所需的参数

`snap_mode`: 抓拍模式，`score_selective` 优选抓拍或 `cross_line` 过线抓拍。

`score_selective`: {
  该项下定义优选抓拍所需的各项控制参数

`ignore_overlap_ratio`: 车辆在 `black_area` 中所占的比例大于多少时，认为该车辆是需要 `ignore` 的

`post_frame_threshold`: 车辆出现多少帧时，输出抓拍信息

`min_tracklet_len`: 可输出抓拍信息的最小  `tracklet` 长度

`snap_min_score`: 大于多少分数时，才输出抓拍信息

`growth_ladder`: 当一个 `tracklet` 的目标的优选分数显著大于上一个最优的目标多少时，才更新最优抓拍目标

}

`cross_line`: {
该项下定义了过线抓拍所需的各项参数

`line_width`: 线宽

`line`: 标定的线，碰到这条线时，输出抓拍信息
}

`black_area`: ignore 区域

之后使用 `process` 方法处理抓拍。具体样例可见 `test\test_snap_pipeline.cpp`

#### 额外功能
抓拍模块中提供了 `SetSnapMode(snap_mode)` 来动态设置抓拍模式。还提供了 `SetBlackArea(black_are)` 来动态设置 ignore 区域.
And `SetLine(line)` method to dynamicly set line which is used in cross line snap mode.

## 单元测试
需要把配置文件放在运行目录下的 config/ 中
./vehicle_snap_method_test

