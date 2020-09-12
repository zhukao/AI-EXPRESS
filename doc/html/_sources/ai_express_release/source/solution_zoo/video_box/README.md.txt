# 人体结构化范式方案
## 介绍
人体结构化参考方案，我们核心目标是对人检测与跟踪。我们选取人头检测，人脸检测，人体检测，人脸关键点，人脸姿态，人体关键点等数个产品模型，并通过MOT人头跟踪，MOT人体跟踪，MOT人体跟踪实现对人体分别进行跟踪，随后通过融合策略，实现三框的融合，最终完成对一个人体目标跟踪。
## 能力集

|模块|输入|输出|备注|
| --------   | -----:  | :----:  |:----: |
|FasteRCNNMethod |图像帧|人脸框、人头框、人体框、人脸关键点、人脸姿态、骨骼关键点|多任务检测 |
|MOTMethod|人脸框|带有跟踪ID号的人脸框及消失目标集合||
|MOTMethod|人头框|带有跟踪ID号的人头框及消失目标集合||
|MOTMethod|人体框|带有跟踪ID号的人体框及消失目标集合||
|MergeMethod|人脸框、人头框、人体框、消失ID|融合后的人员ID||

## 编译
 ```
bash build.sh
 ```
## 打包部署包
 ```
bash deploy.sh
 ```
该脚本会在当前目录下创建deploy文件夹，里面包含通用库、VIO配置文件、模型及body_solution目录

## 运行
将部署包拷贝到板子上，即可运行。
 ```
export LD_LIBRARY_PATH=./lib
./body_solution/body_solution ./configs/vio_config.json.96board ./body_solution/configs/body_solution.json -i
 ```

