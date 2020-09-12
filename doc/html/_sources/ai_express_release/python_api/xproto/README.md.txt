# XProto Python API

## 介绍
XProto Python API 为XProto框架功能在Python的封装, 通过它可以方便快速地定义运行智能应用.

### 插件, 消息, 总线
XProto Python API 中利用 VioPlugin, SmartPlugin 两个插件分别实现获取输入数据和运行工作流的功能. 目前 XProto Python API 支持同步同步、异步两个模式. 同步模式下输入数据将被直接传递给SmartPlugin. 异步模式下, XProto 使用总线来管理各 plugin 和它们产生的数据.
![XProto组件示意图]](docs/xproto.png "XProto组件示意图")

## 前提
XProto Python API 需要和 XStream Python API 配合使用. 您需要了解如何定义工作流.

## 使用
### 同步模式
同步模式下, 您可以直接从 VioPlugin 获取输入数据，供 SmartPlugin 使用:
```python
import xstream
import xproto

# 创建VioPlugin, SmartPlugin
vio = xproto.VioPlugin("96board")
# workflow, callback是您定义的工作流和回调函数
smart = xproto.SmartPlugin(workflow, callback)
# 以同步模式启动Vioplugin
vio.start(sync_mode=True)
t_end = time.time() + 10
while time.time() < t_end:
    # 获取输入
    image = vio.get_image()
    # 传递给SmartPlugin使用
    smart.feed(image)
vio.stop()
smart.stop()
```
同步模式下进行回灌时, 您需要根据回灌图片数来控制对 get_image() 接口的调用.

### 异步模式
异步模式下, 各插件产生的消息由总线管理. 您需要创建插件并指定它们的连接关系, 之后启动它们.:
```python
import xstream
import xproto

# 创建插件
vio = xproto.VioPlugin("96board")
smart = xproto.SmartPlugin(workflow, callback)
# SmartPlugin监听VioPlugin产生的消息
smart.bind(vio.get_message_type())
# 启动Plugins, 默认为异步模式
vio.start()
smart.start()
 
time.sleep(5)

vio.stop()
smart.stop()
```

## xproto包接口介绍
### VioPlugin类
#### \_\_init__(*`platform`*, *`data_source`*=`"panel_camera"`, *`cam_type`*=`"mono"`, *`**kwargs`*)

创建一个 VioPlugin 对象

**参数**
+ `platform` : 使用的硬件平台, 目前支持`96Board`, `2610`.
+ `data_source` : 输入数据源, 默认为`panel_camera`. 目前支持`panel_camera`, `jpeg_image_list`, `nv12_image_list`, `cached_image_list`.
+ `cam_type` : 使用的sensor类型, 默认为`mono`. 目前支持`mono`.
+ `**kwargs` : 回灌模式下通过`name_list`或`image_list`关键字回灌图片列表/文件.

**返回值**
+ VioPlugin对象.

#### start(*`sync_mode`*=`False`)

启动VioPlugin.

**参数**
+ `sync_mode` : 是否为同步模式, 默认为`False`.

**返回值**
+ 无.

#### stop()

停在VioPlugin.

**参数**
+ 无.

**返回值**
+ 无.

### bind(*`msg_type`*, *`msg_cb`*=`None`)

绑定需要监听的消息.

**参数**
+ msg_type : 消息类型.
+ msg_cb ：对应的回调函数.

**返回值**
+ 无.

#### message_type()

返回 VioPlugin 所产生的消息的类型.

**参数**
+ 无.

**返回值**
+ 字符串列表 : VioPlugin 所产生的消息的类型.

#### get_image()

同步模式下获取输入数据.

**参数**
+ 无.

**返回值**
+ 字典 : key: 工作流输入名字, value: 输入数据.


### SmartPlugin类
#### \_\_init__(*`workflow`*, *`callback`*, *`serialize`*, *`push_result`*=`False`)

创建一个 SmartPlugin 对象.

**参数**
+ `workflow` : 定义工作流的函数.
+ `callback` : 工作流回调函数.
+ `serialize` : 智能数据序列化函数.
+ `push_result` : 是否推送智能数据到总线. 默认为`False`.

**返回值**
+ SmartPlugin 对象

#### start()

启动 SmartPlugin. 异步模式下需要显式调用.

**参数**
+ 无

**返回值**
+ 无.

#### stop()

停在 SmartPlugin.

**参数**
+ 无

**返回值**
+ 无.

#### bind()

绑定需要监听的消息.

**参数**
+ msg_type : 消息类型.
+ msg_cb ：对应的回调函数.

**返回值**
+ 无.

#### feed(*`inputs`*)

同步模式下接收输入数据.

**参数**
+ inputs : 字典. key: 工作流输入数据名, value: 输入数据

**返回值**
+ 无.

#### message_type()

返回 SmartPlugin 所产生的消息的类型.

**参数**
+ 无.

**返回值**
+ 字符串列表 : SmartPlugin 所产生的消息的类型.

