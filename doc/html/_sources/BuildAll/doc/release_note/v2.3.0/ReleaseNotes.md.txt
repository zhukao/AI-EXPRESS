# <font color=#0000ff>V2.3.0</font>

## Release Date
2020/07/10

## Features
- 支持X3类海思接口回灌
- Web统一展示端重构优化，使用融合的id、Target属性增加value_string输出、分割结果展示
- 支持X3 JPEG编码功能
- AI Express 项目开源，X2与X3整体打包在一起、去除代码、模型以及配置中的一些外部客户信息
- 更新uvc库
- 模型预测，只依赖bpu-predict对外数据结构与接口，去掉对hbrt接口的直接调用，以及bpu-predict部分内部接口的调用
- 升级bpu-predict，与芯片基础工具链3.6.0版本对齐

## Bugfixs
- 修复xwarehouse示例中特征比较相似度为0的问题