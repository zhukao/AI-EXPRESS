cmake_minimum_required(VERSION 2.8)

project(vision_type C CXX)

include_directories(
        include
)

file(GLOB_RECURSE SOURCE_SRC "src/*.cpp" "src/*.cc" "src/*.c")

set(DEFINITIONS ${DEFINITIONS} -DHOBOT_EXPORTS)

set(INCLUDE_PATHES ${INCLUDE_SEARCH_PATH_LIST} ${CMAKE_HOME_DIRECTORY}/include/)
foreach (dir ${INCLUDE_PATHES})
    set(INCLUDES ${INCLUDES} -I ${dir})
endforeach ()

add_library(vision_type_util STATIC ${SOURCE_SRC})

add_subdirectory(test)

if (NOT ${PARENT_BUILD})
set(OUTPUT_ROOT ${CMAKE_SOURCE_DIR}/output)
endif()

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/vision_type)
install(TARGETS vision_type_util
        DESTINATION ${MY_OUTPUT_ROOT}/lib)
install(DIRECTORY include/
        DESTINATION ${MY_OUTPUT_ROOT}/include/)
install(FILES ${PROJECT_SOURCE_DIR}/include/hobot_vision/blocking_queue.hpp
        DESTINATION ${MY_OUTPUT_ROOT}/include/hobot_vision/)
