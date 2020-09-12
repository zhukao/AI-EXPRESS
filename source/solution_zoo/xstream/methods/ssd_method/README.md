# SSDMethod

## Intro
SSDMethod主要集成了ssd模型算法和后处理，输入支持PyImageFrame类型，内部调用bpu_predict接口进行预测，得到相关的检测结果。

## Build
采用统一编译：  
  编译: sh build.sh

## Usage
### 输入/输出 

#### 输入

|  slot id  | content  |
|  ---- | ----  |
|  slot0  | ImageFrame  |

#### 输出
(输出为检测框，可指定输出的检测框类型；若不指定，则输出检测到的全部检测框)

|  slot id  | content |
|  ---- | ----  |
|  slot0  | detect_box|

 **单实例不支持多线程访问，支持多实例。**

### 配置文件

SSD有2个配置文件，存放在config文件夹下：
ssd_test_workflow.json：workflow配置
ssd_module.json：模型参数、bpu路径、模型文件路径等

对于ssd_module.json，其参数说明如下：
|  字段  | 含义|
|  ---- | ----  |
| net_info | 和模型相关的信息|
| model_name | 编译出的hbm文件中模型的名字 |
| model_version | 模型的版本号，GetVersion接口返回的就是这个值 |
| pyramid_layer | 模型用到的金字塔的第几层 |
| method_outs | method的实际输出，我们可以根据这个输出模型输出能力的子集 |
| model_out_type | 指定输出的检测框类型，未指定时，默认全部输出 |
| bpu_config_path | bpu_predict配置的路径 |
| model_file_path | 模型文件的路径 |
| score_threshold | 检测框的置信度阈值，没达到置信度阈值的box不输出，如果设置为0，程序默认修正为0.3 |
| nms_threshold | nms置信度阈值，如果设置为0，程序默认修正为0.45 |
| model_params | 超参参数 |

### 测试用例
ssd_detect_sdk：使用workflow加载模型、预测、获取结果sample。
