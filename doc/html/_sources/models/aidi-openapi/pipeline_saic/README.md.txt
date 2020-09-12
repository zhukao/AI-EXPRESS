# 艾迪平台使用流程介绍

## 流程概览

  ![完整流程](images/pipeline.png)

## 操作步骤

具体的操作步骤在`pipeline.sh`脚本中，下面对每一个步骤进行说明
建议后续脚本操作都在`pipeline_saic`目录下进行

软件环境要求

- python3.6
- tensorflow-gpu 1.14
- hitc v1.0.47 or above
- traincli 3.1.0 or above

### Step1 环境配置

- 配置个人token

  登录艾迪平台`aidi.hobot.cc`后，在`用户中心/个人中心`复制token

  ![拷贝token](images/copy_token.png)

  复制之后，通过`hitc init -t your_token -C ./hitc_config.yaml`和`traincli init -t your_token -c traincli_config.yaml`对艾迪平台账户token初始化

  ![修改hitc配置](images/set_token_hitc.png)

  ![修改traincli配置](images/set_token_traincli.png)

- 配置GPU集群

  为了向GPU集群提交训练任务，需要申请集群的appid和appkey
  登录艾迪平台之后，在`用户中心/个人中心`的集群配置模块中点击`创建APP KEY`，选择`share-2080ti`队列点击提交进行申请，申请成功后将appid和appkey填入`traincli_config.yaml`文件中的`clusters.mycluster1.appid`和`clusters.mycluster1.appkey`

  ![申请APPKey](images/apply_appkey.png)

  ![修改traincli配置](images/set_appkey.png)

注: 后续艾迪平台会将命令行工具和相应配置文件进行整合

### Step2 数据准备

  注: 如果直接使用艾迪平台已经上传好的数据集，跳过`数据准备`步骤。
  其中，图片数据集名称为`vehicle_rear_val_dataset`，打包数据集中训练集名称为`vehicle_rear_train_dataset_tf`，测试集名称为`vehicle_rear_val_dataset_tf`

  ![图片数据集](images/pic_dataset.png)

  ![打包数据集](images/pack_dataset.png)

- 原始数据下载

  将`adas mini`数据集从HDFS下载到本地并解压

  ```shell
  hdfs dfs -get hdfs://hobot-bigdata/user/yihang.xin/saic_demo/vehicle_rear/train.tar
  hdfs dfs -get hdfs://hobot-bigdata/user/yihang.xin/saic_demo/vehicle_rear/train.json
  hdfs dfs -get hdfs://hobot-bigdata/user/yihang.xin/saic_demo/vehicle_rear/val.tar
  hdfs dfs -get hdfs://hobot-bigdata/user/yihang.xin/saic_demo/vehicle_rear/val.json
  tar -xvf train.tar
  tar -xvf val.tar
  ```

  ![原始数据下载](images/download_dataset.png)

- 图片数据集上传

  上传测试数据集中的图片和标注数据文件到艾迪平台
  由于该步骤比较耗时，建议操作之后另开shell进行后续流程操作

  ```shell
  hitc dataspace upload -n your_data_set_name -t dataset -f ./val -C ./hitc_config.yaml
  hitc dataspace upload -d your_data_set_name -t gt -f ./val.json -C ./hitc_config.yaml
  ```

  ![图片数据集上传](images/dataset_upload.png)

  ![标注数据上传](images/gt_upload.png)

  ![查看上传结果](images/view_dataset.png)

  注: `your_data_set_name`为数据集名称，需要改成您上传的数据集名称；上传成功之后在艾迪平台`数据管理/图片数据集`中可以看到资源详情，包括标注数据ID、图片数量等

- 数据打包

  将训练集和测试集中的图片和标注数据打包为`tfrecord`格式，生成`train.tfrecord`和`val.tfrecord`文件，作为后续训练和预测的输入

  ```shell
  pip install tensorflow-gpu==1.14 -i https://pypi.tuna.tsinghua.edu.cn/simple --user
  PYTHONPATH=./1hostjob:$PYTHONPATH python3 1hostjob/pack/im2tfrec.py --data-dir ./ --split-name train --num-worker 20
  PYTHONPATH=./1hostjob:$PYTHONPATH python3 1hostjob/pack/im2tfrec.py --data-dir ./ --split-name val --num-worker 20
  ```

  ![训练集打包](images/pack_traindataset.png)

  ![测试集打包](images/pack_testdataset.png)

- 打包数据集上传

  ```shell
  hitc dataspace upload -n your_data_set_name -t rec -f ./train.tfrecord -C ./hitc_config.yaml
  hitc dataspace upload -n your_data_set_name -t rec -f ./val.tfrecord -C ./hitc_config.yaml
  ```

  ![训练打包数据集上传](images/train_rec_upload.png)

  ![测试打包数据集上传](images/test_rec_upload.png)

  ![训练打包数据集查看](images/view_train_rec.png)

  ![测试打包数据集查看](images/view_test_rec.png)

  注: `your_data_set_name`为数据集名称，需要改成您上传的数据集名称；上传成功之后在艾迪平台`数据管理/打包数据集`中可以看到资源详情

