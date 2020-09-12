cmake_minimum_required(VERSION 2.8)

project(BehaviorMethod)

include_directories(
        include
)

set(SOURCE_FILES
        src/BehaviorMethod.cpp
        src/BehaviorEvent.cpp
        src/RaiseHandEvent.cpp
        src/StandEvent.cpp
        src/SquatEvent.cpp)

add_library(BehaviorMethod STATIC ${SOURCE_FILES})

set(BASE_LIBRARIES xstream-framework hobotlog jsoncpp opencv_world)
target_link_libraries(${PROJECT_NAME} ${BASE_LIBRARIES})

add_subdirectory(test)

if(NOT ${PARENT_BUILD})
  set(OUTPUT_ROOT ${CMAKE_SOURCE_DIR}/output/)
endif()

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/include)
install(TARGETS ${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/lib)
