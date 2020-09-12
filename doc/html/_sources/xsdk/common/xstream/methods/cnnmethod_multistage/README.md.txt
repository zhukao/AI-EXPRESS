# cnnmethod_multistage

## 背景
CNNMethod主要用于人脸关键点，人脸朝向，人脸识别，活体检测等，通过前处理和后处理的组合，输出上层业务逻辑的需求。CNNMethod_multistage将CNN前后处理拆分成两个独立的method，前处理的主要目的是对输入的数据预处理并送进BPU中得到CNN网络的输出结果，即预处理+预测过程；后处理针对模型不同，将CNN网络输出结果封装为业务数据。大部分情况下前处理是可以复用的，将CNNMethod拆分为前处理和后处理两个Method，开发者可以针对不同的模型自行开发对应的后处理程序使用。
因此在xstream中执行CNN模型的相关workflow如下：
![cnnmethod_workflow](./doc/images/cnn.png)

目前程序已实现的前处理和后处理method包括下列几种：
* 前处理：CNNLmkPreMethod、CNNImgPreMethod、CNNRectPreMethod
* 后处理：CNNLmkposePostMethod、CNNAgegenderPostMethod、CNNAntispfPostMethod、CNNQualityPostMethod、CNNFeaturePostMethod

## 前处理method

### CNNLmkPreMethod（lmk+抓拍）

CNNLmkPreMethod把输入中的每张人脸图片，利用关键点信息完成仿射变换之后送给BPU预测，要注意该BPU接口要求生成的hbm模型对应的输入类型是"-i pyramid"。

该前处理的数据流如下表所示：

输入：

| slot | 内容      | 备注                                         |
| ---- | --------- | -------------------------------------------- |
| 0    | snap_list | 抓拍图像以及人脸关键点，类型为BaseDataVector |

输出：

| slot | 内容     | 备注                                                                              |
| ---- | -------- | --------------------------------------------------------------------------------- |
| 0    | run_data | 当前模型输出结果以及模型信息，类型为CNNMethod的内置数据结构CNNPredictorOutputData |

CNNLmkPreMethod的输入数据封装，具体可参考example/do_fb_feature.cpp。

### CNNRectPreMethod（roi+img）

CNNRectPreMethod将输入的一组矩形框resize到模型输入大小，然后利用BPU单元完成预测过程；需要注意的是该前处理过程resize变换是利用BPU上硬件pymraid+resizer单元实现，与软件（如opencv、libyuv）scale过程有差异，硬件单元处理性能较高，但同时会给算法数值一致性验证带来困扰。该method要求生成的hbm模型对应的输入类型是"-i resizer"

该前处理的数据流如下所示：

输入：

| slot | 内容     | 备注               |
| ---- | -------- | ------------------ |
| 0    | face_box | 需要做预测的检测框 |
| 1    | pyramid  | 图像的金字塔数据   |

输出：

| slot | 内容     | 备注                         |
| ---- | -------- | ---------------------------- |
| 0    | run_data | 当前模型输出结果以及模型信息 |

其中输入数据"face_box"的数据结构类型是BaseDataVector，"face_box"内包括BBox类型的roi；输入数据"pyramid"的数据结构类型是ImageFrame，包含待处理的PymImageFrame类型图像；输出数据"run_data"的数据结构类型是CNNMethod的内置数据结构CNNPredictorOutputData，包括CNN模型nhwc信息，以及模型输出结果mxnet_output等，以输出到后处理中封装为对应的业务数据。CNNRectPreMethod前处理的输入数据封装，具体可参考example/do_fb_rect_cnn.cpp。

### CNNImgPreMethod（roi+img）

该前处理的输入数据与CNNRectPreMethod相同，与CNNRectInputMethod的区别主要是检测框到模型输入大小的resize变换是通过软件实现的（调用第三方开源库opencv或者libyuv），软件过程虽然性能会下降，但是优点在于可以与算法训练的resize变换保持一致，从而保证了算法数值一致性验证。该Method要求生成的hbm模型对应的输入类型是"-i pyramid"。

**注：前处理method包括预测CNN网络输出结果的过程，调用的BPU接口与生成模型的方式相关联，配置的前处理方式应与生成模型匹配。**

## 后处理method

后处理将模型输出的nhwc维度的数据封装为模型对应的业务数据。

### CNNFeaturePostMethod

该后处理的数据流如下所示：

输入：

| slot | 内容     | 备注                           |
| ---- | -------- | ------------------------------ |
| 0    | run_data | 当前模型的输出结果以及模型信息 |

输出：

| slot | 内容         | 备注       |
| ---- | ------------ | ---------- |
| 0    | face_feature | 人脸特征值 |