### Step3 发起训练

  发起训练时不仅需要将上传的打包数据集挂载到容器中，而且通过训练配置文件获取训练集、测试集以及模型的路径

- 配置数据挂载

  在艾迪平台`数据管理/打包数据集`页面中，即可看到数据集的名称，编辑`1hostjob.yaml`文件，将页面中的打包数据集的名称写入`OPTIONAL.DATA_SPACE.INPUT.NAME`中，用来将打包数据集挂载到容器中

  ![打包数据集](images/rec_name.png)

  ![训练任务配置](images/trainjob_config.png)

- 设置训练配置
  - 训练集/测试集的配置
    修改`1hostjob/adas/configs/mobilenetv1.py`文件中的`config.train_param.rec_path`和`config.val_param.rec_path`
    训练数据集`config.train_param.rec_path`的路径为`/data_space/input/name/type/${version}| latest/train.tfrecord`，(可以指定具体版本或者latest，latest指向最新的版本。  example: /data_space/input/name/type/latest/train.tfrecord  或者 /data_space/input/name/type/v0.0.1/train.tfrecord)。测试数据集`config.val_param.rec_path`的路径为`/data_space/input/name/type/${version}|latest/val.tfrecord`,其中`name`为打包数据集的名称，type为打包数据集的类型，一般都是rec。

  ![训练代码读取数据配置](images/train_recpath_config.png)

  - 保存模型的路径配置
    修改`1hostjob/adas/configs/mobilenetv1.py`文件中的`config.save_model_dir`，用来指定训练过程中保存模型的路径，一般配置为用户的HDFS目录下的任意目录
    用户的HDFS目录为`hdfs://hobot-bigdata/user/name/your_model_path/`，其中`name`为用户名称，`your_model_path`为任意目录

    注: 必须改为您的HDFS地址，否则在训练过程中无法保存模型（HDFS地址的目录可以包含多层，不需要提前创建，训练过程中会自动创建）

  ![训练代码模型保存配置](images/train_savemodeldir_config.png)

  - 使用快速模式
    由于训练过程往往比较耗时(8卡下约90小时)，如果想要快速(8卡下约6小时)执行一个训练任务，需要执行该步骤，否则跳过
    - 基于已运行的`with_bn`和`convert_with_bn`阶段的模型结果进行finetune，只需要运行`without_bn`和`convert_without_bn`
      编辑`1hostjob/job.sh`文件，注释掉运行`with_bn`的命令

    ![训练代码使用快速模式](images/train_fast1.png)

    - 指定`without_bn`阶段开始时加载的模型路径
      编辑`1hostjob/adas/configs/mobilenetv1.py`文件，将`config.without_bn_stage.load_weight_path`参数改为`hdfs://hobot-bigdata/user/v-shuaishuai.wang/saic_demo/model/2/convert_with_bn/withoutbn.weights`

    ![训练代码使用快速模式](images/train_fast2.png)

    - 调整`without_bn`和`convert_without_bn`运行的Epoch数
      编辑`1hostjob/adas/configs/mobilenetv1.py`文件，将`config.without_bn_stage.epoch`参数设置为30，将`config.without_bn_stage.lr_decay_step`参数设置为`[20]`

    ![训练代码使用快速模式](images/train_fast3.png)

- 提交训练任务

  ```shell
  traincli submit -f 1hostjob.yaml -c traincli_config.yaml
  ```

  ![提交训练任务](images/submit_trainjob.png)

  ![查看训练任务](images/view_trainjob.png)

  提交成功后，在艾迪平台`任务管理/训练任务`的`运行任务`中标签栏中可以看到创建的训练任务，点击进入之后可以看到训练任务的详细信息

### Step4 发起预测

- 下载模型到本地

  ```shell
  hdfs dfs -get hdfs://hobot-bigdata/user/name/your_model_path/convert_without_bn
  ```

  注：必须改为您之前设置的的模型保存地址，否则无法下载模型到本地。您可以在`1hostjob/adas/configs/mobilenetv1.py`文件中找到`config.save_model_dir`字段，并使用这个字段的内容替换以上命令中的`hdfs://hobot-bigdata/user/name/your_model_path/`这一部分

  ![找到模型地址](images/train_savemodeldir_config.png)

  ![下载模型到本地](images/download_model.png)

  仅仅下载`convert_without_bn`阶段的模型

- 发起预测

  基于下载的`convert_without_bn`阶段的模型，对测试集进行预测

  ```shell
  PYTHONPATH=./1hostjob:$PYTHONPATH python3 1hostjob/adas/inference.py --rec_path ./val.tfrecord --config 1hostjob/adas/configs/mobilenetv1.py --step inference --load-param convert_without_bn/int_inference.weights
  ```

  ![发起预测](images/run_predict.png)

  注: 后续艾迪平台会支持将预测任务提交到GPU集群运行

