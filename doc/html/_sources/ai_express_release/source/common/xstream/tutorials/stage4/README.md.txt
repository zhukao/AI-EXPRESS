## 关闭指定的method实例
在一些应用场景中，我们需要根据应用场景来关闭某些method实例计算节点node。例如, 通过外部传入人脸照片，提取特征时，创建底库时, 需要运行人脸检测，特征提取计算, 但不需要再进行人脸mot跟踪, mot method的计算节点就可以关闭。 在关闭不必要的计算节点的情况下，可以降低芯片的功耗，降低系统的延迟。
XSTREAM提供了一下几种关闭Method的方式：

| 模式    | 详细说明|
| :-----: |:------:|
| Invalid | 令每个输出都是INVALID的BaseData    |
| PassThrough  |  透传输入数据到输出，要求输入数据与输出数据个数一致,且类型相同 |
| UsePreDefine |  拷贝先验数据到输出，要求先验数据个数与输出数据个数一致, 且类型相同 |
| BestEffortPassThrough | 按顺序逐个透传输入数据到输出， 如果输入数据大小多于输出，则只透传前面的slot。如果输入数据大小少于输出，则多余的输出slot为Invalid的BaseData |

在前面stage1, stage2, stage3的基础上, 我们来看如何进行计算节点node的控制

### Invalid Mode
准备一个workflow如下(见`config/control_workflow_disablemethod.json`), 这个workflow的输入数据face_head_box, 最终输出数据是face_head_box_filter_3。face_head_box_filter_5。
```json
{
  "max_running_count": 10000,
  "inputs": ["face_head_box"],
  "outputs": ["face_head_box_filter_2", "face_head_box_filter_3"],
  "workflow": [
    {
      "thread_count": 3,
      "method_type": "BBoxFilter",
      "unique_name": "BBoxFilter_1",
      "inputs": [
        "face_head_box"
      ],
      "outputs": [
        "face_head_box_filter_1"
      ],
      "method_config_file": "null"
    },
    {
      "thread_count": 3,
      "method_type": "BBoxFilter",
      "unique_name": "BBoxFilter_2",
      "inputs": [
        "face_head_box_filter_1"
      ],
      "outputs": [
        "face_head_box_filter_2"
      ],
      "method_config_file": "null"
    },
	  {
      "thread_count": 3,
      "method_type": "BBoxFilter",
      "unique_name": "BBoxFilter_3",
      "inputs": [
        "face_head_box_filter_2"
      ],
      "outputs": [
        "face_head_box_filter_3"
      ],
      "method_config_file": "null"
    }
  ]
}
```
正常运行控制下输入数据 "**face_head_box**"  经BBoxFilter_1节点处理输出数据"**face_head_box_filter_1**"。数据"**face_head_box_filter_1**"经由BBoxFilter_2节点处理产生数据"**face_head_box_filter_2**"。"**face_head_box_filter_2**"经BBoxFilter_3节点处理产生数据"**face_head_box_filter_3**", 既最终输出结果之一。workflow运行图示:

![Invalid1](doc/Invalid1.png)

我们为实现这个案例，创建了几个Method:`BBoxFilter`,`PostBoxFilter`:
(见`method/bbox_filter.cc`,`method/bbox_filter.h`
`method/postbox_filter.cc`,`method/postbox_filter.h`)。这几个method, 通过`config/`下的workflow配置文件和method_factory工厂方法，共同组成Invalid和后面几种模式的案例。

