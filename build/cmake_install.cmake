# Install script for directory: /data/qingpeng.liu/nfs/github/0912_ai_express_release

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/common/xproto/framework/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/common/xproto/msgtype/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/common/xproto/plugins/vioplugin/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/common/xproto/plugins/smartplugin/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/common/xproto/plugins/visualplugin/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/common/xproto/plugins/hbipcplugin/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/common/xproto/plugins/websocketplugin/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/common/xstream/framework/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/common/xstream/vision_type/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/common/xstream/tutorials/stage1/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/common/xstream/tutorials/stage2/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/common/xstream/tutorials/stage3/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/common/xstream/tutorials/stage4/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/common/xstream/methods/fasterrcnnmethod/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/common/xstream/imagetools/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/common/viowrapper/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/common/xstream/methods/cnnmethod/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/common/xstream/methods/motmethod/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/solution_zoo/xstream/methods/gradingmethod/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/solution_zoo/xstream/methods/filtermethod/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/solution_zoo/xstream/methods/mergemethod/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/solution_zoo/xstream/methods/behavior_method/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/solution_zoo/xstream/methods/snapshotmethod/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/solution_zoo/xstream/methods/vehicle_snap_method/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/solution_zoo/xstream/methods/vehicle_plate_match_method/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/solution_zoo/xstream/methods/plate_vote_method/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/solution_zoo/xstream/methods/FilterSkipFrameMethod/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/solution_zoo/xstream/methods/vote_method/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/solution_zoo/xstream/methods/ssd_method/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/solution_zoo/face/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/solution_zoo/body/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/solution_zoo/face_body_multisource/cmake_install.cmake")
  include("/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/source/solution_zoo/tools/xwarehouse/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
