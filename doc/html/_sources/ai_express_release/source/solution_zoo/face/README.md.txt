# 人脸抓拍Demo
## 介绍
该Demo通过串联检测、跟踪、优选和抓拍4个method，共同组成了一个人脸抓拍范式。
## 能力集

|模块|输入|输出|
| --------   | -----:  | :----:  |
|FasteRCNNMethod |图像帧|人脸框、人脸关键点、人脸姿态|多任务检测 |
|MOTMethod|人脸框|带有跟踪ID号的人脸框及消失目标集合|
|GradingMethod|人脸框、人脸姿态、人脸关键点、人脸清晰度|每个目标优选分值|
|SnapshotMethod|图像帧、人脸框、目标优选分值|抓拍图列表|

## 编译
进入ai_express_release发版包
 ```
bash build.sh
 ```
## 打包部署包
 ```
bash deploy.sh
 ```
该脚本会在当前目录下创建deploy文件夹，里面包含通用库、VIO配置文件、模型及face_solution目录

## 运行
将部署包拷贝到板子上，即可运行。
 ```
export LD_LIBRARY_PATH=./lib
./face_solution/face_solution ./configs/vio_config.json.96board ./face_solution/configs/face_solution.json -i
 ```




