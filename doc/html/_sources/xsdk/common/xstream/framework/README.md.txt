<!-- TOC -->

- [Overview](#overview)
    - [XStream Framework解决什么问题](#xstream-framework解决什么问题)
    - [XStream Frameowrk 概述](#xstream-frameowrk-概述)
        - [XStream Framework核心模块](#xstream-framework核心模块)
        - [XStream Framework驱动引擎](#xstream-framework驱动引擎)
- [XStream-Framework 用户使用指南](#xstream-framework-用户使用指南)
    - [Build](#build)
        - [Bazel 编译](#bazel-编译)
            - [安装 bazel](#安装-bazel)
            - [本地及交叉编译](#本地及交叉编译)
        - [Cmake 编译](#cmake-编译)
            - [安装cmake](#安装cmake)
            - [安装交叉编译工具链](#安装交叉编译工具链)
            - [编译](#编译)
        - [编译结果](#编译结果)
        - [环境信息](#环境信息)
    - [Step by step构建XStream SDK](#step-by-step构建xstream-sdk)
        - [XStream SDK接口定义](#xstream-sdk接口定义)
        - [XStream SDK接口说明](#xstream-sdk接口说明)
            - [CreateSDK](#createsdk)
            - [SetConfig](#setconfig)
            - [Init](#init)
            - [UpdateCondig](#updatecondig)
            - [SetConfig](#setconfig-1)
            - [GetVersion](#getversion)
            - [SyncPredict](#syncpredict)
                - [SetCallback](#setcallback)
                - [AsyncPredict](#asyncpredict)
                - [SyncPredict2](#syncpredict2)
        - [XStream SDK使用](#xstream-sdk使用)
            - [创建SDK](#创建sdk)
                - [设置XStream配置初始化](#设置xstream配置初始化)
            - [XStream SDK初始化](#xstream-sdk初始化)
            - [定义和设置 callback](#定义和设置-callback)
            - [使用性能统计工具profiler](#性能统计工具-profiler)
            - [异步运行-输入数据](#异步运行-输入数据)
            - [异步运行](#异步运行)
            - [同步运行-输入数据](#同步运行-输入数据)
            - [同步运行-输出数据](#同步运行-输出数据)
            - [异步运行-多路输出数据](#异步运行-多路输出数据)
            - [同步运行-多路输出数据](#同步运行-多路输出数据)
        - [include 文件列表](#include-文件列表)
        - [实现MethodFactory](#实现methodfactory)
        - [通过Config文件定义workflow](#通过config文件定义workflow)
            - [常用配置](#常用配置)
            - [指定线程优先级的配置](#指定线程优先级的配置)
            - [多路输出配置](#多路输出配置)
        - [数据类型](#数据类型)
            - [错误码](#错误码)
            - [基础数据结构](#基础数据结构)
                - [DataState](#datastate)
                - [BaseData](#basedata)
                - [BaseDataVector](#basedatavector)
                - [XStreamData](#xstreamdata)
                - [InputParam](#inputparam)
                - [DisableParam](#disableparam)
                - [SdkCommParam](#sdkcommparam)
                - [InputData](#inputdata)
                - [OutputData](#outputdata)
                - [XStreamCallback](#xstreamcallback)

<!-- /TOC -->
# Overview
## XStream Framework解决什么问题
降低算法模型，算法策略集成开发的门槛和难度。沉淀和积累智能化工程核心开发能力。


## XStream Frameowrk 概述
XStream-Framwork是一种基于数据流的SDK编程框架：   
1）可以通过JSON配置构建workflow，workflow是一个有向拓扑图，图中每个节点（Node）都管理了一个或多个同类型的method的实例；  
2）method表示一种能力，通常是某类模型能力（人脸检测、人脸Pose等）或者算法策略（过滤策略、融合策略、优选策略等）；  
3）workflow表示一个范式，定义了一组能力的串联方式，比如人脸检测、跟踪、属性（pose、blur等）以及优选等能力级联起来可以构建一个人脸抓拍范式；  
4）XStream-Framework定义了一套面向workflow的sdk C++通用接口，通过设置不同的配置文件同一套接口可以运行不同的workflow。   

![XStream概貌](./doc/images/image-xstream-arch.png "XStream概貌")
### XStream Framework核心模块

| 模块名称或名词 | 描述或解释                                                                                                                                                                                                                                                                            |
| :------------: | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
|    workflow    | workflow表示一个有向拓扑图，图中每个节点（Node）都管理着一个或多个method实例；workflow表示一个范式，定义了一组能力的串联方式，比如人脸检测、跟踪、属性（pose、blur等）以及优选等能力级联起来可以构建一个人脸抓拍范式；XStreamSDK的SetConfig接口指定的配置文件定义了workflow组织方式。 |
|   XStreamSDK   | 对外SDK类，定义了一套面向workflow的通用sdk C++接口，包含创建句柄、初始化、参数配置、同步/异步运行接口、设置回调函数等。                                                                                                                                                               |
|   Scheduler    | workflow的依赖引擎，记录各个Node之间的依赖关系，统一调度各个Node，确保各Node按序完成workflow任务。                                                                                                                                                                                    |
|      Node      | 管理一个或多个同类型的method实例，同时负责任务的分发与后处理，另外根据method的属性可以完成帧的重排序，根据输入参数完成Node skip等。                                                                                                                                                   |
| MethodManager  | 用来Node中Method的管理，包含实例创建、线程池构建及不同阶段调用method对应接口完成初始化、任务分发、参数配置等工作。                                                                                                                                                                    |
|     Method     | method表示一种能力，通常是某类模型能力（人脸检测、人脸Pose等）或者算法策略（过滤策略、融合策略、优选策略等）；作为一个独立模块，包含初始化、任务处理、参数配置等接口。                                                                                                                |
| FrameWork Data | 通过 SyncPredict或 AsyncPredict接口传入的一"帧"数据,并在Node之间流转,最后通过同步接口或异步回调返回的数据，称为Framework Data                                                                                                                                                         |

### XStream Framework驱动引擎
XStream目前底层可以由两种引擎驱动：
- NativeEngine：基于C++11实现的驱动引擎，完成graph构建、数据依赖、流式驱动、线程管理等基础功能；
- HobotSDK Engine:hobotsdk是地平线成熟的面向数据流的编程框架，xstream framework通过配置可以选择将hobotsdk作为底层驱动引擎，解决数据依赖、流式驱动与线程管理等核心问题，详细见[hobotsdk engine](./doc/hobotsdk_engine.md)

# XStream-Framework 用户使用指南
## Build
### Bazel 编译
#### 安装 bazel
**以下安装bazelisk和直接安装bazel方法二选一，建议使用bazelisk更方便**
* 安装 bazelisk
  * 到官方github网站找到最新版本 https://github.com/bazelbuild/bazelisk/releases, 下载对应操作系统的程序到本地，重命名为bazelisk，放置在环境变量PATH中，重命名bazel。如果已安装bazel, 一般是在PATH中, 创建软连接到路径.bazel/bin/bazel, 将这个bazel软连接改连接到bazelisk即可。
  代码根目录下的.bazelversion文件中，指定了bazel版本。bazelisk会根据这个bazel版本号下载具体的bazel版本。
```c
root@3ebb9bdfca19:/tmp# whereis bazel
bazel: /etc/bazel.bazelrc /usr/local/bin/bazel /usr/local/lib/bazel
root@3ebb9bdfca19:/tmp# mv /usr/local/bin/bazel /usr/local/bin/bazel_origin
root@3ebb9bdfca19:/tmp# wget https://github.com/bazelbuild/bazelisk/releases/download/v1.2.1/bazelisk-linux-amd64 -O /usr/local/bin/bazel
```
设置可执行权限并运行，查看是否生效。
```c
root@3ebb9bdfca19:/tmp# chmod a+x /usr/local/bin/bazel
root@3ebb9bdfca19:/tmp# bazel version
Bazelisk version: 56a03d98104be7cfa57d4bbdc03b4c7cea29a6c9
Build label: 1.2.0
Build target: bazel-out/k8-opt/bin/src/main/java/com/google/devtools/build/lib/bazel/BazelServer_deploy.jar
Build time: Wed Nov 20 15:04:55 2019 (1574262295)
Build timestamp: 1574262295
Build timestamp as int: 1574262295
```

* 安装 bazel (当前建议安装1.2.0) 
  * Ubuntu
   参见 [Installing Bazel on Ubuntu](https://docs.bazel.build/versions/master/install-ubuntu.html)
  * Fedora and CentOS
   参见 [Installing Bazel on Fedora and CentOS](https://docs.bazel.build/versions/master/install-redhat.html)
  * macOS
   参见 [Installing Bazel on macOS](https://docs.bazel.build/versions/master/install-os-x.html)
**更多Bazel帮助信息请参见[Bazel Documentation](https://docs.bazel.build/)**

* Artifactory 环境配置
  因部分依赖Artifactory, 如unit test。 需要配置Artifactory 环境如下
 ```
 echo "machine ci.horizon-robotics.com login deploybot password deploybot@Artifactory2016" > ~/.netrc
 ```
#### 本地及交叉编译
* x86_64     本地编译 XStream Framework库文件
`bazel build -s  //xstream/framework:xstream-framework  --define cpu=x86_64 --define os=linux`
其中 `-define cpu=x86_64 --define os=linux` 用于指定CPU类型和OS系统类型

* x86_64     本地编译以及打包 XStream Framework库文件 
`bazel build -s  //xstream/framework:xstream-framework_zip  --define cpu=x86_64 --define os=linux`
在build中用cc_library_pkg定语的rule,可以加`_zip`,表示编译并打包。
 例如:
```c++
 cc_library_pkg(
    name = "xstream-framework",
...
``` 
另外`-define cpu=x86_64 --define os=linux` 用于指定CPU类型和OS系统类型
参见 build_script/build_xstream-framework_x86.sh

* X2J2 64位  交叉编译 XStream Framework库文件
`bazel build -s   //xstream/framework:xstream-framework --crosstool_top="@hr_bazel_tools//rules_toolchain/toolchain:toolchain" --cpu=x2j2-aarch64 --define cpu=x2j2-aarch64 --verbose_failures   --spawn_strategy=local`
参见 build_script/build_xstream-framework_aarch.sh

* X2J2 32位  交叉编译 XStream Framework库文件
`bazel build -s   //xstream/framework:xstream-framework --crosstool_top="@hr_bazel_tools//rules_toolchain/toolchain:toolchain" --cpu=x2j2-armv8l  --define cpu=x2j2-armv8l  --verbose_failures   --spawn_strategy=local`

### Cmake 编译
#### 安装cmake
* ubunutu 环境
 `sudo apt-get install cmake`
* centos 环境
 `sudo yum -y install cmake`

#### 安装交叉编译工具链
  为编译在X2 Soc板上运行的可执行程序或库文件, 需要安装工具链：`x2j2-aarch64-6.5.0`
  具体安装交叉工具链压缩包, 如在公司内部可从以下地址下载：
http://gallery.hobot.cc/download/aiot/toolchain/x2j2-aarch64/project/snapshot/linux/x86_64/general/basic/6.5.0/x2j2-aarch64-6.5.0.tar.xz
  如在公司外部，请联系技术支持人员获取工具链。
  工具链压缩包-sha256值：63e35c43452bca1761751ab7a6af35d88b8f36a8f906c12e4dd284a06309d37f

#### 编译
* X2 交叉编译 
  `build.properties.local` 默认是 X2交叉编译工具链

* 执行交叉编译
  推荐创建编译目录，如代码根目录 `mkdir build`
  `cd build & cmake ../`
  `make`

### 编译结果
 * example:
*bin/bbox_filter_example*   
——以HobotXStream::BBox为数据类型, 基于XStream C++语言API编写的Example案例
*bin/c_bbox_filter_example* 
——以HobotXStreamCapiBBox为数据类型， 基于XStream C语言API编写的Example案例

  * unit test:
*bin/config_test*   
—— 配置相关的单元测试
*bin/cpp_api_test*  
—— C++ API的集中单元测试
*bin/disable_method_test*  
—— 不同方式关闭特定method的单元测试
*bin/node_test* 
—— 对node节点的单元测试
*bin/xstream_multisource_test* 
—— 对多路输入源的单元测试
*bin/xstream_test* 
—— XStream SDK接口额单元测试
*bin/xstream_threadmodel_test* 
—— XStream 线程模式(线程池运行方式)的单元测试
*bin/xstream_threadorder_test*
—— XStream 线程优先级的单元测试
*bin/xstream_threadsafe_test*
—— XStream 线程安全的单元测试
*bin/xstream_callback_test*
—— XStream callback回调的单元测试
*bin/profiler_test*
—— XStream 诊断记录的单元测试

### 环境信息
Centos： x64 gcc4.8.5   
X2：64/32位  gcc-linaro-6.5.0   

## Step by step构建XStream SDK 
xstream给使用者提供了面向workflow的通用sdk接口，可以获取workflow中每个节点的输出结果。
c++语言版本 XStream SDK接口定义在 include/hobotxsdk/xstream_sdk.h
### XStream SDK接口定义
```c++
namespace xstream {
/**
 * 典型使用
 * xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();
 * flow->SetConfig("config_file", config);
 * flow->Init();
 * InputDataPtr inputdata(new InputData());
 * // ... 构造输入数据
 * auto out = flow->SyncPredict(inputdata);
 * // PrintOut(out);
 * // ... 处理输出结果
 * delete flow;
 */

/// 数据流提供的接口
class XStreamSDK {
 public:
  /// 因为构造出来的实例是XStreamSDK接口的子类
  virtual ~XStreamSDK() {}
  /// 通过此方法构造SDK实例
  static XStreamSDK *CreateSDK();

  /// key：config_file，用来设置workflow配置文件路径
  virtual int SetConfig(
      const std::string &key,
      const std::string &value) = 0;  // 设置授权路径、模型路径等等
  /// 初始化workflow
  virtual int Init() = 0;
  /// 针对node更新配置参数
  virtual int UpdateConfig(const std::string &unique_name,
                           InputParamPtr ptr) = 0;
  /// 获取node当前配置
  virtual InputParamPtr GetConfig(const std::string &unique_name) const = 0;
  /// 获取node的版本号
  virtual std::string GetVersion(const std::string &unique_name) const = 0;
  /// 同步运行接口，单路输出
  virtual OutputDataPtr SyncPredict(InputDataPtr input) = 0;
  /// 同步运行接口，多路输出
  virtual std::vector<OutputDataPtr> SyncPredict2(InputDataPtr input) = 0;
  /**
   *  异步接口的callback设置接口
   *
   * 需要在Init()后执行，否则name不为空时无法得到结果
   * @param callback [in], 回调函数
   * @param name [in], workflow 节点
   * unique_name，当使用默认参数时，callback为全局回调，
   *    只有当这一帧数据全部计算结束才会回调报告结果；如果设置了unique_name，则在异步调用中就会
   *    上报当前节点的输出，但同步运行中不会回调。
   */
  virtual int SetCallback(XStreamCallback callback,
                          const std::string &name = "") = 0;  // 设置回调
  /// 异步运行接口
  virtual int64_t AsyncPredict(InputDataPtr input) = 0;  // 异步接口
};
}  // namespace xstream
```
### XStream SDK接口说明
#### CreateSDK
`static XStreamSDK *CreateSDK();`   
 说明：通过该接口创建XStream的实例。

#### SetConfig
`virtual int SetConfig(const std::string &key, const std::string &value) = 0;`   
说明：用于设置整个workflow的配置，目前支持的功能有：  
1）key为"config_file"，value设置为workflow的配置路径，用于设置整个workflow的配置文件。   
2）key为"profiler"，value为"on"，表示打开性能统计功能。"off"表示关闭, 默认为关闭。   
3）key为"profiler_file",value为性能统计输出文件路径，用于设置性能统计文件的路径名称，默认为./profiler.txt   
4）key为"free_framedata", value为"on", 表示尽早地释放掉在后面node节点中不再需要使用的Framework Data中的某项数据。   
打开此项配置,可以减少峰值内存使用。"off"表示关闭, 默认为关闭。   

#### Init
`virtual int Init() = 0;`
说明：用于初始化xstream句柄，必须在调用SetConfig之后执行Init()

#### UpdateCondig
`virtual int UpdateConfig(const std::string &unique_name, InputParamPtr ptr) = 0;`  
说明：用于设置node的参数，最终会通过调用对应的node管理的method实例的UpdateParameter(InputParamPtr ptr)接口，完成参数的更新。
形参unique_name传入node的名字；形参ptr为该node对应的配置信息

#### SetConfig
`virtual InputParamPtr GetConfig(const std::string &unique_name) const = 0;`  
说明：获取某个node的参数，最终会调用对应的node管理的method实例的GetParameter()返回配置信息。

#### GetVersion
`virtual std::string GetVersion(const std::string &unique_name) const = 0;`
说明：获取node对应method的版本信息。

#### SyncPredict
`virtual OutputDataPtr SyncPredict(InputDataPtr input) = 0;`
说明：同步运行接口，传⼊数据，接口会阻塞住，直到整个workflow处理完成，将workflow的结果通过函数返回值返回为止。
该接口需要在Init()之后执行才有效。

##### SetCallback
`virtual int SetCallback(XStreamCallback callback, const std::string &name = "") = 0;`
说明：使用异步运行接口时，设置SetCallback才有效。
将name设置为默认值，通过该接口可以设置整个workflow处理完成后的回调函数；
将name设置为某个node的unique名字，通过该接口可以设置该node处理数据完成后的回调函数。

##### AsyncPredict
`virtual int64_t AsyncPredict(InputDataPtr input) = 0;`
说明：异步运行接口，结果通过SetCallback设置的回调函数捕获。AsyncPredict接口调用后立即返回。
该接口需要在Init()之后执行才有效。

##### SyncPredict2
`virtual std::vector<OutputDataPtr> SyncPredict2(InputDataPtr input) = 0;`
说明: 同步多路输出的场景下， 输出接口可以通过output_type_信息判断输出类型。

### XStream SDK使用
* Example 异步运行模式   
**以下代码中出现的ASSERT_TRUE, EXPECT_EQ, ASSERT_EQ等，来源于googletest。**
**用于对返回值结果等做一些检查，实际应用中不需要使用。**    
如以下代码所示: 
```c
  auto xstream = xstream::XStreamSDK::CreateSDK();
  ASSERT_TRUE(xstream);
  XStreamAPITest::Callback callback;

  EXPECT_EQ(0, xstream->SetConfig("config_file", "./test/configs/basic.json"));
  EXPECT_EQ(0, xstream->SetConfig("profiler", "on"));
  EXPECT_EQ(0, xstream->SetConfig("profiler_file", "./profiler.txt"));
  EXPECT_EQ(0, xstream->Init());
  ASSERT_EQ(0, xstream->SetCallback(
    std::bind(&XStreamAPITest::Callback::OnCallback,
      &callback,
      std::placeholders::_1)));
  ASSERT_EQ(0, xstream->SetCallback(
    std::bind(&XStreamAPITest::Callback::OnCallback,
      &callback,
      std::placeholders::_1),
    "first_method"));
  xstream::InputDataPtr inputdata(new xstream::InputData());
  auto xstream_input_data = std::make_shared<xstream::BaseData>();
  xstream_input_data->name_ = "global_in";
  xstream_input_data->state_ = xstream::DataState::INVALID;
  inputdata->datas_.emplace_back(xstream_input_data);
  PromiseType p;
  auto f = p.get_future();
  inputdata->context_ = &p;
  std::string unique_name = "first_method";
  auto ipp = std::make_shared<xstream::TestParam>(unique_name);

  xstream->AsyncPredict(inputdata);
  auto output = f.get();
  EXPECT_EQ(output->error_code_, 0);
  EXPECT_EQ(output->datas_.size(), inputdata->datas_.size());
  EXPECT_EQ(output->datas_.front()->state_, inputdata->datas_.front()->state_);
  delete xstream;
```
* Example 同步运行模式
如以下代码所示:
```c
  auto xstream = xstream::XStreamSDK::CreateSDK();
  ASSERT_TRUE(xstream);
  EXPECT_EQ(0, xstream->SetConfig("config_file", "./test/configs/basic.json"));
  EXPECT_EQ(0, xstream->Init());
  xstream::InputDataPtr inputdata(new xstream::InputData());
  auto xstream_input_data = std::make_shared<xstream::BaseData>();
  xstream_input_data->name_ = "global_in";
  xstream_input_data->state_ = xstream::DataState::INVALID;
  inputdata->datas_.emplace_back(xstream_input_data);
  auto output = xstream->SyncPredict(inputdata);
  EXPECT_EQ(output->error_code_, 0);
  EXPECT_EQ(output->datas_.size(), inputdata->datas_.size());
  EXPECT_EQ(output->datas_.front()->state_, inputdata->datas_.front()->state_);
```
#### 创建SDK
  调用XStreamSDK的class静态接口CreateSDK, 创建一个XStreamSDK的对象。
代码:
```c
auto xstream = xstream::XStreamSDK::CreateSDK();
```
接口:
```c
static XStreamSDK *CreateSDK();
```

##### 设置XStream配置初始化
代码:
```c
EXPECT_EQ(0, xstream->SetConfig("config_file", "./test/configs/basic.json"));
EXPECT_EQ(0, xstream->SetConfig("profiler", "on"));
EXPECT_EQ(0, xstream->SetConfig("profiler_file", "./profiler.txt"));
EXPECT_EQ(0, xstream->SetConfig("free_framedata", "on"));
```

接口:
```c
virtual int SetConfig(const std::string &key, const std::string &value) = 0
```
#### XStream SDK初始化
根据前面几步的配置，初始化XStream。
```c
xstream->Init()
```
#### 定义和设置 callback
* 定义用户的Callback类,实现一个类似OnCallback函数，参数类型为
  `xstream::OutputDataPtr output`，用来处理XStream workflow的回调结果
代码:

```c 
class CallbackExample {
 public:
  void OnCallback(xstream::OutputDataPtr output) {
    ASSERT_TRUE(output);
    ASSERT_TRUE(output->context_);
    std::cout << "======================" << std::endl;
    std::cout << "seq: " << output->sequence_id_ << std::endl;
    std::cout << "output_type: " << output->output_type_ << std::endl;
    std::cout << "method_unique_name: " << output->unique_name_ << std::endl;
    std::cout << "error_code: " << output->error_code_ << std::endl;
    std::cout << "error_detail_: " << output->error_detail_ << std::endl;
    std::cout << "datas_ size: " << output->datas_.size() << std::endl;
    for (auto data : output->datas_) {
      if (data->error_code_ < 0) {
        std::cout << "data error: " << data->error_code_ << std::endl;
        continue;
      }
      std::cout << "data type_name : " << data->type_ << " " << data->name_
                << std::endl;
      BaseDataVector *pdata = reinterpret_cast<BaseDataVector *>(data.get());
      std::cout << "pdata size: " << pdata->datas_.size() << std::endl;
    }
  }
};
```

  * 设置callback
```c
MethodCallback::Callback callback;
xstream->SetCallback(
  std::bind(&MethodCallback::Callback::OnCallback,
    &callback,
    std::placeholders::_1));
```

#### 性能统计工具-profiler
xstream内部提供了性能统计的工具，用户可以通过xstream的对外接口SetConfig打开或关闭该功能，默认该功能关闭。
* 配置profiler功能

`int SetConfig(const std::string &key, const std::string &value)`

key为"profiler", value为"on", 表示打开性能统计功能。"off"表示关闭, 默认为关闭。   
key为"profiler_file", value为性能统计输出文件路径，用于设置性能统计文件的路径名称，框架层次的统计数据输出至该文件。

若程序中设置多个XStream SDK，则不同的sdk可以设置不同的配置。

```c
xstream1->SetConfig("profiler", "on");  // 设置打开profiler功能
xstream1->SetConfig("profiler_file", "./profiler_1.txt");  // 设置框架层统计数据输出文件

xstream2->SetConfig("profiler", "on");  // 设置打开profiler功能
xstream2->SetConfig("profiler_file", "./profiler_2.txt");  // 设置框架层统计数据输出文件
```
Method内的统计数据默认输出至文件"PROFILER_METHOD.txt", 用户若需要更改，可通过访问性能统计的全局单例设置智能策略内的统计数据：`Profiler::Get()->SetOutputFile("xx.txt")`

* 性能统计数据可视化

用户打开性能统计后，若统计结果输出到文件"profiler.txt"，可以通过访问`chrome://tracing/`，通过页面的`Load`按钮将文件加载，即可看到统计数据可视化界面。以下图为例，页面上的横坐标表示"时间"；前两行纵坐标表示对应Method处理的帧数；后两行纵坐标表示不同线程"thread_id"，同时页面左下角提供了工具以便用户查看数据细节。

![统计数据可视化](./doc/images/profiler.png "统计数据可视化")


#### 异步运行-输入数据
   * datas_: workflow输入的数据数组，要求非空。vector中每个数据对应一个数据slot，会送给workflow的
相关node，作为它们的输入。 在CV场景下，输入数据一般来源于senser或ISP处理后的视频帧数据。
   * params_: 对应请求的参数数组，可以为空。vector中每个数据对应一个具体node的参数。
   * source_id_: 在多路输入的场景下用于分输入源,单一源情况赋值为 0
   * context_: 透传的数据，该数据会透传到OutputData::context_字段, 通过该参数, 可以建立输入数据和输出数据的对应关系。
```c
/// 输入数据类型
struct InputData {
  /// 用户输入的数据，比如图片channel、时间戳、框等等。
  std::vector<BaseDataPtr> datas_;
  /// 当前请求自定义的参数
  std::vector<InputParamPtr> params_;
  /// 数据源 id 用于多路输入时区分输入源,单一源情况赋值为 0
  uint32_t source_id_ = 0;
  /// 透传的数据，该数据会透传到OutputData::context_ 字段
  const void *context_ = nullptr;
};
```
  调用异步接口AsyncPredict给XStream SDK传入数据。

```c
  /*模拟准备数据*/
  xstream::InputDataPtr inputdata(new xstream::InputData());
  auto xstream_input_data = std::make_shared<xstream::BaseData>();
  xstream_input_data->name_ = "image";
  xstream_input_data->state_ = xstream::DataState::INVALID;
  /*datas_在正式场景下需要填入真实数据*/
  inputdata->datas_.emplace_back(xstream_input_data);
  /*context_绑定输出结果*/
  PromiseType p;
  auto f = p.get_future();
  inputdata->context_ = &p;
```

#### 异步运行
* 异步运行
```c
  /*调用异步运行接口*/
  xstream->AsyncPredict(inputdata);
```

#### 同步运行-输入数据
与异步模式相比, 处理结果直接通过SyncPredict返回。
```c
  xstream::InputDataPtr inputdata(new xstream::InputData());
  auto xstream_input_data = std::make_shared<xstream::BaseData>();
  xstream_input_data->name_ = "global_in";
  xstream_input_data->state_ = xstream::DataState::INVALID;
  inputdata->datas_.emplace_back(xstream_input_data);
```

#### 同步运行-输出数据
调用 SyncPredict，直接返回运行的结果
```c
  auto output = xstream->SyncPredict(inputdata);
```

返回数据的结构:
```c
/// 输出数据类型
struct OutputData {
  /// 错误码
  int error_code_ = 0;
  /// 错误信息
  std::string error_detail_ = "";
  /// 当该OutputData为给某个node的定向回调结果时，该字段用于指示node的unique名称
  std::string unique_name_ = "";
  /// 多路输出结果名称
  std::string output_type_ = "";
  /// 输出结果
  std::vector<BaseDataPtr> datas_;
  /// 从InputData透传过来的数据
  const void *context_ = nullptr;
  /// 该结果的序列号
  int64_t sequence_id_ = 0;
  /// 该结果是属于那个输入源产生的结果
  uint32_t source_id_ = 0;
  uint64_t global_sequence_id_ = 0;
};
typedef std::shared_ptr<OutputData> OutputDataPtr;
```
#### 异步运行-多路输出数据
在接口使用上，与普通方式下没有区别，仅在回调函数如OnCallback中需注意，需要对output->datas_遍历，通过 output->output_type判断是否是目标group结果数据。 

#### 同步运行-多路输出数据
调用同步多路预测接口SyncPredict2，接口会阻塞住，直到整个workflow处理完成返回output的数组。里面包括多路数据结果，可以通过OutputData的output_type_字段区分。
```c
 for (int i = 0; i < 10; i++) {
    auto out = flow->SyncPredict2(inputdata);
    for (auto single_out : out) {
      callback.OnCallback(single_out);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if (i == 5) {
      std::string unique_name("BBoxFilter_1");
      auto ptr = std::make_shared<xstream::FilterParam>(unique_name);
      ptr->SetThreshold(90.0);
      flow->UpdateConfig(ptr->unique_name_, ptr);
    }
  }
```

### include 文件列表

|     目录     |      文件名      |                                     功能                                      |
| :----------: | :--------------: | :---------------------------------------------------------------------------: |
| hobotxstream |     method.h     |                        C++版头文件，定义了Method的基类                        |
| hobotxstream | method_factory.h |                      C++版头文件，定义了Method工厂类基类                      |
| hobotxstream |    profiler.h    |                    C++版头文件，定义了性能统计相关的类与宏                    |
|  hobotxsdk   |    version.h     |                 C版头文件，声明了XStream-Framework的版本信息                  |
|  hobotxsdk   |  xstream_data.h  | C++版头文件，定义了xstream-framework的数据类型、参数类型以及sdk输出的数据类型 |
|  hobotxsdk   | xstream_error.h  |                         定义了xstream-framework错误码                         |
|  hobotxsdk   |  xstream_sdk.h   |         C++版头文件，定义了sdk的接口，包含同步运行接口与异步运行接口          |

### 实现MethodFactory
 XStream在Init过程中需要调用method的工厂函数完成实例创建，MethodFactory的工厂函数会根据不同的method type名字返回对应的method的实例，workflow中用到的method都需要添加到该函数中，不然在构建sdk时会失败。   
 MethodFactory类定义在include/hobotxstream/method_factory.h中。
 ```c
 /// Method 工厂方法
  static MethodPtr CreateMethod(const std::string &method_name);
 ``` 
  实现CreateMethod方法， 在这个函数中根据method_name,返回具体的method实例。 method_name来源于Config文件中的method_type字段
 ```c
namespace xstream {
namespace method_factory
MethodPtr CreateMethod(const std::string &method_name) {
  if ("TestMethod" == method_name) {
    return MethodPtr(new TestMethod());
  } else if ("BBoxFilter" == method_name) {
    return MethodPtr(new BBoxFilter());
  } else if ("OrderTestMethod" == method_name) {
    return MethodPtr(new OrderTestThread());
  } else if ("threadsafeMethod" == method_name) {
    return MethodPtr(new SafeTestThread());
  } else if ("passthroughMethod" == method_name) {
    return MethodPtr(new passthroughMethod());
  } else {
    return MethodPtr();
  }
}
}  // namespace method_factory
}  // namespace xstream
 ```
### 通过Config文件定义workflow
#### 常用配置
```json
{
  "inputs": ["face_head_box"],
  "outputs": ["face_head_box_filter2"],
  "workflow": [
    {
      "thread_count": 3,
      "method_type": "BBoxFilter",
      "unique_name": "BBoxFilter_1",
      "inputs": [
        "face_head_box"
      ],
      "outputs": [
        "face_head_box_filter"
      ],
      "method_config_file": "null"
    },
    {
      "thread_count": 3,
      "method_type": "BBoxFilter",
      "unique_name": "BBoxFilter_2",
      "inputs": [
        "face_head_box_filter"
      ],
      "outputs": [
        "face_head_box_filter2"
      ],
      "method_config_file": "null"
    }
  ]
}
```
*说明*

**inputs** : 
	workflow的input数据名称:
    对应于以下代码中的name_   
```c 
   auto xstream_input_data = std::make_shared<xstream::BaseData>();
   xstream_input_data->name_ = "face_head_box";
   xstream_input_data->state_ = xstream::DataState::INVALID;
```                              
**outputs** : 
    workflow的output数据名称:
    对应于callback返回中的  `data->name_`
```c 
    workflow的output 参数。
 for (auto data : output->datas_) {
      std::cout << "output name：" << data->name_
                << std::endl;
      BaseDataVector *pdata = reinterpret_cast<BaseDataVector *>(data.get());
      std::cout << "pdata size: " << pdata->datas_.size() << std::endl;
    }
```  
**workflow**:           
    定义worflow拓扑结构, 由node的数组组成。

**子workflow**:
[XStream-Framework 子workflow说明](./doc/SubWorkflow.md)

***Node***   
**——thread_count** :   
    设置node的执行线程数量，在method属性thread_safe=false的情况下，会创建thread_count的method实例，连续多次输入，默认会按Round-bin的方式分发到对应线程（或实例）。如果在node内部执行时存在阻塞操作，如调用BPU接口进行模型计算时，增加thread_count值，有可能可以提高workflow的整体性能。
    
**——method_type** :    
    method_type 对应于CreateMethod 中的method_name，
    
**——unique_name**
    node的唯一名称，同一种类型的method可以实例化多个不同的node，通过unique_name来区分。

**——inputs**
    该node的输入。

**——outputs**
    该node输出数据。
*注：node的inputs,和outputs属性，决定了数据的流向，从而确定了node的在workflow类AOV网中的关系。  
**——method_config_file**
    指定method局部配置文件路径;路径是workflow配置文件所在位置的相对路径。
*注 指定的是配置文件路径，在method::Init中需要从该路径重新读取进而解析Json config信息。
```c 
       /// 初始化
  virtual int Init(const std::string &config_file_path) = 0;
``` 

#### 指定线程优先级的配置
以下的workflow config文件中，对thread线程的设置与上面不同，通过thread_list设置node的执行线程数，并可通过thread_priority来设置不同的线程优先级。
```json
{
  "inputs": ["face_head_box"],
  "outputs": ["face_head_box_filter2"],
  "optional":
  {
    "sched_upper":
    {
      "policy": "SCHED_FIFO",
      "priority": 30
    },
    "sched_down":
    {
      "policy": "SCHED_FIFO",
      "priority": 30
    }
  },
  "workflow": [
    {
      "thread_list": [0],
      "thread_priority":
      {
        "policy": "SCHED_FIFO",
        "priority": 10
      },
      "method_type": "BBoxFilter",
      "unique_name": "BBoxFilter_1",
      "inputs": [
        "face_head_box"
      ],
      "outputs": [
        "face_head_box_filter"
      ],
      "method_config_file": "sched_fifo0.json"
    },
    {
      "thread_list": [1, 2],
      "thread_priority":
      {
        "policy": "SCHED_FIFO",
        "priority": 20
      },
      "method_type": "BBoxFilter",
      "unique_name": "BBoxFilter_2",
      "inputs": [
        "face_head_box_filter"
      ],
      "outputs": [
        "face_head_box_filter2"
      ],
      "method_config_file": "sched_fifo1.json"
    }
  ]
}
``` 

| thread priority成员 | 值                           | 详细说明                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| :------------------ | :--------------------------- | :-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| policy              | SCHED_OTHER or SCHED_NORMAL: | linux默认调度优先级，分时调度策略、CFS（Completely Fair Scheduler,完全公平调度策略），Ready的线程在等待队列等待时间越长，优先级越高；                                                                                                                                                                                                                                                                                                                                             |
|                     | SCHED_FIFO                   | 1）是一种实时调用策略，优先级高于SCHED_OTHER；可设置线程优先级范围1~99，值越大优先级越高；当SCHED_FIFO的线程状态为runable时，会立即抢占SCHED_OTHER的线程; 2）如果一个SCHED_FIFO线程被一个更高优先级的线程抢占，该线程会放在相同优先级线程的队首；当一个SCHED_FIFO的线程状态变成runnable时，该线程放在相同优先级线程的队尾；3）一个SCHED_FIFO时一种不带时间片的先入先出调度策略；让出cpu要么因为本身被IO blocked，要么被更高优先级线程给抢占了，要么自己主动调了sched_yield让出cpu |
|                     | SCHED_RR                     | 1）也是一种实时调度策略，优先级高于SCHED_OTHER；可设置线程优先级范围1~99，值越大优先级越高；2）SCHED_RR调度策略本身是SCHED_FIFO的简单增强版，两者大部分属性是一样的；区别在于对于相同优先级的线程，SCHED_RR对于相同优先级的线程也是采用时间片轮转的方式，一个线程做完自己的时间片之后就放在该优先级线程的队尾，反之SCHED_FIFO不会主动让出线程；                                                                                                                                   |
|                     | SCHED_BATCH                  | SCHED_BATCH是为批处理任务设计的优先级调度策略，SCHED_IDLE的线程优先级特别低；跟SCHED_OTHER调度策略一样，优先级恒为0，不能设置；                                                                                                                                                                                                                                                                                                                                                   |
|                     | SCHED_BATCH                  | SCHED_DEADLINE顾名思义，是一种可以给线程设置deadline的调度策略；                                                                                                                                                                                                                                                                                                                                                                                                                  |
| priority            | 1~99                         | 仅在 policy 设置为 SCHED_FIFO， SCHED_RR 时可以设置                                                                                                                                                                                                                                                                                                                                                                                                                               |

**optional**   
**——sched_upper** : workflow调度线程   
**——sched_down** :  workflow中node的公共守护线程   ***   
***如果设置线程优先级，建议这两个线程的优先级高于每个node的线程优先级***   
***node***   
**——thread_list** :  指定了node执行线程index的数组，基于thread_list可以实现多个node之间共享执行线程。   
**——thread_priority** : 线程的优先级设置。  
 
```json
"thread_list": [0],
      "thread_priority":
      {
        "policy": "SCHED_FIFO",
        "priority": 10
      },
```

#### 多路输出配置
多路输出应用于这样的场景: FrameWork数据在一些node运行结束后，产生了一些workflow所需的output数据。这些output数据，如果按通常的配置方法，要等到所有workflow中所有node运行完成后，XStream的调用者才能通过异步回调，或者同步运行后获取结果。这样，调用者获得目标结果的数据会相对比较晚。多路输出功能，为缩短一些output数据的返回时间，提供这样的机制，用户可以将输出数据分为多路输出，但某路数据经过一些node，达到完成状态时，即可通过回调函数返回结果，即使这路数据还要参与后续的node计算。多路输出一般配合XStream SDK的异步调用方式使用。
```json
{
  "inputs": ["face_head_box"],
  "outputs": [
    {
      "output_type": "out1",
      "outputs":["face_head_box_filter"]
    },
    {
      "output_type": "out2",
      "outputs":["face_head_box_filter2"]
    }
  ],
  "workflow": [
    {
      "thread_count": 3,
      "method_type": "BBoxFilter",
      "unique_name": "BBoxFilter_1",
      "inputs": [
        "face_head_box"
      ],
      "outputs": [
        "face_head_box_filter"
      ],
      "method_config_file": "null"
    },
    {
      "thread_count": 3,
      "method_type": "BBoxFilter",
      "unique_name": "BBoxFilter_2",
      "inputs": [
        "face_head_box_filter"
      ],
      "outputs": [
        "face_head_box_filter2"
      ],
      "method_config_file": "null"
    }
  ]
}
```
每帧数据FrameWork Data有序列号sequence_id标记，还需增加每路输出OutputData的唯一标记output_type_，在config文件中获取。
多路输出的Json配置方式为：
```json
 "outputs": [
    {
      "output_type": "out1",
      "outputs":["face_head_box_filter"]
    },
    {
      "output_type": "out2",
      "outputs":["face_head_box_filter2"]
    }
  ],
```
### 数据类型
#### 错误码
```c++
// xstream_error.h
#define HOBOTXSTREAM_ERROR_CODE_OK  0

#define HOBOTXSTREAM_ERROR_INPUT_INVALID               -1000
#define HOBOTXSTREAM_ERROR_EXCEED_MAX_RUNNING_COUNT    -1001

#define HOBOTXSTREAM_ERROR_METHOD              -2000
#define HOBOTXSTREAM_ERROR_METHOD_TIMEOUT      -2001
#define HOBOTXSTREAM_ERROR_OUTPUT_NOT_READY    -2002
```

#### 基础数据结构
##### DataState
```c++
// xstream_data.h
enum class DataState {
  VALID = 0,
  FILTERED = 1,
  INVISIBLE = 2,
  DISAPPEARED = 3,
  INVALID = 4,
};
```
说明：用于描述数据的状态   

|    成员     |                             详细说明                             |
| :---------: | :--------------------------------------------------------------: |
|    VALID    |                             数据有效                             |
|  FILTERED   |                   表示该数据被过滤模块过滤掉了                   |
|  INVISIBLE  | 表示该数据对应的对象未检测出来，但是可以根据前后帧的关系推测出来 |
| DISAPPEARED |           表示该数据对应的对象消失了，不在跟踪状态中了           |
|   INVALID   |                           表示数据无效                           |

##### BaseData
```c++
// xstream_data.h
struct BaseData {  // 数据结构基类，框，lmk等等的基类
  BaseData();
  virtual ~BaseData();

  std::string type_ = "";
  std::string name_ = "";
  int error_code_ = 0;
  std::string error_detail_ = "";
  std::shared_ptr<CContext> c_data_;
  DataState state_ = DataState::VALID;
};

typedef std::shared_ptr<BaseData> BaseDataPtr;
```
说明： XStream数据交互的基础数据类型，包含了数据的类型、状态等描述信息，workflow中流通的message都需要继承该基类。 

|     成员      |                         详细说明                         |
| :-----------: | :------------------------------------------------------: |
|     type_     |                    数据类型描述字符串                    |
|     name_     | 数据名称，用于workflow的串联，workflow配置文件中需要使用 |
|  error_code_  |                          错误码                          |
| error_detail_ |                      错误描述字符串                      |
|    c_data_    |            用于获取C语言接口SDK内部上下文信息            |
|    state_     |                       数据状态信息                       |

##### BaseDataVector
```c++
// xstream_data.h
struct BaseDataVector : public BaseData {
  BaseDataVector();

  std::vector<BaseDataPtr> datas_;
};
```
说明： 用于表示一组数据，比如一张图片的多个检测框之类。

|  成员  |                            详细说明                            |
| :----: | :------------------------------------------------------------: |
| datas_ | vector用于保存数组数据，数组中每个成员抽象为BaseData类型的指针 |

##### XStreamData
```c++
// xstream_data.h
template<typename Dtype>
struct XStreamData : public BaseData {
  Dtype value;
};
```
说明: 模版类，用于将已有的数据类型转换成BaseData类型
##### InputParam
```c++
// xstream_data.h
class InputParam {  // 输入数据的自定义参数
 public:
  explicit InputParam(std::string unique_name) {
    unique_name_ = unique_name;
    is_json_format_ = false;
    is_enable_this_method_ = true;
  }
  virtual ~InputParam() = default;
  virtual std::string Format() = 0;

 public:
  bool is_json_format_;
  bool is_enable_this_method_;
  std::string unique_name_;
};

typedef std::shared_ptr<InputParam> InputParamPtr;
```
说明： 用于定义输入参数。  

|          成员          |                                                                           详细说明                                                                            |
| :--------------------: | :-----------------------------------------------------------------------------------------------------------------------------------------------------------: |
|    is_json_format_     |                                           输入参数是否是json字符串。若是json字符串，可以通过Format()接口获取字符串                                            |
| is_enable_this_method_ | 是否使能该字符串。若不使能该node，则workflow运行过程中不会调用该node的DoProcess接口，此时该node的输出可以使用传入的预设的node输出或者直接透传前一个node的结果 |
|      unique_name_      |                                                                  method实例node的unique名称                                                                   |
|        Format()        |                                                                     返回具体的参数字符串                                                                      |

##### DisableParam
```c++
// xstream_data.h
class DisableParam : public InputParam {
 public:
  enum class Mode {
    /// 输入数据透传到输出，要求输入输出大小一致
    PassThrough,
    /// 拷贝先验数据到输出，要求先验数据大小与输出大小一致
    UsePreDefine,
    /// 令每个输出都是INVALID的BaseData
    Invalid
  };
  explicit DisableParam(const std::string &
  unique_name, Mode mode = Mode::Invalid) : InputParam(unique_name), mode_{mode} {
    is_enable_this_method_ = false;

  }
  virtual ~DisableParam() = default;
  virtual std::string Format() {
    return unique_name_ + " : disabled";
  }
  Mode mode_;
  /// 先验数据，用于填充node输出
  std::vector<BaseDataPtr> pre_datas_;
};

typedef std::shared_ptr<DisableParam> DisableParamPtr;
```
说明：一种特定的输入参数，用于控制node Skip条件，目前支持Disable、passthrough、UserPredefined三种Skip条件。   

|    成员    |                               详细说明                               |
| :--------: | :------------------------------------------------------------------: |
|   mode_    |    表示使用数据透传方式还是使用用户传入的先验结果作为该node的输出    |
| pre_datas_ | 若用户需要传入的先验结果作为该node输出，则通过该成员变量保存先验结果 |

##### SdkCommParam
```c++
// xstream_data.h
class SdkCommParam : public InputParam {
 public:
  SdkCommParam(std::string unique_name, std::string param) : InputParam(unique_name) {
    param_ = param;
    is_json_format_ = true;
  }
  virtual std::string Format() { return param_; }
  virtual ~SdkCommParam() = default;

 public:
  std::string param_;   // json格式
};
typedef std::shared_ptr<SdkCommParam> CommParamPtr;
```
说明： 一种Json格式的框架通用输入参数类型。  

|  成员  |         详细说明         |
| :----: | :----------------------: |
| param_ | json字符串格式的参数类型 |

##### InputData
```c++
// xstream_data.h
struct InputData {  // 输入数据类型
  std::vector<BaseDataPtr>
      datas_;  // 用户输入的数据，比如图片channel、时间戳、框等等
  std::vector<InputParamPtr> params_;  // 当前请求自定义的参数

  const void *context_ = nullptr;
};
typedef std::shared_ptr<InputData> InputDataPtr;
```
说明：XStream-Framework同步运行与异步运行接口传入的输入数据，包含数据、控制参数等。   

|  成员   |                                                  详细说明                                                  |
| :-----: | :--------------------------------------------------------------------------------------------------------: |
| datas_  | workflow输入的数据，要求非空。vector中每个数据对应一个数据slot，会送给workflow的相关node，作为它们的输入。 |
| params_ |                     对应请求的参数，可以为空。vector中每个数据对应一个具体node的参数。                     |
 
##### OutputData
```c++
// xstream_data.h
struct OutputData {
  /// 错误码
  int error_code_ = 0;
  /// 错误信息
  std::string error_detail_ = "";
  /// 当该OutputData为给某个node的定向回调结果时，该字段用于指示node的unique名称
  std::string unique_name_ = "";
  /// 多路输出结果名称
  std::string output_type_ = "";
  /// 输出结果
  std::vector<BaseDataPtr> datas_;
  /// 从InputData透传过来的数据
  const void *context_ = nullptr;
  /// 该结果的序列号
  int64_t sequence_id_ = 0;
  /// 该结果是属于那个输入源产生的结果
  uint32_t source_id_ = 0;
  uint64_t global_sequence_id_ = 0;
};
typedef std::shared_ptr<OutputData> OutputDataPtr;
typedef std::shared_ptr<OutputData> OutputDataPtr;
```
说明： XStream-Framework同步运行与异步运行接口返回的输出结果；也可以作为单个node处理完返回的结果，供对应的回调函数捕获

|     成员      |                                                    详细说明                                                     |
| :-----------: | :-------------------------------------------------------------------------------------------------------------: |
|  error_code_  |                                              错误码，无错误则应为0                                              |
| error_detail_ |                                                  错误描述信息                                                   |
| unique_name_  |                 node名称，对于XStream-Framework同步运行与异步运行接口返回的输出结果，该字段为空                 |
| output_type_  | 返回的输出类型信息，多路输出情况下对应workflow outputs中定义的type信息，单路输出类型为"__NODE__WHOLE_OUTPUT__ " |
|    datas_     |                                               返回的具体数据结果                                                |
|   context_    |                                 分析任务传递的用户数据，在结果中会进行透传回来                                  |

##### XStreamCallback
```c++
typedef std::function<void(OutputDataPtr)> XStreamCallback;
```
说明： XStream-Framework支持的回调函数格式：返回值为void，有且只有一个形参，形参类型为OutputDataPtr
