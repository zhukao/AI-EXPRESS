# <font color=#0000ff>V2.2.1</font>

## Release Date
2020/06/19

## Features
- USB Camera功能稳定性优化: 支持将X3开发板，通过USB接口接到AP上，AP通过UVC/HID拿到图像与智能结果
- 添加年龄、性别、口罩模型：在face_solution、body_solution中添加年龄性别以及口罩检测，支持X2与X3
- 框架层面支持json初始化Method：具体Method的初始化实现后续支持

## Bugfixs
- 修复solution退出问题：修复程序退出不能完全释放资源