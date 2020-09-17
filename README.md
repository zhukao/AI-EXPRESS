AIExpress
=======

# 简介
AI Express，中文名称AI应用开发中间件，是地平线芯片“天工开物”（Horizon OpenExplorer™️ Platform）AI开发平台的一部分，旨在通过全面降低开发者门槛、提升开发速度、保证开发质量，赋能产业智慧升级。

# 快速上手

**硬件环境**
* 1台安装64位Linux操作系统的开发机(或者虚拟机)。操作系统Debian(推荐)/Ubuntu、CentOS，主要用于编译AIExpress代码和日常开发。
* 1台安装Windows操作系统的开发机，用于烧录系统镜像，串口调试。
* 1个1080p的USB摄像头
* x3svb开发板(旭日3 AI 开发板)

**1. Linux开发机环境准备**
* 安装`cmake 3.15+`以上版本,安装方式：
```bash
wget https://github.com/Kitware/CMake/releases/download/v3.17.2/cmake-3.17.2.tar.gz \
    && tar -zxvf cmake-3.17.2.tar.gz \
    && cd cmake-3.17.2 \
    && ./bootstrap \
    && make \
    && sudo make install \
    && cd .. \
    && rm -rf cmake-3.17* 
```
* 下载并安装芯片交叉编译工具[gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu](https://pan.horizon.ai/index.php/s/d3QH3MfzHT5fwd2)，推荐安装路径:`/opt/`，如果交叉编译工具链有更新，需同步修改根目录下的CMakeLists.txt。
具体修改内容：
* `set(CMAKE_C_COMPILER /opt/${工具链目录名}/bin/aarch64-linux-gnu-gcc)`
* `set(CMAKE_CXX_COMPILER /opt/${工具链目录名}/bin/aarch64-linux-gnu-g++)`

**2. Windows开发机环境准备**

* 下载开发板系统镜像及烧录工具：[开发板系统镜像及烧录工具下载地址](https://pan.horizon.ai/index.php/s/ymA5ELRCb7JyTka)

各文件说明：

  | 名称             |             说明 |
| ---------------- | ---------------: |
| SystemImage/disk_X3SDB-Linux-0827.img              |       x3svb开发板系统镜像文件 |
| Tools_Windows/hbupdate_win64_0.7.1.zip           |         hbupdate开发板烧录工具 |
| Tools_Windows/win32diskimager-1.0.0-install.zip    |     win32diskimager开发板烧录工具 |
| Tools_Windows/CP210x_USB2UART_Driver.zip    |     Windows串口驱动安装包 |
| Tools_Windows/PL2302-USB-to-Serial Comm Port.zip    |     Windows串口驱动安装包 |
| Tools_Windows/PL2303-M_LogoDriver_Setup_v202_20200527.zip         | Windows串口驱动安装包 |


**3. 开发板环境准备**

* 开发板接线

  将USB Camera插入板子的USB口中，将网线一端插入开发板网口，另一端插入Windows开发机，电源插入开发板电源接口。
* 目前x3svb开发板适配了1080p的USB摄像头([具体型号参考](https://developer.horizon.ai/forum/id=5f312d96cc8b1e59c8581511))

* 开发板系统镜像烧录教程: [X3开发板板镜像烧录体验](https://developer.horizon.ai/forum/id=5f1aa3ee86cc4d95e81a73e6)

* 开发板使用注意事项：https://developer.horizon.ai/forum/id=5efac2d32ab6590143c16024

更多参考：[x3svb开发板资料包（最新汇总版）](https://developer.horizon.ai/forum/id=5f156192740aaf0beb3119dd)


 **4. 编译**

代码仓库提供了编译一键脚本build.sh，git clone代码后可直接编译。 编译时需要指定平台信息即可，具体编译如下：

 ```
bash build.sh x3
 ```

编译的可执行文件和库在build/bin和build/lib目录下

**5. 部署**

代码仓库提供了一键部署脚本deploy.sh，可将模型、可执行程序、库文件、配置文件以及测试图片整理到deploy部署目录中。将deploy目录拷贝到x3svb开发板上就可以运行参考示例。

 ```
bash deploy.sh
 ```
该脚本会创建deploy部署包，包括如下几个部分：

| 名称             |             备注 |
| ---------------- | ---------------: |
| lib              |       动态依赖库 |
| models           |         模型集合 |
| face_solution    |     人脸解决方案 |
| body_solution    |     人体解决方案 |
| face_body_multisource    |     多路输入多workflow解决方案 |
| configs          |     vio 配置文件 |
| run.sh           |         运行脚本 |

**6. 运行**

直接在开发板的deploy目录下，运行run.sh脚本即可运行指定的测试程序。具体运行命令：
```
sh run.sh body x3svb usb_cam
```

**7. 结果展示**

当开发板上run.sh程序执行后，可以在PC上打开Chrome浏览器，输入x3svb开发板的ip，然后点击页面的Web展示端，即可查看人体结构化解决方案的效果。  


各个测试程序的介绍及运行方法请参考相应源码目录下的README.md。

除了上述人体结构化解决方案外，AIExpress会陆续支持人脸抓拍、人脸识别、行为分析、视频多路盒子、手势识别、体感游戏等、智慧电视等参考示例，有任何建议或问题，欢迎提Issue。

# AIExpress用户手册

参考doc目录下导航页`doc/html/index.html`。


# [地平线开发者社区相关资源](https://developer.horizon.ai/)

## 多路盒子video_box
多路盒子的solution，具体描述可以参考：https://developer.horizon.ai/forum/id=5f2be161740aaf0beb31234a

## 行为分析behavior
行为分析solution，提供了摔倒检测的功能，功能搭建可以参考：https://developer.horizon.ai/forum/id=5efab48f38ca27ba028078dd

## 体感游戏
可以参考：https://developer.horizon.ai/forum/id=5ef05b412ab6590143c15d6a

## 手势识别
可以参考：https://developer.horizon.ai/forum/id=5f30f806bec8bc98cb72b288

## UVC Device
将X3作为UVC设备，通过USB接口接入android系统的硬件上，x3svb开发板通过uvc协议传输图像，通过HID协议传输智能结果。具体可以参考： https://developer.horizon.ai/forum/id=5f312a94cc8b1e59c858150c






