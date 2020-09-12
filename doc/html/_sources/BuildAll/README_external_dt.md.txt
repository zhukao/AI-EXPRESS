XStream
=======

# 介绍

XStream 发版包编译、运行介绍。

# 编译
## 编译环境

需提前准备好交叉编译工具链，默认路径如下：
 ```
set(CMAKE_C_COMPILER /opt/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /opt/gcc-linaro-6.5.0-2018.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++)
 ```
 如果交叉编译工具链地址变动，需同步修改CMakeLists.txt
 ## 编译命令
执行如下命令，编译的可执行文件和库在build/bin和build/lib目录下。hbipc参数仅限x2使用HbipcPlugin作为输出plugin时使用。
 ```
bash build.sh [ x2 ] [ hbipc ]
 ```
# 部署
 ```
bash deploy.sh
 ```
该脚本会创建deploy部署包，包括如下几个部分：

| 名称             |             备注 |
| ---------------- | ---------------: |
| lib              |       动态依赖库 |
| models           |         模型集合 |
| vehicle_solution |     车辆解决方案 |
| configs          |     vio 配置文件 |
| run.sh           |         运行脚本 |

# 运行
直接运行run.sh脚本即可运行指定的测试程序。默认使用96baord配置。各个测试程序的介绍及运行方法请参考相应源码目录下的README.md
 ```
sh run.sh vehicle [ 96board | x2-ipc ] [ 720p | 1080p ]
 ```
 ## 硬件说明
| 开发板           |             备注                            |
| --------------  | ---------------:                            |
| 96board         | X2 96board开发板，demo中只配置了1080P的sensor，720p目前配置为回灌  |
| x2-ipc          | X2 IPC, demo中配置了1080P的sensor。用于演示BT1120输入SPI输出。     |