在一些场景中，我们希望暂时停止BBoxFilter_3的运行，例如人脸识别业务。
下面我们来看下如何实现的，示例代码见`disable_method_main.cc`。在不加控制参数的情况下，我们通过打印看下数据输出，我们可以看到两组输出数据"**face_head_box_filter_2**", "**face_head_box_filter_3**"，state均为VALID:0。
```
============Output Call Back============
—seq: 0
—output_type: __NODE_WHOLE_OUTPUT__
—error_code: 0
—error_detail_: 
—datas_ size: 2
——output data face_head_box_filter_2 state:0
——data type:BaseDataVector name:face_head_box_filter_2
——output data face_head_box_filter_3 state:0
——data type:BaseDataVector name:face_head_box_filter_3
============Output Call Back End============
```
#### demo 1
接下来，我们准备关闭节点`BBoxFilter_3`。
在准备Input Data这步, 构建一个DisableParam对象给HobotXStream::InputParamPtr共享指针,加入到Input data的params_参数中
* 准备Input Data数据
```c++
  InputDataPtr inputdata(new InputData());
  BaseDataVector *data(new BaseDataVector);
  HobotXStream::BBox *bbox1(new HobotXStream::BBox(
    hobot::vision::BBox(0, 0, 40, 40)));
  bbox1->type_ = "BBox";
  data->name_ = "face_head_box";
  data->datas_.push_back(BaseDataPtr(bbox1));
  inputdata->datas_.push_back(BaseDataPtr(data));
```
* 为Input Data增加参数
```c++
  // 2 set DisableParam Invalid for "BBoxFilter_3"
  std::cout << "------------ invalid output------------" << std::endl;
  // DisableParam set mode with default value Mode::Invalid
  HobotXStream::InputParamPtr invalidFilter3(new HobotXStream::DisableParam("BBoxFilter_3"));
  // add invalid parameter to inputdata params_
  inputdata->params_.push_back(invalidFilter3);
  // start synchronous predict, and then 
  out = flow->SyncPredict(inputdata);
  callback.OnCallback(out);
```
这里我们通过构造函数传入"BBoxFilter_3", 既指定method实例计算节点node的unique name为"BBoxFilter_3"将被关闭。HobotXStream::DisableParam构造函数 `DisableParam(const std::string & unique_name, Mode mode = Mode::Invalid)`设置mode默认值为Mode::Invalid，因此`HobotXStream::DisableParam("BBoxFilter_3")`等同于
`HobotXStream::DisableParam("BBoxFilter_3", Mode::Invalid)`
我们再看下打印输出，这时输出数据"**face_head_box_filter_3**"，state均为INVALID:4。在这次workflow运行过程中, "BBoxFilter_3"节点没有被调用到，它的输出数据被标记为INVALID。
```
============Output Call Back============
—seq: 1
—output_type: __NODE_WHOLE_OUTPUT__
—error_code: 0
—error_detail_: 
—datas_ size: 2
——output data face_head_box_filter_2 state:0
——data type:BaseDataVector name:face_head_box_filter_2
——output data face_head_box_filter_3 state:4
——data type:BaseData name:face_head_box_filter_3
============Output Call Back End============
```
经过DisableParam参数控制后的workflow运行图如下所示:

![Invalid2](doc/Invalid2.png)

#### demo 2
我们再来看下如果关闭"BBoxFilter_3"的前置节点"BBoxFilter_2"又会如何。
同样设置构建一个DisableParam对象给HobotXStream::InputParamPtr共享指针,加入到Input data的params_参数中。
如果在inputdata的params数组记得需要清理下，否则几条DisableParam规则会一起生效.
```c++
  // 3 set DisableParam Invalid output "BBoxFilter_2"
  std::cout << "------------ invalid output------------" << std::endl;
  // DisableParam set mode with default value Mode::Invalid
  HobotXStream::InputParamPtr invalidFilter2(new HobotXStream::DisableParam("BBoxFilter_2"));
  // add invalid parameter to inputdata params_
  inputdata->params_.clear();
  inputdata->params_.push_back(invalidFilter2);
  // start synchronous predict, and then 
  out = flow->SyncPredict(inputdata);
  callback.OnCallback(out);
```

再次运行workflow,因为我们看到因为BBoxFilter_3输入数据"**face_head_box_filter_2**"的state是INVALID:4,前置条件不满足。打印输出如下，
error_code:-2002 HOBOTXSTREAM_ERROR_OUTPUT_NOT_READY，
error_detail:face_head_box_filter_3 is not ready;
```
============Output Call Back============
—seq: 2
—output_type: __NODE_WHOLE_OUTPUT__
—error_code: -2002
—error_detail_: face_head_box_filter_3 is not ready;
—datas_ size: 1
——output data face_head_box_filter_2 state:4
——data type:BaseData name:face_head_box_filter_2
============Output Call Back End============
```
经过DisableParam参数控制后的workflow运行图示:

![Invalid3](doc/Invalid3.png)

