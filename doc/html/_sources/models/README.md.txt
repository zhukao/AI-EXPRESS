# aiexpress_aidi_model

本仓库用于存放使用艾迪平台时的模型上传、编译结果下载和模型编译参数脚本。
建议[使用方法](#使用方法)与[使用建议](#使用建议)结合使用。

艾迪平台介绍:http://aidi.hobot.cc/help

# 使用方法
### 1.环境要求
  python版本：2.7或者3.5以上(受艾迪sdk所限，aiexpress_aidi_model仓库对版本无明确要求，建议优先使用python3)
### 2.使用步骤
#### 第1步:下载本仓库master分支代码:
```bash 
git clone --recursive git@gitlab.hobot.cc:iot/xsdk/aiexpress_aidi_model.git 
```

#### 第2步:安装艾迪基础环境包
下载本仓库代码后，进入艾迪平台sdk仓库，根据自身情况安装艾迪工具包。

a).如果你的开发机账号是root用户，并且使用了python-virtualenv工具来切换python2和python3，则执行：
```$bash
root@debian:~# cd aidi-openapi
root@debian:~# bash aidi_install_virtualenv.sh
```
b).如果你的开发机账号是普通用户，没有使用任何额外工具(大多数用户)

##### 安装艾迪基础环境包前环境检查

1.艾迪基础环境包通过pip install的安装方式，目前只支持python3，python2暂不支持。如果必须要在python2上安装，可尝试使用源码安装的方式，即：
```bash
$ cd aidi-openapi
$ bash aidi_install.sh
```
如果源码安装方式仍然不工作，请邮件或者企业微信联系qingpeng.liu@horizon.ai。

2.pip的版本和python3的解释器版本需要是一一对应的。即，不能出现pip install对应的是python2的情况，或者pip install对应的版本是python3.5，而python3对应的版本却是python3.8。

例如，下面的是我机器上的正确执行结果，表明pip和python3的版本都是对应的python 3.5
```bash
$ pip --version
pip 20.0.2 from /usr/local/lib/python3.5/dist-packages/pip (python 3.5)
$ python3 --version
Python 3.5.2
```
如果pip和python3的version不一致，需要调整为一致。比如安装更高版本的python或者通过建立symbolic link的方式调整为一致。

以上环境检查1、2都没有问题后，即可执行下面的命令，开始安装。

```bash
$ pip install six
$ pip install urllib
$ pip install python-dateutil
$ pip install requests
$ pip install aidi==1.0.4 -i http://pypi.hobot.cc/hobot-local/simple --trusted-host pypi.hobot.cc --user
$ pip install aidi_notify==1.0.1 -i http://pypi.hobot.cc/hobot-local/simple --trusted-host pypi.hobot.cc --user
$ pip install aidi_userauth==1.0.1 -i http://pypi.hobot.cc/hobot-local/simple --trusted-host pypi.hobot.cc --user
$ pip install aidi_utils==1.0.9 -i http://pypi.hobot.cc/hobot-local/simple --trusted-host pypi.hobot.cc --user
```

安装结束后，如有下面的输出即表示安装成功。
```bash
$ pip list | grep aidi
aidi                          1.0.4
aidi-notify                   1.0.1
aidi-userauth                 1.0.1
aidi-utils                    1.0.9
```
安装成功后，即可进入下一步。若安装过程中有任何问题，可随时与qingpeng.liu@horizon.ai联系或者在本Repository创建Issue。

#### 第3步:使用本仓库脚本

global_config.py为全局配置，配置中包含艾迪平台的个人token、编译模型的编译器hbdk-cc版本、需要上传的模型类型(单个模型或者多合一模型)、模型源文件的路径等配置信息。

global_config.py中必须先配置`token`字段。token就相当于使用艾迪平台的通行证，没有通行证，则无法使用艾迪平台。

token获取方式：登录http://aidi.hobot.cc (初始用户名: 个人邮箱前缀,初始密码:Aa123456)，web页面右上角，个人账号-->Personal Center-->User Token)上，该token用作身份识别和鉴权，点击[Copy]按钮复制即可,token没有完全在页面展示，请使用[Copy]按钮进行复制。


## 1.模型上传

### 1.1单个模型上传

#### 具体说明
修改global_config.py脚本的配置信息，对于单个模型文件上传来说，有以下字段为必须修改项，其他字段为可选修改项：

`composition_type`字段为模型的类型，含义是上传的模型类型是单个模型还是多合一模型。如需上传单个模型，需设置为：`single`。

