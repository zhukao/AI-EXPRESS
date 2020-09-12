## 活体检测预测工具

#### `run_predict_parallel.py`
对测试集跑预测的入口，可以同时跑多个模型的测试。

#### `eval_single_frame.py`
输入预测工具输出的`anti_spoofing_outputs.txt`，给出**单帧模型**的预测指标。

#### `eval_multi_frames.py`
输入预测工具输出的`anti_spoofing_outputs.txt`，给出**单帧模型+策略**的预测指标。