该后处理的输入是前处理Method的输出数据结构，输出数据"face_feature"的类型是BaseDataVector，定义为"vector"的原因是前处理输入为snap_list，其中包含多个跟踪目标的抠图信息，且每个跟踪目标也可能包含多张抠图。BaseDataVector内包含的基本数据结构是hobot::vision::Feature（定义于vision_type），输出人脸特征向量。

**对输出人脸特征数据进行处理的sample可参考example/do_fb_feature.cpp中的函数PrintFaceFeature。**

### CNNLmkposePostMethod

该后处理的数据流如下图所示：

输入：

| slot | 内容     | 备注                           |
| ---- | -------- | ------------------------------ |
| 0    | run_data | 当前模型的输出结果以及模型信息 |

输出：

| slot | 内容 | 备注       |
| ---- | ---- | ---------- |
| 0    | lmk  | 人脸关键点 |
| 1    | pose | 人脸朝向   |

该后处理Method输出lmk和pose信息。由于单张图像可能存在多个人脸，输出数据"lmk"和"pose"的基本数据类型是BaseDataVector，"lmk"内包含的数据结构结构是hobot::vision::Landmarks，"pose"内的数据结构是hobot::vision::Pose3D，都定义于vision_type。

**对输出人脸关键点以及人脸朝向数据进行处理的sample可参考example/do_fb_rect_cnn.cpp中的函数DumpPoseLmk。**

### CNNQualityPostMethod
该后处理的数据流如下所示：

输入：

| slot | 内容     | 备注                           |
| ---- | -------- | ------------------------------ |
| 0    | run_data | 当前模型的输出结果以及模型信息 |

输出：

| slot | 内容              | 备注           |
| ---- | ----------------- | -------------- |
| 0    | Blur              | 清晰度         |
| 1    | Brightness        | 亮度           |
| 2    | Eye_Abnormalities | 眼睛表情       |
| 3    | Mouth_Abnormal    | 嘴部表情       |
| 4    | Left_Eye          | 左眼可见区域   |
| 5    | Right_Eye         | 右眼可见区域   |
| 6    | Left_Brow         | 左眉毛可见区域 |
| 7    | Right_Brow        | 右眉毛可见区域 |
| 8    | ForeHead          | 额头可见区域   |
| 9    | Left_Cheek        | 左脸颊可见区域 |
| 10   | Right_Cheek       | 右脸颊可见区域 |
| 11   | Nose              | 鼻子可见区域   |
| 12   | Mouth             | 嘴部可见区域   |
| 13   | Jaw               | 下巴可见区域   |

该method为人脸质量模型后处理过程，通过人脸质量模型可以预测人脸清晰度等质量信息，从而可以有效地过滤一些模糊、遮挡以及一些大角度的人脸图像。该后处理Method输出人脸质量信息，包括"blur"、"brightness"、"eye_abnormalities"、"mouth_abnormal"、"left_eye"、"right_eye"、"left_brow"、"right_brow"、"forehead"、"left_cheek"、"right_check"、"nose"、"mouse"、"jaw"14个维度的质量信息。输出的数据结构都是BaseDataVector，内部包含的基本数据类型都是hobot::vision::Attribute，定义于vision_type。

**对输出人脸质量进行处理的sample可参考example/do_fb_rect_cnn.cpp中的函数DumpFaceQuality。**

### CNNAgegenderPostMethod

该后处理的数据流如下所示：

输入：

| slot | 内容     | 备注                           |
| ---- | -------- | ------------------------------ |
| 0    | run_data | 当前模型的输出结果以及模型信息 |

输出：

| slot | 内容   | 备注 |
| ---- | ------ | ---- |
| 0    | Age    | 年龄 |
| 1    | Gender | 性别 |

该后处理输出年龄"age"、性别"gender"信息，输出的数据结构都是BaseDataVector，内部包含的基本数据类型分别是hobot::vision::Age，hobot::vision::Gender，定义于vision_type。

**对输出年龄性别进行处理的sample可参考example/do_fb_rect_cnn.cpp中的函数DumpAgeGender。**

### CNNAntispfPostMethod
该后处理的数据流如下所示：

输入：

| slot | 内容     | 备注                           |
| ---- | -------- | ------------------------------ |
| 0    | run_data | 当前模型的输出结果以及模型信息 |

输出：

| slot | 内容     | 备注                     |
| ---- | -------- | ------------------------ |
| 0    | anti_spf | 活体值，包括value和score |

该后处理输出活体检测信息，输出的数据结构是BaseDataVector，内部包含的基本数据类型是hobot::vision::Attribute，定义于vision_type。

**对输出活体信息进行处理的sample可参考example/do_fb_rect_cnn.cpp中的函数DumpAntiSpf。**

