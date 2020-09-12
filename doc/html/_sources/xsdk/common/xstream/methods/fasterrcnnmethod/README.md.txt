# FasterRCNNMethod

## Intro
FasterRCNNMethod主要是对fasterrcnn检测算法的集成，输入支持PyImageFrame和CVImageFrame两种类型，内部调用bpu_predict接口进行预测，得到相关的检测结果。目前method已支持人脸框，人头框，人体框，人脸关键点，人脸3dpose，人体关键点，人体reid，人体单例分割，车辆，非机动车，行人，车前窗，车牌，车颜色，双排车牌号码等输出。

## Build
```shell
# sh cicd/scripts/build_linux.sh, 默认编译release版，sh cicd/scripts/build_linux.sh debug 编译debug版
# 打包example程序与依赖的库：sh cicd/scripts/copy_runtime.sh 然后将打包好的release文件夹拷贝到开发板上就可以运行了。
```
## Usage
### Example

|  TestFasterRCNNImage  | 跑fasterrcnn，输入为bgr格式的图像|
|  ---- | ----  |
| TestFBFasterrcnn | 回灌方式跑fasterrcnn|
| TestFBPyramid | 回灌通路测试|
| TestModelInfo | 获取模型信息|
| TestTwoFasterRCNN | 同时运行两个fasterrcnn实例，fasterrcnnmethod支持创建多个实例|
| TestX2DEVDualPyramid | 双目camera通路测试|
| TestX2DEVFasterRCNNPyramid | 实时视频跑fasterrcnn|
| TestX2DEVSinglePyramid | 单目camera通路测试|

### 输入/输出 

#### 输入

|  slot id  | content  |
|  ---- | ----  |
|  slot0  | ImageFrame  |

#### 输出
(输出槽的信息和使用的配置文件有关，以face_pose_lmk_config.json为例)

|  slot id  | content |
|  ---- | ----  |
|  slot0  | face_box|
| slot1 | landmark|
| slot2 | pose|

 **单实例不支持多线程访问，支持多实例。**

### 配置文件

根据实际使用情况，fasterrcnn目前有四个配置文件，存放在configs文件夹下，分别对应faceDet模型，faceMultitask(face+pose+lmk)模型,personMultitask（face+head+body+kps+reid+mask） 模型以及vechicle（车辆检测）模型。原则上每集成一个新的模型，是需要添加一个对应的配置文件的。下面以face_pose_lmk_config.json配置为例，讲解配置里主要参数的意思。

|  字段  | 含义|
|  ---- | ----  |
| net_info | 和模型相关的信息|
| model_name | 编译出的hbm文件中模型的名字 |
| model_version | 模型的版本号，GetVersion接口返回的就是这个值 |
| pyramid_layer | 模型用到的金字塔的第几层 |
| method_outs | method的实际输出，我们可以根据这个输出模型输出能力的子集 |
| bpu_config_path | bpu_predict配置的路径 |
| model_file_path | 模型文件的路径 |
| face_pv_thr | 人脸的置信度阈值，没达到置信度阈值的face_box及关联的人脸lmk和人脸pose一并不输出，默认阈值为0 |
| model_out_sequence | 模型输出各分支的信息，我们需要根据model_out_sequence来进行模型结果的后处理 |

对于"model_out_sequence",其内部个参数含义如下：

|  字段  | 含义|
|  ---- | ----  |
| name | 输出的名字 |
| type | 输出的类型 |
| box_name |依赖的box的名字 |
| model_input_width | 模型输入的宽 |
| model_input_height | 模型输入的高 |

### 如何集成一个新的模型

假设你有一个新的fasterrcnn模型要集成，集成的步骤是什么？例如这个模型是个车辆检测相关的模型，输出能力包括车辆，车牌，车前窗，主驾驶，副驾驶。

1. 利用编译工具（hbcc）将模型文件（mxnet对应一个json和param文件）编译成一个hbm文件，确定好模型的名字（也可以通过编译器提供的接口获得），询问算法同事每层的输出是什么，修改配置文件中的model_out_sequence，适配每层的输出。
2. 编写模型的配置文件
3. 根据每层的输出类型，确定是否要扩充FasterRCNNBranchOutType以及FasterRCNNOutMsg以及是否需要添加新的后处理代码
4. 如果需要添加新的后处理代码，则根据算法同事提供的后处理python代码编写c++后处理代码
5. 通过打印数值或渲染图片的方式初步验证集成是否正确。
6. 跑测试集验证指标一致性








