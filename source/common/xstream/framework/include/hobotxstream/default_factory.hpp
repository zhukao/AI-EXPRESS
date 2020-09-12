/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @brief     default method factory
 * @author    zhuoran.rong
 * @email     zhuoran.rong@horizon.ai
 * @version   0.0.0.1
 * @date      2020.04.15
 */

#include <string>

#include "hobotlog/hobotlog.hpp"
#include "hobotxstream/method_factory.h"

namespace xstream {

namespace method_factory {
    // 默认全局Method Factory工厂函数的声明
    // 该符号为一个弱符号,也就是说:如果这个函数没有
    // 被定义,编译链接仍然成功,但是使用的时候会报错
    // 这样做的好处是,如果用户不使用Default Method Factory,
    // 就不需要再定义一个为了防止链接出错的空函数放进代码里
    __attribute__ ((weak))  // NOLINT
    MethodPtr CreateMethod(const std::string &method_name);
}

// 默认MethodFactory
class DefaultMethodFactory final : public MethodFactory {
 public:
    DefaultMethodFactory() {
        // 构造默认Method Factory的时候,检查这个函数是否链接进来了.
        HOBOT_CHECK(method_factory::CreateMethod)
            << "Missing global default Method Factory";
    }
    ~DefaultMethodFactory() = default;

    // Method 工厂
    MethodPtr CreateMethod(const std::string &method_name) {
        return method_factory::CreateMethod(method_name);
    }
};

}  // namespace xstream
