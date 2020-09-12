# <font color=#0000ff>V2.2.0</font>

## Release Date
2020/06/13

## Features
- 适配X3模型: 基于hbdk3.7.10，提供X3版本模型;基于bpu-predict master分支，bpu-predict第一套接口进行模型预测;适配FasterRCNNMethod以及CNNMethod等预测Method，同时支持X2与X3;适配SSDMethod，同时支持X2和X3
- 适配X3 VIO: 支持X3系统软件原生vio接口，获取camera图像以及图像回灌;支持X3媒体库接口获取camera图像;适配imx327（1080p），os8a10(2160p)，s5kgm（2160p与4000x3000);封装VIOWrapper，支持X2/X3接口统一调用
- 现有Solution支持X3平台: 现有face_solution， face_recog solution，body solution, vehicle solution， face_body_multisource能够正常在X3上运行
- web统一客户端: 现有solution的结果展示切换到web 统一客户端， 能够显示视频、检测结果、属性分类、关键点结果
- 新增人体行为分析solution: 集成人体多任务检测，人体摔倒模型，支持举手、站立、下蹲等策略分析。支持X2与X3
- 新增体感游戏solution: 集成人体多任务检测，人体特征提取，人体行为三个模型，web客户端展示游戏界面，支持CrappyBird和PandaRun两个游戏。支持X2与X3
- 新增USB Camera功能: 支持将X3开发板，通过USB接口接到AP上，AP通过UVC/HID拿到图像与智能结果。





