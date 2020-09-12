cmake_minimum_required(VERSION 2.8)

project(VehiclePlateMatchMethod)

include_directories(include)
file(GLOB SOURCE_FILES "src/*.cpp")

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} vehicle_snap_method)

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)
install(FILES ${PROJECT_SOURCE_DIR}/include/VehiclePlateMatchMethod/VehiclePlateMatchMethod.h
    DESTINATION ${MY_OUTPUT_ROOT}/include/VehiclePlateMatchMethod)
install(TARGETS ${PROJECT_NAME} DESTINATION ${MY_OUTPUT_ROOT}/lib)

add_subdirectory(test)


