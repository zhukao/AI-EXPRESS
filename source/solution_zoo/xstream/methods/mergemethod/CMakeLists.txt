cmake_minimum_required(VERSION 2.8)

project(MergeMethod)

option(SUPPORT_RGB_NIR_MERGE "support rgb nir merge" OFF)
if (${SUPPORT_RGB_NIR_MERGE})
    add_definitions(-DRGB_NIR_MERGE)
endif ()

include_directories(
        include
        include/MergeMethod
)

set(SOURCE_FILES
        src/method/MergeMethod.cpp
        src/method/data_type/data_type.cpp
        src/method/strategy/head_face.cpp
        src/method/strategy/head_body.cpp
        src/method/strategy/rgb_nir.cpp
        )

add_library(MergeMethod STATIC ${SOURCE_FILES})

set(BASE_LIBRARIES xstream-framework hobotlog jsoncpp opencv_world)
if (${SUPPORT_RGB_NIR_MERGE})
    set(BASE_LIBRARIES ${BASE_LIBRARIES} dual_camera_algo vio cam)
endif ()

target_link_libraries(${PROJECT_NAME} ${BASE_LIBRARIES})

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/MergeMethod/MergeMethod.h
        DESTINATION ${MY_OUTPUT_ROOT}/include/MergeMethod)

install(TARGETS ${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/lib)

add_subdirectory(test)
add_subdirectory(example)
