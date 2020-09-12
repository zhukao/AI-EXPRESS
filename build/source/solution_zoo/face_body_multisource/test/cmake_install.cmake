# Install script for directory: /data/qingpeng.liu/nfs/github/0912_ai_express_release/source/solution_zoo/face_body_multisource/test

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

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/data/qingpeng.liu/nfs/github/0912_ai_express_release/output/face_body_multisource/face_body_multisource_test" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/data/qingpeng.liu/nfs/github/0912_ai_express_release/output/face_body_multisource/face_body_multisource_test")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/data/qingpeng.liu/nfs/github/0912_ai_express_release/output/face_body_multisource/face_body_multisource_test"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/data/qingpeng.liu/nfs/github/0912_ai_express_release/output/face_body_multisource/face_body_multisource_test")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/data/qingpeng.liu/nfs/github/0912_ai_express_release/output/face_body_multisource" TYPE EXECUTABLE FILES "/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/bin/face_body_multisource_test")
  if(EXISTS "$ENV{DESTDIR}/data/qingpeng.liu/nfs/github/0912_ai_express_release/output/face_body_multisource/face_body_multisource_test" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/data/qingpeng.liu/nfs/github/0912_ai_express_release/output/face_body_multisource/face_body_multisource_test")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/data/qingpeng.liu/nfs/github/0912_ai_express_release/output/face_body_multisource/face_body_multisource_test"
         OLD_RPATH "/data/qingpeng.liu/nfs/github/0912_ai_express_release/deps/x2_prebuilt/lib:/data/qingpeng.liu/nfs/github/0912_ai_express_release/deps/ipc_tracking/lib:/data/qingpeng.liu/nfs/github/0912_ai_express_release/deps/bpu_predict/x2/lib:/data/qingpeng.liu/nfs/github/0912_ai_express_release/deps/jsoncpp/lib:/data/qingpeng.liu/nfs/github/0912_ai_express_release/deps/hobotlog/lib:/data/qingpeng.liu/nfs/github/0912_ai_express_release/deps/protobuf/lib:/data/qingpeng.liu/nfs/github/0912_ai_express_release/deps/gtest/lib:/data/qingpeng.liu/nfs/github/0912_ai_express_release/deps/opencv/lib:/data/qingpeng.liu/nfs/github/0912_ai_express_release/deps/libjpeg-turbo/lib:/data/qingpeng.liu/nfs/github/0912_ai_express_release/deps/libyuv/lib:/data/qingpeng.liu/nfs/github/0912_ai_express_release/deps/hobot/lib:/data/qingpeng.liu/nfs/github/0912_ai_express_release/deps/iou_based_mot/lib:/data/qingpeng.liu/nfs/github/0912_ai_express_release/deps/zlib/lib:/data/qingpeng.liu/nfs/github/0912_ai_express_release/deps/live555/lib:/data/qingpeng.liu/nfs/github/0912_ai_express_release/deps/xwarehouse/lib:/data/qingpeng.liu/nfs/github/0912_ai_express_release/deps/uWS/lib64:/data/qingpeng.liu/nfs/github/0912_ai_express_release/deps/openssl/lib:/data/qingpeng.liu/nfs/github/0912_ai_express_release/build/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/data/qingpeng.liu/nfs/github/0912_ai_express_release/output/face_body_multisource/face_body_multisource_test")
    endif()
  endif()
endif()

