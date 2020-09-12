![GluonFace](./logo.png)

A Face toolkit based on MXNet/Gluon.

You can find more information in [GluonFace wiki](http://wiki.hobot.cc/display/AlgorithmRD/GluonFace).


# Introduction

GluonFace contain a lot of **face related tasks** implemented with [Gluon](https://mxnet.apache.org/api/python/docs/api/gluon/index.html) and [GluonHorizon](http://gitlab.hobot.cc/ptd/algorithm/ai-platform-algorithm/GluonHorizon), such as **face landmark, pose, attribute(gender, age, glass, mask, race, national, helmet), anti_spoofing, recognition, 3D face reconstruction** and so on.
And some multi_tasks, such as the combination of landmark and pose, gender and age.
More information about these tasks can be found in [GluonFace task introduction](http://wiki.hobot.cc/pages/viewpage.action?pageId=83336379).


# Request List

   - Python 2.7 or Python 3.6+, recommend Python 3.6 + for Python 2.7 has some bugs in multiprocessing. You can use [Anaconda](https://www.anaconda.com/) to manage your development environment.
     -  Note: opencv-python == 3.4.5.20
   - MXNet/Gluon, recommend Horizon MXNet with **nnvm** branch.
   - [GluonHorizon](http://gitlab.hobot.cc/ptd/algorithm/ai-platform-algorithm/GluonHorizon) for quantization training.
   - [BytePS](https://github.com/bytedance/byteps) (optional), if use BytePS instead of KVStore.
     -  Set `config.train.dist_type = 'byteps'`.


# How to train?

## Train on dev
```
python scripts/{single_task}_or_{multi_tasks}/xxx/train.py --cfg scripts/{single_task}_or_{multi_tasks}/xxx/configs/xxx.yaml
```

## Train on cluster
```
python gluon_face/common/train_on_cluster.py --cfg scripts/{single_task}_or_{multi_tasks}/xxx/configs/xxx.yaml
```
#### Note
`job_list: []` in ymal file is the specific command executed on the cluster.


# How to develop with GluonFace?

You can find specifications in [Development Specification](http://wiki.hobot.cc/pages/viewpage.action?pageId=83336410).
And And please follow the corresponding [code](http://wiki.hobot.cc/pages/viewpage.action?pageId=83336409) & [interface](http://wiki.hobot.cc/pages/viewpage.action?pageId=83336408) specifications.


# Docs
```
cd docs & & make html & & cd ..
```


# Concat

This repo is currently maintained by[zijian.zhou](zijian.zhou@horizon.ai). If you have any questions, please contact me.
