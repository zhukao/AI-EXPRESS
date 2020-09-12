## 活体检测小工具


#### `seg_video_by_time.py`
将整段视频按照既定的时间段单独切分成独立的视频。主要是将含有多个ID的视频分为每个ID一个小段的视频。

#### `extract_frames.py`
抽帧工具。可以按照想要的帧率进行抽帧，同时可以检查当前视频是否已经完成抽帧，如果完成，则直接跳过。

#### `select_face.py`
人脸优选代码。

#### `select_images.py`
根据帧数来选择图片，主要用于数据量非常大时减小数据量。

#### `gen_label.py`
根据文件路径，将每张图片打上活体与假体的label。

#### `class_statistic.py`
查看list中各个类别的数据分别有多少。

#### `cp_images.py`
将抽帧出来的图像单独复制到一个目录下。原因：抽帧工具抽完的图片命名都是`00000000.jpg`，全部放到一个文件夹下时，会重名，所以有此工具。

#### `trans_img_to_1080.py`
将人脸图贴到一张1080的图上并保存。

#### `trans_rgb_to_yuv444_or_gray.py`
将图像转换成yuv444或者gray，方便工程进行回灌测试。
给工程回灌测试的流程是:
- 用`cp_images.py`将图片复制出来。
- 用`trans_img_to_1080.py`将图像转换成1080p。
- 用`trans_rgb_to_yuv444_or_gray.py`将1080p图像转换成工程回灌所用的图。

#### `get_image_size.py`
不需要读图片即可得到图像的大小。

#### `parse_x1_results.py`
工程端（鹏飞给的yuv每张图的框）输出的框解析出来对应到每张图像上。

#### `plot_rects_anti_spoofing.py`
画活体检测的人脸框，并保存图片。

#### `select_somethion.py`
自定义一个规则，将list中符合要求的留下。

#### `split_data.py`
弃用。

#### `split_data2.py`
弃用。

