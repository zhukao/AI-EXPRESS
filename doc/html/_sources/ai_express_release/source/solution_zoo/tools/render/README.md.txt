# 回灌渲染工具
## 介绍

回灌效果评价的可视化手段，将逐帧的智能结果叠加到原图并渲染成视频

## 回灌阶段（设备端执行）
1. 修改solution配置文件（例如face/configs/face_solution.json）enable_result_to_json项为true，使能智能结果保存功能
2. 修改vio配置文件(例如vio_config.json.96board)为回灌方式，回灌结束后，ctrl+c中止程序，程序退出前会在当前目录生成smart_data.json文件


## 渲染阶段（PC端执行）
1. 根据回灌solution类型，修改car_render.sh或者person_render.sh脚本，指定name list/video output/source image等信息
2. 运行脚本，进行逐帧渲染，渲染结束后会在指定路径生成mp4格式视频