因此关闭节点,会影响到输出数据的后置节点的输出。在这个例子中节点"BBoxFilter_2"只影响到了节点"BBoxFilter_3"。因此从输出数据的结果看，有效结果与关闭节点"BBoxFilter_3"相同。



### PassThrough Mode
场景:在给计算节点设置Invalid的案例2中,我们看到如果关闭中间节点, 会导致后置节点计算条件不满足, 而无法运行。那么在一些场景下,我们需要关闭前置节点的计算功能,但不影响后置节点的运行。例如,有些中间节点,进行数据的过滤,在有些特殊场景下,我们需要即时关闭这些过滤,让数据越过这个节点,直接让后续节点处理。这时PassThrough模式就可以上场了，当然PassThrough模式要求，关闭节点的输入数据和输出数据个数(InputData.data_.size())是一致的。
```json
{
  "max_running_count": 10000,
  "inputs": ["face_head_box"],
  "outputs": ["face_head_box_filter_2"],
  "workflow": [
    {
      "thread_count": 3,
      "method_type": "BBoxFilter",
      "unique_name": "BBoxFilter_1",
      "inputs": [
        "face_head_box"
      ],
      "outputs": [
        "face_head_box_filter_1"
      ],
      "method_config_file": "null"
    },
    {
      "thread_count": 3,
      "method_type": "PostBoxFilter",
      "unique_name": "PostBoxFilter_2",
      "inputs": [
        "face_head_box_filter_1"
      ],
      "outputs": [
        "face_head_box_filter_2"
      ],
      "method_config_file": "null"
    }
  ]
}
```
这里我们设置了一个简单的workflow(见`config/control_workflow_pass_through.json`)。输入数据是"**face_head_box**", 输出数据是"**face_head_box_filter_2**"。第一个节点"BBoxFilter_1"处理输出数据"**face_head_box_filter_1**",
经"BBoxFilter_2"处理后生成输出数据"**face_head_box_filter_2**"。
示例代码见`pass_through_method_main.cc`,首先按worflow的定义, 给workflow输入一个name是"face_head_box",type是"BBOX"的输入数据
```c++
  // 2 prepare input data
  InputDataPtr inputdata(new InputData());
  BaseDataVector *data(new BaseDataVector);
  HobotXStream::BBox *bbox1(new HobotXStream::BBox(
    hobot::vision::BBox(0, 0, 60, 60)));
  bbox1->type_ = "BBox";
  data->name_ = "face_head_box";
  data->datas_.push_back(BaseDataPtr(bbox1));
  inputdata->datas_.push_back(BaseDataPtr(data)
```
我们先按正常模式运行一遍,我们看下结果如下。workflow按照预期给出输出数据face_head_box_filter_2
```
============Output Call Back============
—seq: 0
—output_type: __NODE_WHOLE_OUTPUT__
—error_code: 0
—error_detail_: 
—datas_ size: 1
——output data face_head_box_filter_2 state:0
——data type_name : BaseDataVector face_head_box_filter_2
============Output Call Back End============
```
用图来表示workflow的运行：

![PassThrough1](doc/PassThrough1.png)

接下来，我们尝试用PassThrough方式关闭`PostBoxFilter_2`节点。创建一个DisableParam对象给HobotXStream::InputParamPtr共享指针,加入到Input data的params_参数中，在DisableParam的构造函数中, 设置mode 为PassThrough: `HobotXStream::InputParamPtr
      pass_through(new HobotXStream::DisableParam("PostBoxFilter_2", HobotXStream::DisableParam::Mode::PassThrough)`
```c++
// 4 set pass through output
  std::cout << "------------ pass through output----------" << std::endl;
  inputdata->params_.clear();
  HobotXStream::InputParamPtr
      pass_through(new HobotXStream::DisableParam("PostBoxFilter_2", HobotXStream::DisableParam::Mode::PassThrough));
  inputdata->params_.push_back(pass_through);
  out = flow->AsyncPredict(inputdata);
  usleep(1000000);
```
注意,这次我们使用了异步的调用方式,为了通过日志输出来确定执行结果,我们让主线程sleep 1秒以获得日志输出。
同时为了确定"PostBoxFilter_2"节点没有调用到，在PostBoxFilter::DoProcess中增加了日志输出。
这时候，我们再用图来看下workflow的运行：

