# Xwarehouse Sample
## 介绍
该sample程序使用xwarehouse库实现特征增加、删除、修改、查找、1:1对比等操作。
其中，特征信息是固定的，不支持动态获取特征。
## 能力集
xwarehouse依赖库、头文件、接口文档存放在deps/xwarehouse目录。
## 编译
进入ai_express_release发版包
 ```
bash build.sh
 ```
## 打包部署包
 ```
bash deploy.sh
 ```
该脚本会在当前目录下创建deploy文件夹，包含xwarehouse的sample app及其运行所依赖的库文件。

## 运行
将部署包拷贝到板子上，即可运行sample。
 ```
export LD_LIBRARY_PATH=./lib
./xwarehouse_sample/xwarehouse_sample
 ```
