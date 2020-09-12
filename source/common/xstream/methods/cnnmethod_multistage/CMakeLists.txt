cmake_minimum_required(VERSION 2.8)
project(CNNMethod_Multistage)

option(USE_BGR2NV12 "select BGR to convert directly to nv12" OFF)
message("USE_BGR2NV12 is ${USE_BGR2NV12}")
if(USE_BGR2NV12)
    add_definitions(-DUSE_BGR2NV12)
    message("select BGR to convert directly to nv12")
endif(USE_BGR2NV12)


option(COVERAGE_TEST "open coverage build flag" OFF)
message("COVERAGE_TEST is ${COVERAGE_TEST}")
if (${COVERAGE_TEST})
  list(APPEND CMAKE_C_FLAGS " --coverage")
endif()
if ((NOT (${RELEASE_LIB})) OR (${COVERAGE_TEST}))
  list(APPEND CMAKE_C_FLAGS " -g -0g")
endif()

include_directories(
        include
        src
)

set(SOURCE_FILES
    src/CNNPostProcessor/AgeGenderPostProcessor.cpp
    src/CNNPostProcessor/AntiSpfPostProcessor.cpp
    src/CNNPostProcessor/ClassifyPostProcessor.cpp
    src/CNNPostProcessor/CNNPostProcessor.cpp
    src/CNNPostProcessor/FaceIdPostProcessor.cpp
    src/CNNPostProcessor/FaceQualityPostProcessor.cpp
    src/CNNPostProcessor/LmkPosePostProcessor.cpp
    src/CNNPostProcessor/PlateNumPostProcessor.cpp
    src/CNNPostProcessor/VehicleColorPostProcessor.cpp
    src/CNNPostProcessor/VehicleTypePostProcessor.cpp
    src/CNNPredictor/CNNPredictor.cpp
    src/CNNPredictor/ImgInputPredictor.cpp
    src/CNNPredictor/LmkInputPredictor.cpp
    src/CNNPredictor/RectInputPredictor.cpp
    src/CNNPredictor/VehicleImgInputPredictor.cpp
    src/util/AlignFace.cpp
    src/CNNConst.cpp
    src/CNNPredictorOutputData.cpp

    include/CNNPostProcessor/AgeGenderPostProcessor.h
    include/CNNPostProcessor/AntiSpfPostProcessor.h
    include/CNNPostProcessor/ClassifyPostProcessor.h
    include/CNNPostProcessor/CNNPostProcessor.h
    include/CNNPostProcessor/FaceIdPostProcessor.h
    include/CNNPostProcessor/FaceQualityPostProcessor.h
    include/CNNPostProcessor/LmkPosePostProcessor.h
    include/CNNPostProcessor/PlateNumPostProcessor.h
    include/CNNPostProcessor/VehicleColorPostProcessor.h
    include/CNNPostProcessor/VehicleTypePostProcessor.h
    include/CNNPredictor/CNNPredictor.h
    include/CNNPredictor/ImgInputPredictor.h
    include/CNNPredictor/LmkInputPredictor.h
    include/CNNPredictor/RectInputPredictor.h
    include/CNNPredictor/VehicleImgInputPredictor.h
    include/util/AlignFace.h
    include/BPUModel_Manager.h
    include/CNNConst.h
    include/CNNMethodConfig.h
    include/CNNMethodCreator.h
    include/CNNPredictorOutputData.h
)

include_directories(${PROJECT_SOURCE_DIR}/include)
link_directories(${PROJECT_SOURCE_DIR}/lib/3rdparty
                 ${PROJECT_SOURCE_DIR}/example/lib)

include_directories(${PROJECT_SOURCE_DIR}/example/include)

add_library(CNNMethod_Multistage STATIC ${SOURCE_FILES})

add_subdirectory(example)

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/CNNMethod_Multistage)
install(TARGETS CNNMethod_Multistage
        DESTINATION ${MY_OUTPUT_ROOT}/lib)
install(DIRECTORY  ${PROJECT_SOURCE_DIR}/include/
        DESTINATION ${MY_OUTPUT_ROOT}/include/)
