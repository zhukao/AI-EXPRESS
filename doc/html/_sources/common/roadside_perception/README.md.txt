Road Side Perception
========

## Introduction
`roadside perception SDK`将视频检测结果，映射定位到二维地图平面上。

目前支持的转换类型包括：
* 机动车
    * 车辆bbox检测框 --> 车辆行驶方向，车辆行驶速度
    * 车轮关键点，车型，行驶方向 --> 车辆定位中心点
* 非机动车
    * 非机动车bbox检测框 --> 定位中心点，行进方向，速度
* 人
    * 人bbox检测框 --> 定位中心点，行进方向，速度

---
## Build
##### dependency
```
artifact1 third_party.google.googletest:gtest:1.0.0:zip:false:false
artifact2 com.hobot.native.hobot:hobot:1.5.1:zip:true
artifact3 com.hobot.native.hobot:hobotlog:2.1.0:zip:true
artifact4 third_party:jsoncpp:1.8.4:zip:true
```
##### Environment
The library is tested with vendor centos and toolchain gcc-4.8.5. If you use different environment, please revise your build.property.local file accordingly.
##### make
```
git clone git@gitlab.hobot.cc:iot/smart-security/changsha/roadside_perception.git
cd roadside_perception
./gradlew
mkdir build; cd build;
cmake ..
make
```
生成单元测试的二进制文件：`./bin/unit_test`
##### run unit test
```
mkdir output
./bin/unit_test
```

---
## project struct
* 头文件 include/roadside_perception/*.hpp
    * api定义 roadside_perception_api.hpp
    * data定义 roadside_perception_data.hpp
    * namespace定义 roadside_perception_namespace.hpp
* 部件：
    * common 通用部件（内外参数转换、配置读入和输出、frame缓存模块、定位通用函数）
    * completer 使用卡尔曼滤波，平稳定位，计算速度
    * locators 定位模块
    * smoother 平滑模块
    * speed 速度计算（已弃用）
* 入口函数：
    * roadside_perception_api.hpp 内的
    std::pair<Status, RoadSideData> Process(const RoadSideData& data) const;
---
## Usage
输入：逐帧传入模型输出结果，数据格式如下
```
struct RoadSideTarget {
  std::vector<float> bbox;                          // 输入bbox检测框
  std::vector< std::vector<float> > key_points;     // 输入的关键点（x1,y1,x2,y2,x3,y3,x4,y4)
  std::vector<float> key_points_score;              // 关键点得分
  TargetType target_type;                           // 机动车（vehicle）、非机动车（non-motor）、行人（pedestrian）
  uint64 track_id;
  VehicleAttribute vehicle_attribute;               // 根据target_type选填
  NonMotoAttribute nonmoto_attribute;
  PedestrianAttribute pedestrian_attribute;
};

struct VehicleAttribute {
  VehicleType vehicle_type;                         // 车型（参考roadside_perception_data.hpp/VehicleType）
  Color color;                                      // 车颜色（参考roadside_perception_data.hpp/Color）
};

```
输出：逐帧输出处理结果，数据格式如下
```
struct RoadSideTarget {
  std::vector<float> bbox;                          // 输入的关键点（x1,y1,x2,y2,x3,y3,x4,y4)
  std::vector< std::vector<float> > key_points;     // 关键点
  std::vector<float> key_points_score;              // 关键点得分
  TargetType target_type;
  double longitude;                                 // 定位结果，对应x
  double latitude;                                  // 定位结果，对应y
  KeyPointsCenter key_points_center;                // 关键点优化后输出
  uint64 track_id;
  float speed;                                      // 车速 (km/h)
  VehicleAttribute vehicle_attribute;               // 与target_type对应
  NonMotoAttribute nonmoto_attribute;
  PedestrianAttribute pedestrian_attribute;
};

struct VehicleAttribute {
  VehicleType vehicle_type;                         // 车型（参考roadside_perception_data.hpp/VehicleType）
  Color color;                                      // 车颜色（参考roadside_perception_data.hpp/Color
  float width = 1.f;                                // 车宽（尾边）
  float length = 1.f;                               // 车长（侧边）
  float orientation;                                // [0 - 360];
};

struct KeyPointsCenter {
  std::vector<float> center;                        // longitude, latitude
  std::vector<float> size;                          // width, length
  float orientation;                                // [0 - 360];
};

```

具体例子可见`test/data/test_pipline.cpp`

---
## 可配置项
路径: test/data
1. 内外参：`test_camera_calibration.json`
2. 比例尺：`test_measure_scale.json`
3. 配置文件：`test_config.conf`
4. 先验车辆大小：`test_vehicle_size.json`
```
{
    "[C]Description1": "This is an example for configuration file, please set your values as needed. Lines starting with [C] are comments.",
    "camera": {
        "calibration": "../test/data/test_camera_calibration.json",
        "measure_scale": "../test/data/test_measure_scale.json",
        "camera_location": "../test/data/test_camera_location.json",
        "video_size": "../test/data/test_video_size.txt",
        "vehicle_size": "../test/data/test_vehicle_size.json",
        "gap": "../test/data/test_gap_size.json"
    },
    "[C]location_type": "Candidates are [map, gps]",
    "location_type": "map",
    "pre_smoother": [                           // 前平滑处理
        {
            "smoother_type": "box",             // 平滑类型（box、orientation、location、kps_score、kps）
            "target_type": ["vehicle", "non-motor", "pedestrian"],  // 平滑对象
            "smoother_config": {
                "alpha": 0.5,                   // 平滑程度正相关
                "lambda": 0.001,                // 平滑程度正相关
                "cache_time": 3000              // 平滑缓存时间
            }
        },
        {
            "smoother_type": "vehicle_color",   // 投票平滑（vehicle_color、vehicle_type）
            "target_type": ["vehicle"],
            "smoother_config": {
                "cache_time": 3000
            }
        }
    ],
    "post_smoother": [                          // 后处理平滑
        {
            "smoother_type": "location",
            "target_type": ["vehicle"],
            "smoother_config": {
                "alpha": 0.7,
                "lambda": 0.001,
                "cache_time": 3000
            }
        }
    ],
    "completer": {
        "speed_limit": 500,
        "max_predict_time": 0,
        "tracklet": {
        "min_occur_times": 2,
        "max_miss_time": 3000,
        "speed": {
	        	"[C]speed metric": "Cadidates are ['km/h' or 'm/s']",
	        	"metric": "km/h",
            "time_per_display": 100,
            "smooth_alpha": 0.3
            }
        }
    },
     "speed": {
       "cache_time" : 3000,
       "[C]metric": "Cadidates are ['km/h' or 'm/s']",
       "metric": "km/h",
       "speed_limit": 200
    },
    "strategy": {
        "@down_gap": 700,
        "@up_gap": 300,
        "down_gap": 0,
        "up_gap": 0,
        "video_size": [1920,1080],
        "cache_time": 3000,
        "window": 15,
        "delta": 0.15,
        "alpha": 0.5
    }
}
```
