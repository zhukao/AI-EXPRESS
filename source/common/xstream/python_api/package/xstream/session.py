import xstream
import xstream_internal as xi

__all__ = [
    'Session'
]


class Session:

    flow_ = None

    def __init__(self, workflow, *inputs):
        # 创建json字符串
        json_cfg = xstream.serialize(workflow, *inputs)
        # 保存到文件
        # XXX 文件保存至何处, method的路径需要?
        cfg_file = open("./%s.json" % workflow.__name__, 'w')
        cfg_file.write(json_cfg)
        cfg_file.close()  # 关闭文件
        # 调用xstream internal的构造函数,创建一个flow
        self.flow_ = xi.XStreamSDK()
        self.flow_.init("./%s.json" % workflow.__name__)

    # 设置workflow的回调函数,可以针对某个node的输出或者全局的输出
    def callback(self, cb, node_name=""):
        self.flow_.set_callback(cb, node_name)
        return self

    # 异步预测
    def forward(self, **kwargs):
        self.flow_.async_predict(**kwargs)

    # 释放相关资源
    def close(self):
        del self.flow_

    def __exit__(self):
        self.close()