## json sample文件

### Rect前处理+Lmkpose后处理 RectPre_LmkposePost

```
{
  "inputs": [
    "face_box",
    "pyramid"
  ],
  "outputs": [
    "lmk",
    "pose",
    "pyramid",
    "face_box"
  ],
  "workflow": [
    {
      "unique_name": "rect_input",
      "method_type": "CNNRectPreMethod",
      "inputs": [
        "face_box",
        "pyramid"
      ],
      "outputs": [
        "cnn_run_data"
      ],
      "method_config_file": "method_conf/pose_lmk.json"
    },
	{
      "unique_name": "lmkpose_post",
      "method_type": "CNNLmkposePostMethod",
      "inputs": [
        "cnn_run_data"
      ],
      "outputs": [
        "lmk",
        "pose"
      ],
      "method_config_file": "method_conf/pose_lmk.json"
    }
  ]
}
```

### Lmk前处理+Feature后处理 LmkPre_FeaturePost

```
{
  "inputs": [
    "snap_list"
  ],
  "outputs": [
    "face_feature"
  ],
  "workflow": [
    {                                  
      "unique_name": "lmk_pre",
      "method_type": "CNNLmkPreMethod",
      "inputs": [  
        "snap_list"
      ],          
      "outputs": [    
        "cnn_run_data"
      ],                                              
      "method_config_file": "method_conf/feature.json"
    },
    {
      "unique_name": "feature_post",
      "method_type": "CNNFeaturePostMethod",
      "inputs": [
        "cnn_run_data"
      ],
      "outputs": [
        "face_feature"
      ],
      "method_config_file": "method_conf/feature.json"
    }
  ]
}
```

### Img前处理+Quality后处理 ImgPre_QualityPost

```
{
  "inputs": [
    "face_box",
    "pyramid"
  ],
  "outputs": [
    "face_box",
    "blur",
    "brightness",
    "eye_abnormalities",
    "mouth_abnormal",
    "left_eye",
    "right_eye",
    "left_brow",
    "right_brow",
    "forehead",
    "left_cheek",
    "right_check",
    "nose",
    "mouse",
    "jaw"
  ],
  "workflow": [
    {
      "unique_name": "img_pre",
      "method_type": "CNNImgPreMethod",
      "inputs": [
        "face_box",
        "pyramid"
      ],
      "outputs": [
        "cnn_run_data"
      ],
      "method_config_file": "method_conf/face_quality.json"
    },
    {
      "unique_name": "quality_post",
      "method_type": "CNNQualityPostMethod",
      "inputs": [
        "cnn_run_data"
      ],
      "outputs": [
        "blur",
        "brightness",
        "eye_abnormalities",
        "mouth_abnormal",
        "left_eye",
        "right_eye",
        "left_brow",
        "right_brow",
        "forehead",
        "left_cheek",
        "right_check",
        "nose",
        "mouse",
        "jaw"
      ],
      "method_config_file": "method_conf/face_quality.json"
    } 
  ]
}
```

### Rect前处理+Agegender后处理 RectPre_AgegenderPost

```
{
  "inputs": [
    "face_box",
    "pyramid"
  ],
  "outputs": [
    "age",
    "gender",
    "pyramid",
    "face_box"
  ],
  "workflow": [
    {
      "unique_name": "rect_pre",
      "method_type": "CNNRectPreMethod",
      "inputs": [
        "face_box",
        "pyramid"
      ],
      "outputs": [
        "cnn_run_data"
      ],
      "method_config_file": "method_conf/age_gender.json"
    },
	{
      "unique_name": "agegender_post",
      "method_type": "CNNAgegenderPostMethod",
      "inputs": [
        "cnn_run_data"
      ],
      "outputs": [
        "age",
        "gender"
      ],
      "method_config_file": "method_conf/age_gender.json"
    }
  ]
}
```

### Rect前处理+Antispf后处理 RectPre_AntispfPost

```
{
  "inputs": [
    "face_box",
    "pyramid"
  ],
  "outputs": [
    "cnn_anti_spf_bgr"
  ],
  "workflow": [
    {
      "unique_name": "rect_pre",
      "method_type": "CNNRectPreMethod",
      "inputs": [
        "face_box",
        "pyramid"
      ],
      "outputs": [
        "cnn_run_data"
      ],
      "method_config_file": "method_conf/anti_spf.json"
    },
    {
      "unique_name": "antispf_post",
      "method_type": "CNNAntispfPostMethod",
      "inputs": [
        "cnn_run_data"
      ],
      "outputs": [
        "cnn_anti_spf_bgr",
        "norm_rois"
      ],
      "method_config_file": "method_conf/anti_spf.json"
    }
  ]
}
```