# 人脸&人体多输入多workflow方案
## 介绍

该Solution集成了Face和Body两个能力集，具体请看对应的Solution。

该Solution面向AI盒子场景，支持多路输入和多Workflow，并可以通过配置文件选择哪路输入进入到哪个Workflow中

## 编译

```
bash build.sh
```
## 打包
 ```
bash deploy.sh
 ```
该脚本会在当前目录下创建deploy文件夹，里面包含通用库、VIO配置文件、模型及face_body_multisource目录

## 配置

比如一个Solution接收两路输入，其中第一路进入到人脸抓拍的Workflow中，同时第一路和第二路都输入到人体检测Workflow，典型的配置如下所示：

```json
{
    "workflows": [
        {
            "xstream_workflow_file": "./face_body_multisource/configs/face_snapshot.json",
            "source_list": [
                0
            ],
            "overwrite": true,
            "enable_profile": 0,
            "profile_log_path": "/userdata/log/face.txt"
        },
        {
            "xstream_workflow_file": "./face_body_multisource/configs/body_detection.json",
            "source_list": [
                0, 1
            ],
            "overwrite": true,
            "enable_profile": 0,
            "profile_log_path": "/userdata/log/body.txt"
        }
    ],
    "enable_result_to_json": false
}
```

其中：

+ `workflows`： 表示当前solution使用哪些Workflow；
  + `xstream_workflow_file`：Workflow的配置文件；
  + `source_list`：输入到该Workflow的source id，每一个source id代表一路，可以支持；
  + `overwrite`：是否覆盖对应Workflow中对应的`source_number`字段，一般为`true`，不需要修改；
  + `enable_profile`：是否启用profile功能；
  + `profile_log_path`：保存profile日志的路径。
+ `enable_result_to_json`：表示是否将智能结果保存到json文件。

## 运行

将部署包拷贝到板子上，即可运行。
 ```
export LD_LIBRARY_PATH=./lib
./face_body_multisource/face_body_multisource ./configs/vio_config.json.96board ./face_body_multisource/configs/face_body_solution.json -i
 ```