`upload_model_dir`为最终编译生成的hbm文件名，同时它也是实验模型目录名和发版模型目录名，需要为其指定一个名字。例如，设置为to_be_uploaded_hbm_file_name_example(为了风格统一，建议使用全小写的命名方法，如果需要使用分隔符，建议使用下划线"_"，而不是"-"，大多数非必要情况下，甚至不建议使用下划线，更详细规则可参考[使用建议](#使用建议))。

`single_model_src_json`字段为MXNet框架的单个模型源文件json文件的路径。

`single_model_src_params`字段为MXNet框架的单个模型源文件params文件的路径。

`single_model_src_pb`为TensorFlow框架的模型源文件。


例如，我有一个输出是人脸质量的模型，需要上传到艾迪平台上，根据模型的功能，我为其命名为face_quality，然后按如下配置global_config.py即可。
```python
composition_type = "single"                         # 单个模型
upload_model_dir = "face_quality"                   # 模型名字
single_model_src_json = "./quality_x2.0.5.json"     # MXNet平台下的模型源文件
single_model_src_params = "./quality_x2.0.5.params" # MXNet平台下的模型源文件
```

以上配置完成后，进入到upload_model目录，然后执行上传脚本，即可完成上传。命令行会返回python脚本的执行结果。
```$bash
#cd upload_model
#python upload_src_model.py
```

备注：
1.暂不支持批量上传单个模型，如果有很多单个模型需要上传，请每次上传一个，逐个上传，逐一验证。

2.模型上传后，可登录 http://aidi.hobot.cc  在web端实验模型、发版模型对应页面下，确认已上传成功。

### 1.2多个模型合并成一个模型上传(简称：多合一模型上传)

多合一模型上传适用场景：需要将多个模型通过hbdk-pack的方式，打包合并成一个模型文件的情形。例如，我需要将车牌模型、多任务检测模型、车型模型、车颜色识别模型，这4个实验模型组合在一起，合并成一个名为vehicle_solution的发版模型，并使用艾迪平台进行管理。

多合一模型上传需要的前提：需要多合一的每个子模型，都已经通过[单个模型上传](#单个模型上传)的方式上传到艾迪上。仍以车辆解决方案模型为例，假设我需要编译一个名为vehicle_solution的多合一模型，这个模型包含：车牌模型、多任务检测模型、车型模型、车颜色识别模型。我需要先通过[单个模型上传](#单个模型上传)的方式，将4个模型的源文件逐一上传至艾迪平台，然后再执行多个模型上传。


#### 具体说明
与单个模型上传类似，根据需要，修改global_config.py文件中的配置信息，对于多合一模型上传来说，有以下字段为必须修改项，其他字段为可选修改项：

`composition_type`字段为模型的类型，含义表示是单个模型还是多合一模型，如果需要上传多合一模型，需设置为：`packed`。

`upload_model_dir`字段为最终编译生成的hbm文件名，同时它也是发版模型目录名，需要为其指定一个名字。例如，设置为to_be_uploaded_hbm_file_name_example(为了风格统一，建议使用全小写的命名方法，如果需要使用分隔符，建议使用下划线"_"，而不是"-"，大多数非必要情况下，甚至不建议使用下划线，更详细规则可参考[使用建议](#使用建议))。

`packed_model_composition`字段为描述多合一模型由哪些模型组成。

例如，我需要一个功能是车辆检测与车牌识别的多合一模型，需要上传到艾迪平台上进行管理。我为多合一模型命名为VehicleSolution，这个模型由plate、vehicle_multitask、vehicle_type、vehicle_color 4个实验模型组成。则我按如下配置global_config.py即可。

```python
composition_type = "packed"           # 多合一模型
upload_model_dir = "vehicle_solution" # 多合一模型最终文件名
packed_model_composition = ["plate", "vehicle_multitask", "vehicle_multitask", "vehicle_type"] # 多合一模型内部每个子模型名
```
以上配置完成后，进入到upload_model目录，然后执行上传脚本，即可完成上传。命令行会返回python脚本的执行结果。
```$bash
# cd upload_model
# python upload_src_model.py
```

备注：
1.多合一模型只有发版模型目录，没有实验模型目录。

2.暂不支持批量上传多合一模型，如果有多个模型需要上传，请每次上传一个，逐个上传，逐一验证。

3.模型上传后，可登录 http://aidi.hobot.cc  在web端确认已上传成功。

## 2.模型编译
*  统一编译脚本
*  单个模型编译脚本
         

## 3.下载模型的编译结果文件
模型的编译结果指的是由hbdk-cc编译生成的.hbm文件、编译log、模型在模拟器运行的性能指标等文件组成的zip压缩包。下载模型编译结果由多个模型下载脚本组成，不同的脚本，下载各自类别下的编译结果文件。

|文件名称|说明|
| ---- | ---- |
| release_aiot.py | 下载aiot相关的对外release的模型 |
| release_auto.py | 下载auto相关的对外release的模型 |
| release_all.sh  | 下载所有对外release的模型 |
| debug_internal.py | 下载所有的内部模型(即，除release_all.sh之外的所有模型) |
| all_model.sh | 下载所有的模型 |
| download_single_model.py| 下载某个模型(单个模型或者多合一模型)的编译结果 |

### 下载单个模型的编译结果文件
根据上传时，设置的模型目录名(即global_config.py中的upload_model_dir字段)，下载对应模型目录名的编译结果。

修改download_single_model.py中的upload_model_dir字段的值为某个发版模型目录名。

例如，我之前已经通过[单个模型上传](#单个模型上传)和[2.模型编译](#2.模型编译)，成功上传和编译了一个名为`to_be_uploaded_hbm_filename_example`的模型，现在我需要下载这个模型的编译结果，则直接修改download_single_model.py脚本中的upload_model_dir字段为如下，即可。
```python
#download_single_model.py
download_model_dir = "to_be_uploaded_hbm_filename_example"

```
然后执行
```bash
#python download_single_model.py
```
即可完成模型编译结果的下载。

### 批量下载模型的编译结果文件

本地编译调试和CI/CD流程中，会执行all_model.sh，将项目内部在艾迪平台上的**所有**模型的编译结果文件下载到download_model目录下。在编译代码时，使用debug编译类型，即可触发调用all_model.sh。

对外打包发版时，会执行release_all.sh，只对对外release的模型文件，进行下载和打包。在编译代码时，使用release编译类型，即可触发调用release_all.sh。

# 使用建议
1.当模型源文件内容发生变动，或者模型源文件在AIDI上不存在的时候，需要上传模型源文件。(模型源文件定义: MXNet平台的model.json+model.params, TensorFlow平台的model.pb文件)

2.如果是输出不同的模型，建议使用独立的目录进行管理。例如，有2种多任务模型face_id和face_age_gender，则建议分别创建face_id_multitask目录和face_age_gender_multitask目录，分别管理。

3.实验模型目录名、发版模型目录名，输出的编译结果(即hbm文件名)，三者最好使用同一个名字，管理方便。脚本中默认会使用hbm文件名作为三者的名字。

4.除下载模型编译结果的python脚本外，其他文件，默认不建议设置版本，默认取模型的最新版本即可，如果需要，可自行指定即可。

5.同一份模型源文件，只是编译参数(为了适配不同的图像宽高)不同，使用不同版本号+commit message通过不同的编译任务名进行区分。

例如：需要对模型person_multitask，基于一份模型源文件，使用不同的图像宽高编译参数，进行编译，可以使用person_multitask_1024x768和person_multitask_960x540来分别区分两个编译任务。
  
  编译任务的定义：艾迪平台是以编译任务为最小的编译管理单位，即，每一次编译的动作都会被称为一个编译任务，也就是每次执行hbdk-cc编译都是一个编译任务。同一组编译模型参数可以对应多个编译任务。

6.基于AIDI上已经存在的模型源文件，只进行局部修改（比如剪枝优化，检测目标数减少等)，不用再创建模型目录，只在原模型目录下添加一个版本，使用不同版本号+commit message通过不同的编译任务进行区分。

7.模型源文件相差较大，即使是同一功能模型，但是适用不同场景，也需要创建各自独立的模型目录(实验模型目录、发版模型目录)。比如都是face id,可以用场景(或者其他标识)进行区分，face_id_rain和face_id_snow，两个模型的实验模型目录、发版模型都应创建两套独立的目录，分别进行管理。

8.绝大多数情况下，都不建议在命名过程中使用缩写，因为代码是多人维护，缩写会带来沟通成本和犯错成本(比如忘写、写错、缩写名冲突、含义不够明确等)，缩写的好处大于坏处。如果一定要用，强烈建议在本文档页面，建立缩写表。并在使用的缩写的地方写清注释，注明缩写的含义及来源，便于他人理解。

9.为了风格统一，当为模型取名时，建议使用全小写的命名方法，如果需要使用分隔符，建议优先使用下划线"_"，而不是"-"，大多数非必要情况下，甚至不建议使用下划线。

# FAQ


# 内部实现
本脚本主要基于仓库根目录下aidi-openapi sdk接口实现。

接口参考文档：
http://gitlab.hobot.cc/ptd/ap/toolchain/ai-platform/aidi-openapi/blob/master/README.md

http://wiki.hobot.cc/pages/viewpage.action?pageId=86159671