![PassThrough-2](doc/PassThrough-2.png)

运行后的日志如下，"PostBoxFilter_2"节点没有运行，输出数据既"PostBoxFilter_2"节点输入数据face_head_box_filter_1更替为face_head_box_filter_2
```
============Output Call Back============
—seq: 1
—output_type: __NODE_WHOLE_OUTPUT__
—error_code: 0
—error_detail_: 
—datas_ size: 1
——output data face_head_box_filter_2 state:0
——data type_name : BaseDataVector face_head_box_filter_2
============Output Call Back End============
```

###  Use Predefined Mode
场景：在一些启动初始化过程，调试场景中，通过这个Use Predefined模式中，用户可以通过DisableParam参数不仅关闭某个计算节点，并使本应其产生的数据改成DisableParam参数自带的数据。
构建如`config/control_workflow_use_predefined.json`中的workflow。这个workflow中仅有"BBoxFilter_1"和"PostBoxFilter_2"两个前后计算节点。
```json
{
  "max_running_count": 10000,
  "inputs": ["face_head_box"],
  "outputs": ["face_head_box_filter_2"],
  "workflow": [
    {
      "thread_count": 3,
      "method_type": "BBoxFilter",
      "unique_name": "BBoxFilter_1",
      "inputs": [
        "face_head_box"
      ],
      "outputs": [
        "face_head_box_filter_1"
      ],
      "method_config_file": "null"
    },
    {
      "thread_count": 3,
      "method_type": "PostBoxFilter",
      "unique_name": "PostBoxFilter_2",
      "inputs": [
        "face_head_box_filter_1"
      ],
      "outputs": [
        "face_head_box_filter_2"
      ],
      "method_config_file": "null"
    }
  ]
}
```
workflow运行过程可见

![PreDefined-1](doc/PreDefined-1.png)

如下代码(见`use_predefined_method_main.cc`)，首先构建DisableParamPtr对象pre_define, 关闭"BBoxFilter_1"节点，将mode设置为UsePreDefine。
```c++
 // 3 use pre-defined input data
  std::cout << "------------ pre-defined output----------" << std::endl;
  inputdata->params_.clear();
  HobotXStream::DisableParamPtr
      pre_define(
        new HobotXStream::DisableParam(
          "BBoxFilter_1",
          HobotXStream::DisableParam::Mode::UsePreDefine));
  
  // 4 preprea new input data   
  BaseDataVector *pre_data(new BaseDataVector);
  HobotXStream::BBox *pre_bbox1(new HobotXStream::BBox(
    hobot::vision::BBox(0, 0, 20, 20)));
  pre_bbox1->type_ = "BBox";
  pre_data->name_ = "face_head_box";
  pre_data->datas_.push_back(BaseDataPtr(pre_bbox1));
  pre_define->pre_datas_.emplace_back(BaseDataPtr(pre_data));

  inputdata->params_.push_back(pre_define);
  auto out = flow->SyncPredict(inputdata);
```
接下来构建BaseDataVector,和准备一个Input数据基本一致，区别在于将这个新数据pre_data加入pre_define的pre_datas_成员中。新的数据我们设置BOX大小为(0, 0, 20, 20)的矩形。将DisableParamPtr对象pre_define设置为Input参数后，我们再运行一次workflow，我们来看下结果是怎样的。

![PreDefined-2](doc/PreDefined-2.png)


这次的日志输出如下，PostBoxFilter_2节点的PostBoxFilter Method,在"BBoxFilter_1"关闭的情况下，收到的数据不是输入数据Box(0, 0, 60, 60), 而是控制参数重新定义的(0,0,20,20)。在这之后的流程中PostBoxFilter_2按输出数据(0,0,20,20)进行计算，输出结果数据face_head_box_filter_2

```
------------ pre-defined output----------
PostBoxFilter::DoProcess
input size: 1
filter out: 0,0,20,20
============Output Call Back============
—seq: 0
—output_type: __NODE_WHOLE_OUTPUT__
—error_code: 0
—error_detail_: 
—datas_ size: 1
——data type:BaseDataVector name:face_head_box_filter_2
pdata size: 0
============Output Call Back End============
```

