include_directories(
        include
)


set(SOURCE_FILES
        gtest_main.cc
        test_main.cpp
)

add_executable(Filter_unit_test ${SOURCE_FILES})

if (${PLATFORM_WIN})
     set(CMAKE_CXX_FLAGS_RELEASE "/MT")
     set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
     target_link_libraries(Filter_unit_test debug gtestd)
     target_link_libraries(Filter_unit_test optimized gtest)
else()
     target_link_libraries(Filter_unit_test  ${PROJECT_NAME} jsoncpp hobotlog xstream-framework gtest pthread dl)
endif()
