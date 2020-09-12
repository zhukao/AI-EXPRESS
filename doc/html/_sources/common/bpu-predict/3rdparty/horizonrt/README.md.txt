地平线 异构runtime
=======
# 介绍
HorizonRT（Horizon Runtime）是地平线开发的异构runtime，目标是：客户的模型不经量化训练即可在X2A上运行起来。具体是：量化工具直接将浮点模型转换为定点模型，然后利用
HorizonRT将定点模型运行起来，模型中包含BPU支持的OP和CPU支持的OP，其中BPU支持的OP会在BPU中被调度执行，CPU支持的OP会在ARM上被调度执行，异构地完成整个模型的推理工作。
# 特性
- 支持模型在ARM+BPU环境下异构执行
- 支持自定义算子的插件机制
- 优化调度和内存分配，加速推理性能

# 相关参考
[异构runtime](http://wiki.hobot.cc/pages/viewpage.action?pageId=68771648)

# 其它
后续继续补充完善README。
