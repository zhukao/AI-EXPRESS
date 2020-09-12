# aidi-openapi

## 说明

包含四个python包

- aidi: 艾迪平台通用功能，包括训练、预测、评测、模型、编译等功能
- aidi_userauth: 艾迪平台用户与权限功能
- aidi_utils: 对aidi包的辅助功能
- aidi_notify: 艾迪平台消息通知功能，支持邮件和企业微信

## 安装

### 下载源代码安装

```shell
# 非python虚拟环境
sh aidi_install.sh
# python虚拟环境
sh aidi_install_virtualenv.sh
```

### 公司内部源安装

```shell
pip install aidi -i http://pypi.hobot.cc/hobot-local/simple --trusted-host pypi.hobot.cc --user
pip install aidi_userauth -i http://pypi.hobot.cc/hobot-local/simple --trusted-host pypi.hobot.cc --user
pip install aidi_utils -i http://pypi.hobot.cc/hobot-local/simple --trusted-host pypi.hobot.cc --user
pip install aidi_notify -i http://pypi.hobot.cc/hobot-local/simple --trusted-host pypi.hobot.cc --user
```

## 示例代码说明

示例代码统一在example目录

- 训练数据打包
- 训练数据上传
- 评测脚本上传
  - example/evalprofilecreate.py
- 评测参数上传
  - example/evalparamscreate.py
- 创建EvalSet
- 发起训练
- 上传预测结果
  - example/appendpr.py
- 创建实验模型目录(设置权限为public)
  - example/modelcreate.py
- 创建实验模型目录(设置权限为private)
  - example/modelcreateprivate.py
- 保存本地实验模型
  - example/modelappend.py
- 保存HDFS实验模型
  - example/hdfsmodelappend.py
- 保存Bucket中的实验模型
  - example/bucketmodelappend.py
- 实验模型发布到发版模型
  - example/modelpublish.py
- 下载实验模型
  - example/modelget.py
- 创建发版模型目录
  - example/pmodelcreate.py
- 创建发版模型
  - exampe/pmodelappend.py
- 发版模型增加或删除实验模型
  - example/pmodelupdate.py
- 下载发版模型
  - example/pmodelget.py
- 发起编译
  - example/compilationtaskcreate.py
- 下载编译结果
  - example/compilationtaskget.py
- 发版模型正式发版
  - example/compilationpublish.py
- 发起回灌
  - example/fueltaskcreate.py
- 发起预测
- 发起评测
- 查看评测报告
- 提交和获取全流程参数
  - example/submitpipelineparam.py
  - example/getpipelineparam.py
- 在dagtask之间传递参数
  - example/submitpipelineparam.py
  - example/getpipelineparam.py
