# 介绍
vio_wrapper是一套C++接口，封装了x2/x3 vio库的功能，包括单目camera视频获取、双目camera视频获取以及本地图片回灌功能。   
该repo独立编译时，需要根据是X2还是X3平台，从artifact上拉去不同的vio版本，进行编译。    
当该repo被git repo源码依赖的时候，也应该根据x2/x3平台，源码依赖x2_prebuilt或者x3_prebuit

# 编译
./build.sh

# 运行
./run.sh
