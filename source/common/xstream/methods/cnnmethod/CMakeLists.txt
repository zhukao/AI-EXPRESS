cmake_minimum_required(VERSION 2.8)
project(CNNMethod)

option(USE_BGR2NV12 "select BGR to convert directly to nv12" OFF)
add_definitions(-DSEARCH_PERFORMANCE)
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
    src/CNNMethod.cpp
    src/CNNConst.cpp
    src/util/ModelInfo.cpp
    src/util/AlignFace.cpp
    src/Predictor/Predictor.cpp
    src/Predictor/RectInputPredictor.cpp
    src/Predictor/ImgInputPredictor.cpp
    src/Predictor/LmkInputPredictor.cpp
    src/Predictor/VehicleImgInputPredictor.cpp
    src/Predictor/LmkSeqInputPredictor.cpp
    src/Predictor/VidPredictor.cpp
    src/PostPredictor/PostPredictor.cpp
    src/PostPredictor/AgeGenderPostPredictor.cpp
    src/PostPredictor/BinaryClassifyPostPredictor.cpp
    src/PostPredictor/AntiSpfPostPredictor.cpp
    src/PostPredictor/FaceIdPostPredictor.cpp
    src/PostPredictor/LmkPosePostPredictor.cpp
    src/PostPredictor/PlateNumPostPredictor.cpp
    src/PostPredictor/VehicleColorPostPredictor.cpp
    src/PostPredictor/VehicleTypePostPredictor.cpp
    src/PostPredictor/ClassifyPostPredictor.cpp
    src/PostPredictor/FaceQualityPostPredictor.cpp
    src/PostPredictor/ActPostPredictor.cpp
    src/PostPredictor/BackbonePostPredictor.cpp
    src/PostPredictor/VidPostPredictor.cpp
    src/PostPredictor/CommonLmkPostPredictor.cpp
    include/CNNMethod/CNNMethod.h
    include/CNNMethod/CNNConst.h
    include/CNNMethod/util/util.h
    include/CNNMethod/util/ModelInfo.h
    include/CNNMethod/util/AlignFace.h
    include/CNNMethod/util/ActDataPreprocess.h
    include/CNNMethod/util/CNNMethodData.h
    include/CNNMethod/util/CNNMethodConfig.h
    include/CNNMethod/Predictor/Predictor.h
    include/CNNMethod/Predictor/PredictorFactory.h
    include/CNNMethod/Predictor/ImgInputPredictor.h
    include/CNNMethod/Predictor/LmkInputPredictor.h
    include/CNNMethod/Predictor/RectInputPredictor.h
    include/CNNMethod/Predictor/VehicleImgInputPredictor.h
    include/CNNMethod/Predictor/LmkSeqInputPredictor.h
    include/CNNMethod/Predictor/VidPredictor.h
    include/CNNMethod/PostPredictor/PostPredictor.h
    include/CNNMethod/PostPredictor/AgeGenderPostPredictor.h
    include/CNNMethod/PostPredictor/BinaryClassifyPostPredictor.h
    include/CNNMethod/PostPredictor/AntiSpfPostPredictor.h
    include/CNNMethod/PostPredictor/FaceIdPostPredictor.h
    include/CNNMethod/PostPredictor/LmkPosePostPredictor.h
    include/CNNMethod/PostPredictor/PostPredictorFactory.h
    include/CNNMethod/PostPredictor/PlateNumPostPredictor.h
    include/CNNMethod/PostPredictor/VehicleColorPostPredictor.h
    include/CNNMethod/PostPredictor/VehicleTypePostPredictor.h
    include/CNNMethod/PostPredictor/ClassifyPostPredictor.h
    include/CNNMethod/PostPredictor/FaceQualityPostPredictor.h
    include/CNNMethod/PostPredictor/ActPostPredictor.h
    include/CNNMethod/PostPredictor/BackbonePostPredictor.h
    include/CNNMethod/PostPredictor/VidPostPredictor.h
    include/CNNMethod/PostPredictor/CommonLmkPostPredictor.h
)

include_directories(${PROJECT_SOURCE_DIR}/include)
link_directories(${PROJECT_SOURCE_DIR}/lib/3rdparty
                 ${PROJECT_SOURCE_DIR}/example/lib)

include_directories(${PROJECT_SOURCE_DIR}/example/include)

add_library(CNNMethod STATIC ${SOURCE_FILES})

add_subdirectory(test)
add_subdirectory(example)

# set(OUTPUT_ROOT ${PROJECT_SOURCE_DIR}/output)

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/CNNMethod)
install(TARGETS CNNMethod
        DESTINATION ${MY_OUTPUT_ROOT}/lib)
install(FILES ${PROJECT_SOURCE_DIR}/include/CNNMethod/CNNMethod.h
        DESTINATION ${MY_OUTPUT_ROOT}/include/CNNMethod/)