### Best Effort PassThrough Mode
场景：在PassThrough的场景下，输入数据个数和输出数据个数刚好相等，或者只做修改，是非常特殊的情况，更多的实际场景下，输入数据和输出数据并不匹配。因此，这里提供一个更灵活的关闭节点模式**Best Effort PassThrough Model**。在这种模式下，输入数据大小多于输出，则只透传前面的slot。如果输入数据大小少于输出，则多余的输出slot为Invalid的BaseData。BestEffortPassThrough是PassThrough的改进版本，多数情况下更推荐使用estEffortPassThrough模式。
接下来，我们看下这种模式如何使用。

#### demo1
首先，我们构造一个workflow如下,见`config/control_workflow_best_effort_use_predefined.json`, 输入数据是face_head_box,输出数据是"***face_head_box_filter_3***", "***face_head_box_filter_2***", "***post_box_filter_1***"。节点"BBoxFilter_1"将输入数据处理后转化成"***face_head_box_filter_1***","***face_head_box_filter_2***"后输出。"BBoxFilter_3"将"***post_box_filter_1***"
处理后生成"***face_head_box_filter_3***", "PostBoxFilter_4"将"***face_head_box_filter_2***"处理后生成"***post_box_filter_1***"。
```json
{
  "max_running_count": 10000,
  "inputs": ["face_head_box"],
  "outputs": ["face_head_box_filter_3", "face_head_box_filter_2", "post_box_filter_1"],
  "workflow": [
    {
      "thread_count": 3,
      "method_type": "BBoxFilter",
      "unique_name": "BBoxFilter_1",
      "inputs": [
        "face_head_box"
      ],
      "outputs": [
        "face_head_box_filter_1",
        "face_head_box_filter_2"
      ],
      "method_config_file": "null"
    },
    {
      "thread_count": 3,
      "method_type": "BBoxFilter",
      "unique_name": "BBoxFilter_3",
      "inputs": [
        "face_head_box_filter_1"
      ],
      "outputs": [
        "face_head_box_filter_3"
      ],
      "method_config_file": "null"
    },
    {
      "thread_count": 3,
      "method_type": "PostBoxFilter",
      "unique_name": "PostBoxFilter_4",
      "inputs": [
        "face_head_box_filter_2"
      ],
      "outputs": [
        "post_box_filter_1"
      ],
      "method_config_file": "null"
    }
  ]
}
```
正常情况下，以上的workflow应该如下图运行

![BEPassThrough-1](doc/BEPassThrough-1.png)

接下来，我们看下使用BestEffortPassThrough的情况下，workflow的运行。
首先创建一个DisableParam对象给HobotXStream::InputParamPtr共享指针，DisableParam 构造函数中设置目标计算节点Node的unique name是"BBoxFilter_1"，既关闭第一个计算节点。"BBoxFilter_1"的输入参数是"***face_head_box***",输出参数是"***face_head_box_filter_1***", "***face_head_box_filter_2***", 如果仅按PassThrough方式来关闭，会怎样呢？
我们设置mode为PassThrough，加入input data的params_参数(见`best_effort_pass_though_method_main.cc`)。
```C++
 // 4 set through output
  std::cout << "------------ pass through output----------" << std::endl;
  HobotXStream::InputParamPtr
    pass_through(
      new HobotXStream::DisableParam(
        "BBoxFilter_1",
        HobotXStream::DisableParam::Mode::PassThrough));
  inputdata->params_.push_back(pass_through);
  auto out = flow->AsyncPredict(inputdata);
```
接着，我们运行下会有什么效果。
```
============Output Call Back============
—seq: 0
input size: 1
input slot 0 is invalid
—output_type: __NODE_WHOLE_OUTPUT__
—error_code: filter out: -6006
—error_detail_: face_head_box_filter_3 is not ready;face_head_box_filter_2 is not ready;post_box_filter_1 is not ready;
—datas_ size: 0
============Output Call Back End============
```
根据打印显示：
error_code:-6006 代表 HOBOTXSTREAM_ERROR_OUTPUT_NOT_READY(-2002)*3,有3组数据错误码是 HOBOTXSTREAM_ERROR_OUTPUT_NOT_READY
error_detail_: face_head_box_filter_3 is not ready
   face_head_box_filter_2 is not ready
   post_box_filter_1 is not ready
