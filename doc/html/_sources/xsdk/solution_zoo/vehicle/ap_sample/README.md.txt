# 车路协同方案AP侧Sample
## 介绍
该Sample基于x2-ipc提供AP侧使用BT1120发送图像和hbipc bifspi通路数据数据的样例。
程序启动后会向CP侧发送yuv图像并请求连接CP端。连接成功后开始接收数据并进行解析。

## 编译
```
bash build.sh
 ```
## 打包
 ```
bash deploy.sh
 ```
该脚本会在当前目录下创建deploy文件夹，其中包含运行时库和可执行程序。
此外，还包含bif驱动及相关脚本。

## 运行
将部署包拷贝到板子上，即可运行。
 ```
sh run.sh
 ```

## 工具
部署包tools目录下提供了hbipc-utils工具。使用它可以进行AP-CP间的文件传输，或在AP侧运行命令在CP上。
运行前需要先加载bif驱动
```
Usage:
    hbipc-utils put src_file(ap) dest_file(cp)
    hbipc-utils get dest_file(ap) src_file(cp)
    hbipc-utils run "command"
default use bifspi, append "--bifsd" to use bifsd if HW support
```
