import os
import cv2
import sys
import json
import imageio
import csv
import numpy as np
import argparse
from PIL import Image, ImageDraw, ImageFont


def read_det_res_json(InputFile, AllFrameNumber):
    person_data_dic = {x: [] for x in range(AllFrameNumber)}
    vehicle_data_dic = {x: [] for x in range(AllFrameNumber)}
    with open(InputFile, 'r') as fn:
        root = json.loads(fn.read())
        for i in range(AllFrameNumber):
            person_data = []
            vehicle_data = []
            targets = root["{}".format(i)]
            if targets is None:
                continue
            if "vehicle" in targets:
                for target in targets["vehicle"]:
                    vehicle_data.append(target)
            if "person" in targets:
                for target in targets["person"]:
                    person_data.append(target)
            person_data_dic[i] = person_data
            vehicle_data_dic[i] = vehicle_data
    return person_data_dic, vehicle_data_dic


def load_image_name_list(img_name_list_path):
    img_name_list = []
    with open(img_name_list_path, 'r') as fn:
        for line in fn.readlines():
            line = line.strip()
            img_name_list.append(line)

    return img_name_list


def draw_car_data(data_img, candidate_bboxes, thick=1, color=None):
    for item in candidate_bboxes:
        trkid = item["id"]
        left = item["vehicle_bbox"][0]
        top = item["vehicle_bbox"][1]
        right = item["vehicle_bbox"][2]
        bottom = item["vehicle_bbox"][3]
        pt1 = (int(round(left)), int(round(top)))
        pt2 = (int(round(right)), int(round(bottom)))
        cv2.rectangle(data_img, pt1, pt2, color, thick)
        text = "id:%d" % trkid[0]
        cv2.putText(data_img, text, (int(round(left)), int(round(top) + 20)),
                    cv2.FONT_HERSHEY_PLAIN, 2, color, 2)
        if "vehicle_plate_bbox" in item:
            left = item["vehicle_plate_bbox"][0]
            top = item["vehicle_plate_bbox"][1]
            right = item["vehicle_plate_bbox"][2]
            bottom = item["vehicle_plate_bbox"][3]
            pt1 = (int(round(left)), int(round(top)))
            pt2 = (int(round(right)), int(round(bottom)))
            cv2.rectangle(data_img, pt1, pt2, color, thick)
        if "vehicle_plate_num" in item:
            img = Image.fromarray(cv2.cvtColor(data_img, cv2.COLOR_BGR2RGB))
            draw = ImageDraw.Draw(img)
            # 字体的格式
            fontStyle = ImageFont.truetype('simsun.ttc', 40, encoding="utf-8")
            text = "num:{}".format(item["vehicle_plate_num"])
            draw.text((right, bottom), text, (0, 0, 255), font=fontStyle)
            data_img = cv2.cvtColor(np.asarray(img), cv2.COLOR_RGB2BGR)
    return data_img


def draw_person_data(data_img, candidate_bboxes, thick=1, color=None):
    for items in candidate_bboxes:
        result = {}
        for item in items:
            result.update(item)
        trkid = result["id"]
        l = 0
        t = 0
        if "face" in result:
            left = result["face"][0]
            top = result["face"][1]
            l = left
            t = top
            right = result["face"][2]
            bottom = result["face"][3]
            pt1 = (int(round(left)), int(round(top)))
            pt2 = (int(round(right)), int(round(bottom)))
            cv2.rectangle(data_img, pt1, pt2, color, thick)

        if "body" in result:
            left = result["body"][0]
            top = result["body"][1]
            l = left
            t = top
            right = result["body"][2]
            bottom = result["body"][3]
            pt1 = (int(round(left)), int(round(top)))
            pt2 = (int(round(right)), int(round(bottom)))
            cv2.rectangle(data_img, pt1, pt2, color, thick)

        if "kps" in result:
            for i in range(17):
                x = result["kps"][2*i]
                y = result["kps"][2*i + 1]
                if x != 0 and y != 0:
                    cv2.circle(data_img, (x, y), 1, color, thick)

        if "face" in result or "body" in result:
            text = "id:%d" % trkid
            cv2.putText(data_img, text,
                        (int(round(left)), int(round(top) + 20)),
                        cv2.FONT_HERSHEY_PLAIN, 2, color, 2)
    return data_img


def parse(ImgNameList, VideoSavePath, InputFiles, AllFrameNumber):
    person_data, vehicle_data = read_det_res_json(InputFiles, AllFrameNumber)
    ImageNameList = load_image_name_list(ImgNameList)

    if VideoSavePath.strip() != '':
        writer = imageio.get_writer(VideoSavePath, fps=25)

    ImgNumber = AllFrameNumber
    for i in range(ImgNumber):
        ImgDraw = cv2.imread(ImageNameList[i], cv2.IMREAD_COLOR)
        print("frame %d" % i)
        # if i > 1000:
        #     break
        if person_data[i] is not []:
            ImgDraw = draw_person_data(ImgDraw, person_data[i],
                                       thick=2, color=(255, 0, 0))

        if vehicle_data[i] is not []:
            ImgDraw = draw_car_data(ImgDraw, vehicle_data[i],
                                    thick=2, color=(255, 0, 0))

        if VideoSavePath.strip() != '':
            writer.append_data(ImgDraw[:, :, ::-1])

    if VideoSavePath.strip() != '':
        writer.close()


def help():
    print("Format: python ScriptFile ImgNameList VideoSavePath" +
          "EmptyFrameNumber AllFrameNumber InputFiles...")
    sys.exit(0)


if __name__ == '__main__':
    EmptyFrameNumber = 0
    AllFrameNumber = 0
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--image_list",
        type=str,
        required=True,
        help="image list for render",
    )
    parser.add_argument(
        "--video_path",
        type=str,
        required=True,
        help="render video save path",
    )
    parser.add_argument(
        "--log_file",
        type=str,
        required=True,
        help="log file to revender",
    )

    args = parser.parse_args()

    ImgNameList = args.image_list
    VideoSavePath = args.video_path
    InputFiles = args.log_file
    # print("Image name list path: {}".format(ImgNameList))
    # print("Video save path: {}".format(VideoSavePath))
    # print("Log files: {}".format(InputFiles))

    if os.path.exists(VideoSavePath):
        os.remove(VideoSavePath)
    AllFrameNumber = len(open(args.image_list, 'r').readlines())
    print("Image nums: {}".format(AllFrameNumber))
    parse(ImgNameList, VideoSavePath, InputFiles, AllFrameNumber)
