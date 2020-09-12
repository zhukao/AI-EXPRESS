include_directories(
        ${PROJECT_SOURCE_DIR}/include
        include
        include/method
        include/hobotxstream
)

include_directories(
        ../../../../../deps/jsoncpp/include/
        ../../../../../deps/hobotlog/include/
        ../../../../../deps/opencv/include/
        ../../../../../deps/gtest/include/
        ../../vision_type/include/
)
link_directories(
        ../../../../../deps/hobotlog/lib/
        ../../../../../deps/jsoncpp/lib/
        ../../../../../deps/gtest/lib/
)

set(METHOD_FILES
          src/method/bbox_filter.cpp
          src/method/MultiSourceTestMethod.cpp
   )

set(SOURCE_FILES ${METHOD_FILES}
        gtest_main.cc
        method_factory.cpp
    )
set(BASE_LIBRARIES xstream hobotlog jsoncpp gtest pthread)

add_executable(method_init_test ${SOURCE_FILES} method_init_test.cpp)
target_link_libraries(method_init_test
                      PUBLIC ${BASE_LIBRARIES}
)

if(XSTREAM_HOBOTSDK_ENGINE)
  message("XSTREAM_HOBOTSDK_ENGINE is:${XSTREAM_HOBOTSDK_ENGINE}")
else()
  add_executable(timer_test ${SOURCE_FILES} timer_test.cpp)
  target_link_libraries(timer_test
                        PUBLIC ${BASE_LIBRARIES}
)
endif()

add_executable(xstream_unit_test ${SOURCE_FILES} method_cb.cpp)
target_link_libraries(xstream_unit_test
                      PUBLIC ${BASE_LIBRARIES}
)

add_executable(cpp_api_test ${SOURCE_FILES}  cpp_api_test.cc)
target_link_libraries(cpp_api_test
                      PUBLIC ${BASE_LIBRARIES}
)

add_executable(xstream_test ${SOURCE_FILES} xstream_test.cpp)
target_link_libraries(xstream_test
                      PUBLIC ${BASE_LIBRARIES}
)

add_executable(node_test ${SOURCE_FILES} node_test.cpp)
target_link_libraries(node_test
                      PUBLIC ${BASE_LIBRARIES}
)

add_executable(workflow_test ${SOURCE_FILES} subworkflow_test.cpp)
target_link_libraries(workflow_test
                      PUBLIC ${BASE_LIBRARIES}
)

#add_executable(c_api_test c_api_test.cc gtest_main.cc)
#target_link_libraries(c_api_test bbox_filter gtest pthread)

set(THREAD_MODEL_SOURCES ${SOURCE_FILES}
                         thread_model.cpp
                         src/method/bbox_filter.cpp
                         src/data_types/array.cpp
   )
add_executable(xstream_threadmodel_test ${THREAD_MODEL_SOURCES})
target_link_libraries(xstream_threadmodel_test
                      PUBLIC ${BASE_LIBRARIES}
)

set(THREAD_ORDER_SOURCES ${SOURCE_FILES}
                         thread_order.cpp
                         src/data_types/orderdata.cpp
   )
add_executable(xstream_threadorder_test ${THREAD_ORDER_SOURCES})
target_link_libraries(xstream_threadorder_test
                      PUBLIC ${BASE_LIBRARIES}
)

set(THREAD_SAFE_SOURCES ${SOURCE_FILES}
                         thread_safe.cpp
                         src/data_types/orderdata.cpp
   )
add_executable(xstream_threadsafe_test ${THREAD_SAFE_SOURCES})
target_link_libraries(xstream_threadsafe_test
                      PUBLIC ${BASE_LIBRARIES}
)

set(MULTISOURCE_TEST_SOURCES ${SOURCE_FILES}
                         multisource_test.cpp
   )
add_executable(xstream_multisource_test ${MULTISOURCE_TEST_SOURCES})
target_link_libraries(xstream_multisource_test
                      PUBLIC ${BASE_LIBRARIES}
)

add_executable(config_test ${SOURCE_FILES} config_test.cpp)
target_link_libraries(config_test
                      PUBLIC ${BASE_LIBRARIES}
)

