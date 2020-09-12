cmake_minimum_required(VERSION 2.8)

project(vehicle_snap_method)

include_directories(
    include
    src
    test
)

file(GLOB PROJECT_INC
    "include/vehicle_common_utility/*.hpp"
    "include/vehicle_match_strategy/*.hpp"
    "include/vehicle_snap_strategy/*.hpp"
    "include/*.hpp"
    "include/*.h"
)

file(GLOB PROJECT_SRC
    "src/vehicle_common_utility/*.cpp"
    "src/vehicle_match_strategy/*.cpp"
    "src/vehicle_snap_strategy/*.cpp"
    "src/*.cpp"
)

set(SOURCE_FILES ${PROJECT_INC} ${PROJECT_SRC})

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
set(BASE_LIBRARIES xstream-framework jsoncpp hobotlog
    opencv_world protobuf dl m pthread rt)
target_link_libraries(${PROJECT_NAME} ${BASE_LIBRARIES})

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME}
    DESTINATION ${MY_OUTPUT_ROOT}/lib)
install(FILES ${PROJECT_SOURCE_DIR}/include/vehicle_snap_strategy/vehicle_snap_strategy_api.hpp
    DESTINATION ${MY_OUTPUT_ROOT}/include/${PROJECT_NAME}/)

add_subdirectory(test)
