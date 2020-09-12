##########################################################################
# File Name: render.sh
# Author: chengfei
# mail: fei.cheng@horizon.ai
# Created Time: 2020年03月15日 星期日 15时19分12秒
#########################################################################
#!/bin/sh
python video_render.py --image_list=car_render.list --video_path=./car_render.mp4 --log_file=smart_data_car.json
