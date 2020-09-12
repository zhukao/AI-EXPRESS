/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @brief     人脸/人体的多路/单路 solution demo
 * @author zhuoran.rong(zhuoran.rong@horizon.ai)
 * @date
 */

#include <signal.h>
#include <iostream>
#include <cstring>
#include <thread>

int solution_main(int argc, const char **argv);

int main(int argc, const char **argv) {
  if (argc < 4) {
    std::cout
        << "Usage: vehicle_demo vio_config_file xroc_config_file [-i/-d/-w/-f] "
        << std::endl;
    return 0;
  }
  return solution_main(argc, argv);
}