- 预测结果保存

  预测结果为`val_pr.json`，将该文件上传到艾迪平台且与测试图片数据集绑定

  ```shell
  hitc dataspace upload -d your_data_set_name -t pr -f ./val_pr.json -C ./hitc_config.yaml
  ```

  ![预测结果上传](images/pr_upload.png)

  ![预测结果查看](images/view_pr.png)

  注: `your_data_set_name`为与预测结果绑定的图片数据集名称。上传成功后在`数据管理/图片数据集`详情页中的`预测结果`标签栏下看到预测数据ID

### Step5 发起评测

  将测试集的图片、标注文件、预测结果结合到一起，评测模型的精度指标，且在测试集图片上观察模型效果

- 查看评测策略和评测参数

  在艾迪平台的`评测管理/算法任务类型/detection`下，查看到所有可用的检测任务评测策略。点击`saic_detection`，查看到所有可用的评测参数配置文件，支持查看、修改、另存为等功能

  ![查看评测脚本](images/view_profile.png)

- 上传自定义的评测策略和参数（可选）

  艾迪平台支持进行自定义的评测策略和参数，用户能够自行决定评测的难度、侧重点，以及指标的计算方式和可视化的具体内容

  本次评测任务，可以直接使用`saic_detection`评测策略和默认参数

- 发起评测任务

  编辑评测任务配置文件`eval_task.yaml`，除了`algo_task_type_id`和`send_email`参数外，其他项根据实际任务进行内容的修改

  ```shell
  hitc evaltask submit -f eval_task.yaml -C ./hitc_config.yaml
  ```

  ![修改评测参数](images/evaltask_config.png)

  ![发起评测任务](images/submit_evaltask.png)

- 查看任务运行结果

  评测任务任务提交后，在艾迪平台的`评测管理/评测任务/单评测任务`下查看到创建的评测任务，鼠标悬浮到任务的右侧`Logs`位置，可以查看到当前任务的运行状态和log链接，通过log链接可查看任务执行信息

  当任务运行状态显示`success`（请刷新页面获取最新状态），点击ID或者任务名称跳转到进任务详情页面，展示了任务的核心数值指标、数值信息统计表、预测效果和评测结果可视化等内容，允许用户从宏观和微观两个角度观察本次预测的精度特征

  ![查看评测任务](images/view_evaltask.png)

  核心指标和数值统计中，展示了 `ap(average precision)`、`rec(recall)`、`prec(precision)`、`tp/fp/fn(命中/误检/漏检)目标数统计` 信息供用户参考

  可视化内容中将每张图中的 `标注（蓝色）`、`命中（绿色）`、`误检（红色）`、`漏检（黄色）`在原图中标注了出来，可以通过 `fp_num 误检数量`、`fn_num 漏检数量`、`fp_max_score 误检最高置信度` 等指标对图片进行排序，通过 `最小置信度阈值`、`仅展示badcase` 按钮对展示出的图片和目标进行筛选，从而精确的看到当前模型在测试集上所擅长以及欠缺的能力

  ![查看评测结果](images/view_result1.png)

  ![查看评测结果](images/view_result2.png)
  
### Step6 保存模型

  当模型评测的指标符合预期后，可以将模型进行保存

- 创建模型目录
  在艾迪平台`模型管理/实验模型`页面，点击`创建实验模型目录`按钮，进行模型目录的创建
- 模型添加版本
  修改保存模型的配置文件`model_append.yaml`，各个配置参数参见注释
  执行`hitc model append`命令后，模型文件会被上传到艾迪平台，由于该模型文件比较大，需要等待一段时间后在艾迪平台上才能看到模型的详细信息，比如模型文件名称、MD5值等

  ```shell
  hitc model append -f ./model_append.yaml -C ./hitc_config.yaml
  ```

  ![保存模型](images/upload_model.png)

  ![查看模型信息](images/view_model.png)

### Step7 模型编译

- 发起模型编译
  基于上传的实验模型，发起编译任务。在实验模型版本列表详情页面，选择编译按钮进入编译任务设置页面
  填写任务名称、版本等信息（其中HBDK版本选择`v3.7.8`，任务模式选择`自定义`，模型管理中模型名称填写为`ssd`，模型编译参数选择`vehicle_rear_compile_param`），点击下方提交按钮

  ![进入模型编译](images/enter_model_compile.png)

  ![提交编译任务](images/submit_model_compile.png)

- 下载编译结果
  提交编译任务后，在实验模型版本详情页面可以查看到该实验模型关联的编译任务。在编译任务详情页面中，点击Log按钮查看编译过程日志
  编译成功结果后，点击下载按钮下载编译结果

  ![查看编译结果](images/view_compile.png)

  ![下载编译结果](images/download_compile.png)