代表预期的输出结果 "***face_head_box_filter_3***", "***face_head_box_filter_2***", "***post_box_filter_1***" 均为not ready。显然，后续节点"BBoxFilter_3"， "PostBoxFilter_4"均未能正常运行。这时workflow如下图:

![BEPassThrough-2](doc/BEPassThrough-2.png)

接下来，我们看下使用Best Effort Pass Through会有怎样的效果。创建一个DisableParam对象给HobotXStream::InputParamPtr共享指针，DisableParam构造函数中设置目标计算节点Node的unique name是"BBoxFilter_1",设置mode为BestEffortPassThrough，加入input data的params_参数。
```C++
  // 5 set best pass through output
  std::cout << "------------best effort pass through output----------" << std::endl;
  inputdata->params_.clear();
  HobotXStream::InputParamPtr
    b_effort_pass_through(
      new HobotXStream::DisableParam(
        "BBoxFilter_1",
        HobotXStream::DisableParam::Mode::BestEffortPassThrough));
  inputdata->params_.push_back(b_effort_pass_through);
  out = flow->AsyncPredict(inputdata);
```
我们运行一下，来看运行后的打印输出,如下：

```bash
============Output Call Back============
—seq: 1
—output_type: __NODE_WHOLE_OUTPUT__
—error_code: -2002
—error_detail_: post_box_filter_1 is not ready;
—datas_ size: 2
——output data face_head_box_filter_3 state:0
——data type:BaseDataVector name:face_head_box_filter_3
pdata size: 0
——output data face_head_box_filter_2 state:4
——data type:BaseData name:face_head_box_filter_2
pdata size: 18446744073709551610
============Output Call Back End============
```

这回`error_code: -2002
error_detail_: post_box_filter_1 is not ready;`
"***post_box_filter_1***" 状态是not ready,
output data 有2组：
"***face_head_box_filter_3***"的状态是**VALID**(0) 
"***face_head_box_filter_2***"的状态是**INVALID**(4)
再来回顾下workflow:
```json
 "workflow": [
    {
      "thread_count": 3,
      "method_type": "BBoxFilter",
      "unique_name": "BBoxFilter_1",
      "inputs": [
        "face_head_box"
      ],
      "outputs": [
        "face_head_box_filter_1",
        "face_head_box_filter_2"
      ],
      "method_config_file": "null"
    },
    {
      "thread_count": 3,
      "method_type": "BBoxFilter",
      "unique_name": "BBoxFilter_3",
      "inputs": [
        "face_head_box_filter_1"
      ],
      "outputs": [
        "face_head_box_filter_3"
      ],
      "method_config_file": "null"
    },
    {
      "thread_count": 3,
      "method_type": "PostBoxFilter",
      "unique_name": "PostBoxFilter_4",
      "inputs": [
        "face_head_box_filter_2"
      ],
      "outputs": [
        "post_box_filter_1"
      ],
      "method_config_file": "null"
    }
  ]
```
在BestEffortPassThrough的控制下的workflow如下图

![BEPassThrough-3](doc/BEPassThrough-3.png)

在BestEffortPassThrough的模式控制下，"BBoxFilter_1"没有运行，但输入参数"***face_head_box***"直接转换成了输出参数"***face_head_box_filter_1***",而"***face_head_box_filter_2***"因没有对应的输入数据,被设置为**INVALID**(4)。
"***face_head_box_filter_1***"在计算节点"BBoxFilter_3"处理下，生成"***face_head_box_filter_3***"，因此最终输出数据中"***face_head_box_filter_3***"的状态是**VALID**(0)。 而"PostBoxFilter_4"节点，因为输入数据 "***face_head_box_filter_2***"被设置为**INVALID**(4),不满足运行要求, 输出数据"***post_box_filter_1***"状态就被设置为了not ready。
这样在"BBoxFilter_1"节点不运行的情况下，通过输入数组的"***face_head_box***"的"Best Effort"转化，依然保证了"BBoxFilter_3"节点的正常运行。

