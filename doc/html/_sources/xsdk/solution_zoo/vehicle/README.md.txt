# 车路协同方案
## 介绍
车辆结构化参考方案是地平线在车路协同领域的交付沉淀，它支持功能如下：
* 机动车相关检测：包括车辆，车牌，车前窗三种框检测，以及车体颜色，车牌颜色，车牌类型，车牌号的属性识别。
* 非机动车的检测和行人检测
* 机动车，非机动车，行人的跟踪以及框级别的融合。

效果截图：
![交叉路口](./doc/images/crossroad_scene.png "交叉路口")
![公路](./doc/images/road_scene.jpg "公路")
![高速1违法行为识别](./doc/images/expressway1.jpg "高速违法行为识别")
![高速2车辆识别](./doc/images/expressway2.jpg "高速2车辆识别")

## 能力集
### 输入
图片数据
### 输出
车路协同智能分析结果，包括以下内容：
* 车辆信息 车辆颜色、车辆类型
* 车牌信息 车牌号、车牌类型（单双牌）、车牌颜色

### 相关Method
基础method：FasterRCNNMethod、MOTMethod、CNNMethod、VoteMethod

特有method：
* FilterSkipFrameMethod 跳帧，降低终端设备的计算压力
* PlateVoteMethod 车牌投票，在某些图片中车牌不清晰时，根据策略在有限帧数内选择最清晰的车牌作为最终结果
* vehicle_plate_match 车辆车牌匹配，根据vehicle_snap_method的识别结果，把识别到的车辆、车牌信息，一一匹配

## 编译
```
bash build.sh
 ```
## 打包
 ```
bash deploy.sh
 ```
该脚本会在当前目录下创建deploy文件夹，里面包含通用库、VIO配置文件、模型及vehicle_solution目录

## 运行
将部署包拷贝到板子上，即可运行。
 ```
export LD_LIBRARY_PATH=./lib
./vehicle_solution/vehicle_solution ./configs/vio_config.json.96board ./vehicle_solution/configs/smart_config.json ./vehicle_solution/configs/hbipc_config.json -i
 ```

