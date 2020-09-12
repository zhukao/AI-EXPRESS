/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      callback.h
 * @brief     callback function
 * @author    Qingpeng Liu (qingpeng.liu@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-02-24
 */

#ifndef XSTREAM_TUTORIALS_STAGE3_CALLBACK_H_
#define XSTREAM_TUTORIALS_STAGE3_CALLBACK_H_


namespace Stage3Async {

class Callback {
 public:
  void OnCallback(xstream::OutputDataPtr output) { ParseOutput(output); }

 private:
  void ParseOutput(xstream::OutputDataPtr output) {
    using xstream::BaseDataVector;
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
      std::cout << "Output Data " << pdata->name_ << ":" << std::endl;
      for (size_t i = 0; i < pdata->datas_.size(); ++i) {
        auto safe_data =
            std::static_pointer_cast<xstream::OrderData>(
                pdata->datas_[i]);
        std::cout << "used_sequence_id insert id = "
                  << safe_data->sequence_id << std::endl;
      }
    }
  }
};

}  // namespace Stage3Async

#endif  // XSTREAM_TUTORIALS_STAGE3_CALLBACK_H_
