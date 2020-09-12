import sys
import os
sys.setdlopenflags(os.RTLD_LAZY)
import xstream.xstream    # noqa

node1 = xstream.Method("Node_1").inputs(["data1"]).outputs(
    ["data1", "data2", "data3", "data4"])
node1.config_file("Node1_cfg.json").thread_count(2)
node2 = xstream.Method("Node_2").inputs(
    ["data1", "data2"]).outputs(["data1", "data2", "data3"])
node3 = xstream.Method("Node_3").inputs(
    ["data3", "data4"]).outputs(["data1", "data2", "data3"])

node5 = xstream.Method("Node_5").inputs(
    ["data1", "data2"]).outputs(["data1", "data2", "data3"])
node6 = xstream.Method("Node_6").inputs(
    ["data1", "data3"]).outputs(["data1", "data2", "data3"])
node7 = xstream.Method("Node_7").inputs(
    ["data1", "data3"]).outputs(["data1", "data2", "data3"])
node8 = xstream.Method("Node_8").inputs(
    ["data1", "data2", "data2", "data3"]).outputs(["data1", "data2", "data3"])
node9 = xstream.Method("Node_9").inputs(
    ["data1", "data2"]).outputs(["data1", "data2", "data3"])

cnn_method = xstream.Method("CNNMethod").inputs(["image"])

# 创建workflow


def sub_workflow(data1):
    data1_1, data2_1, data3_1, data4_1 = node1(data1)

    with xstream.scope("pre1"):
        # node1的下一级
        data1_2, data2_2, data3_2 = node2(data1_1, data2_1)
        data1_3, data2_3, data3_3 = node3(data3_1, data4_1)

        with xstream.scope("pre2"):
            # node2的下一级
            data1_5, data2_5, data3_5 = node5(data1_2, data2_2)
            data1_6, data2_6, data3_6 = node6(data1_2, data3_2)

            # node3的下一级
            data1_7, data2_7, data3_7 = node7(
                data1_3, data3_3,
                Config={
                    "a": 1,
                    "b": 2
                },
                ThreadList=[1, 2, 3, 4]
            )

        # node5 node6的下一级
        data1_8, data2_8, data3_8 = node8(data1_5, data2_5, data2_6, data3_6)

    # node7的下一级
    data1_9, data2_9, data3_9 = node9(data1_7, data2_7)

    # 返回当前工作流的结果
    return data1_8, data2_8, data1_6, data1_9, data2_9


def my_workflow(image):
    lmk = cnn_method(image, outputs=["data1"], config_file="cnn_cfg.json")
    out1, out2, out3, out4, out5 = sub_workflow(lmk)
    eyes = cnn_method(out2, out3, inputs=["data2", "data1"], outputs=[
                      "eyes"], config_file="cnn_cfg2.json")

    # return eyes
    return eyes, out4


# 将workflow序列化成json
jsondata = xstream.serialize(my_workflow)  # 可以对其写入至文件等
print(jsondata)

# # 执行异步操作
# def on_data(**outputs):
#     # 回调函数
#     print("data recieved:", outputs)

# if __name__ == "__main__":
#     with xstream.Session(all_workflow, image = Variable("image")) as sess:
#         sess.Callback(on_data)
#         for image in image_set():
#             sess.Forward(image = image)  # 提供输入数据

#
