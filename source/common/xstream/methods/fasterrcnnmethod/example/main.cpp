/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @brief     test
 * @author    yaoyao.sun
 * @date      2019.04.12
 */

#include <stdlib.h>
#include <iostream>
#include <map>
#include <string>

#include "hobotlog/hobotlog.hpp"

typedef int (*example_fn)(int, char **);

extern int TestModelInfo(int argc, char **argv);
extern int TestFasterRCNNImage(int argc, char **argv);
extern int TestFBPyramid(int argc, char **argv);
extern int TestFBFasterrcnn(int argc, char **argv);
extern int TestDumpNV12(int argc, char **argv);
extern int TestX2DEVSinglePyramid(int argc, char **argv);
extern int TestX2DEVDualPyramid(int argc, char **argv);
extern int TestX2DEVFasterRCNNPyramid(int argc, char **argv);
extern int TestHBCCInfo(int argc, char **argv);
extern int TestTwoFasterRCNN(int argc, char **argv);
extern int DumpFaceDetResult(int argc, char **argv);

void PrintUsage() {
  std::cout << "Usage: example [command] [args]\n"
      "\n"
      "Run a specified example.\n"
      "\n"
      "Command:\n"
      "  model_info\n"
      "  faster_rcnn_image\n"
      "  fb_pyramid\n"
      "  fb_fasterrcnn\n"
      "  dump_nv12\n"
      "  x2_dev_single_pyramid\n"
      "  x2_dev_dual_pyramid\n"
      "  x2_dev_faster_rcnn_pyramid\n"
      "  hbcc_info\n"
      "  two_faster_rcnn\n"
      "  dump_face_det_result\n"
      "\n"
      "Extra:\n"
      " \n"
      "You can use tools such as valrind to check memory leaks\n"
      "valgrind --leak-check=full"
      " --show-leak-kinds=all example [command] [subcommand] "
      << std::endl;
}

static std::map<std::string, example_fn> examples = {
    {"model_info", TestModelInfo},
    {"faster_rcnn_image", TestFasterRCNNImage},
    {"fb_pyramid", TestFBPyramid},
    {"dump_nv12", TestDumpNV12},
    {"fb_fasterrcnn", TestFBFasterrcnn},
    {"x2_dev_single_pyramid", TestX2DEVSinglePyramid},
    {"x2_dev_dual_pyramid", TestX2DEVDualPyramid},
    {"x2_dev_faster_rcnn_pyramid", TestX2DEVFasterRCNNPyramid},
    {"hbcc_info", TestHBCCInfo},
    {"two_faster_rcnn", TestTwoFasterRCNN},
    {"dump_face_det_result", DumpFaceDetResult}
};

int main(int argc, char **argv) {
  SetLogLevel(HOBOT_LOG_VERBOSE);
  if (argc > 1 && examples.count(argv[1])) {
    examples[argv[1]](argc - 1, argv + 1);
    std::cout << "main exit!!" << std::endl;
  } else {
    PrintUsage();
  }
}