#### demo2
上一个例子中，是输入参数个数少于输出参数个数的情况(见`config/control_workflow_best_effort_use_predefined_2nd.json`)。接下来看下，如果输入参数大于输出参数时，Best Effort Pass Through 模式将输入参数按顺序转化成输出参数，多余部分丢弃。
在上一个demo的workflow基础上,增加输入参数"face_head_box2"
```json
{
  "max_running_count": 10000,
  "inputs": ["face_head_box","face_head_box2"],
  "outputs": ["face_head_box_filter_3", "post_box_filter_1"],
  "workflow": [
    {
      "thread_count": 3,
      "method_type": "BBoxFilter",
      "unique_name": "BBoxFilter_1",
      "inputs": [
        "face_head_box",
        "face_head_box2"
      ],
      "outputs": [
        "face_head_box_filter_1"
      ],
      "method_config_file": "null"
    },
    {
      "thread_count": 3,
      "method_type": "BBoxFilter",
      "unique_name": "BBoxFilter_3",
      "inputs": [
        "face_head_box_filter_1"
      ],
      "outputs": [
        "face_head_box_filter_3"
      ],
      "method_config_file": "null"
    },
    {
      "thread_count": 3,
      "method_type": "PostBoxFilter",
      "unique_name": "PostBoxFilter_4",
      "inputs": [
        "face_head_box2"
      ],
      "outputs": [
        "post_box_filter_1"
      ],
      "method_config_file": "null"
    }
  ]
}
```
在不加DisableParam参数的运行条件下, 打印输出如下，说明未发生错误，各输出参数状态正确。
```
============Output Call Back===============
—seq: 0
—output_type: __NODE_WHOLE_OUTPUT__
—error_code: 0
—error_detail_: 
filter out: —datas_ size: 2
——output data face_head_box_filter_3 state:0
——data type:BaseDataVector name:face_head_box_filter_3
pdata size: 0
——output data post_box_filter_1 state:0
——data type:BaseDataVector name:post_box_filter_1
pdata size: 1
============Output Call Back End============
```
同样的DisableParam, 设置目标计算节点Node的unique name是"BBoxFilter_1", 设置mode为BestEffortPassThrough。(见`best_effort_pass_though_method_main_2.cc`)
```C++
// 5 set best pass through output
  std::cout << "------------best effort pass through output----------" << std::endl;
  inputdata->params_.clear();
  HobotXStream::InputParamPtr
    b_effort_pass_through(
      new HobotXStream::DisableParam(
        "BBoxFilter_1",
        HobotXStream::DisableParam::Mode::BestEffortPassThrough));
  inputdata->params_.push_back(b_effort_pass_through);
  out = flow->AsyncPredict(inputdata);
```
再次运行看下结果如下，没有发生错误。
```
============Output Call Back============
—seq: 2
—output_type: __NODE_WHOLE_OUTPUT__
—error_code: 0
—error_detail_: 
—datas_ size: 2
——output data face_head_box_filter_3 state:0
——data type:BaseDataVector name:face_head_box_filter_3
pdata size: 0
——output data post_box_filter_1 state:0
——data type:BaseDataVector name:post_box_filter_1
pdata size: 1
============Output Call Back End============
```
预期输出结果"***face_head_box_filter_3***","***post_box_filter_1***"都得到正确状态的返回值。在运行过程中, 计算节点*BBoxFilter_1*"在关闭的状态下, "***face_head_box***"被转化成"***face_head_box_filter_1***"。因后续节点的输入数据没有收到影响, 因此没有发生任何错误, 所有其他期待的输出数据都状态正常。
输出印证了workflow运行状况:
![BEPassThrough-4](doc/BEPassThrough-4.png)

### 总结
以上就是控制计算节点运行的4种方式: Invalid, UsePreDefine, PassThrough, BestEffortPassThrough。
* Invalid模式: 直接关闭节点，输出数据是INVALID。依赖其输出数据的后置网络，都会受到影响，无法输出期待的输出数据。
* UsePreDefine模式: 直接定义关闭节点的输出数据。在模拟, 测试等场景可以使用
* PassThrough模式: 直接将关闭节点的输入数据当做输出数据,前提是输入数据和输出相等。
* BestEffortPassThrough: 如果关闭节点的输入数据多于或等于输出数据，则按顺序将输入数据拷贝到输出数据；如果输入数据少于输出数据, 则多余的输出为Invalid的BaseData。BestEffortPassThrough模式是PassThrough模式的替代版本。
