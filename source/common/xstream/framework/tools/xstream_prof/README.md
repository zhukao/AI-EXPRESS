# XSTREAM 性能分析工具

## 介绍
XStream性能分析工具分为链接到进程中的日志记录部分和独立运行的分析工具.

## 使用
在当前目录下执行`make`可以在sample目录下生成sample程序.  
执行sample程序:
```bash
#export TR_LOG_FILE=./record.log
#./sample/sample
TR_LOG_FILE=./record.log ./sample/sample
# 会在当前目录生成record.log文件
./tools/binary/xstream_prof ./record.log
```
### 快捷键
+ +: 放大
+ -: 缩小
+ 0: 显示全部
+ w/a/s/d或者方向键: 平移显示窗口
+ 1: 切换到 "Time Order" 窗口
+ 2: 切换到 "Left Heavy" 窗口
+ 3: 切换到 "Sandwich" 窗口
+ r: 展开火焰图中的递归
+ n: 转到下一个线程（如果有的话）
+ p: 转到上一个线程（如果有的话）
