# <font color=#0000ff>V1.0.0</font>

## Release Date
2020/03/20

## Features
- 支持基于hobotsdk engine的XRoc-framework.
- 重构vehicle相关CNN处理，添加了VehicleImgInputPredictor前处理方式．
- 扩充CNN前处理Normalize方式，以支持vehicle相关CNN.
- 补充vehicle solution UT测试用例.
- 文档完善．

## Bugfixs
- 解决车辆因模型重复load，无法平滑退出的问题．
- 清理子repo冗余cmake标志．
- 单目情况下将96board vio可用slot数量从３改为７，以降低丢帧率．





