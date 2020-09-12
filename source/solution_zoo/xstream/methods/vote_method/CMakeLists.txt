cmake_minimum_required(VERSION 2.8)

project(VoteMethod)
set(target_name vote_method)

include_directories(
    include
    include/VoteMethod
)

set(SOURCE_FILES
    src/vote_method.cpp
)

add_library(${target_name} STATIC ${SOURCE_FILES})
set(BASE_LIBRARIES xstream-framework jsoncpp hobotlog pthread)
target_link_libraries(${target_name} ${BASE_LIBRARIES})

add_subdirectory(test)
add_subdirectory(example)

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${target_name}/)
install(FILES ${PROJECT_SOURCE_DIR}/include/${target_name}/vote_method.h
        DESTINATION ${MY_OUTPUT_ROOT}/include/${target_name})
install(TARGETS ${target_name}
        DESTINATION ${MY_OUTPUT_ROOT}/lib)
#install(DIRECTORY config/ DESTINATION ${OUTPUT_ROOT}/config)



