cmake_minimum_required(VERSION 2.8)
project(face_solution)

set(SOURCE_FILES
        main.cpp
        method_factory.cpp
        )

add_executable(face_solution ${SOURCE_FILES})

set(BASE_LIBRARIES smartplugin visualplugin
        FasterRCNNMethod bpu_predict SnapShotMethod GradingMethod vote_method
        MOTMethod ipc_tracking xstream-framework xstream-imagetools hobotlog
        jsoncpp opencv_world websocketplugin vioplugin)
if (${PLATFORM_X2})
set(UVC_LIBRARIES )
endif()
if (${PLATFORM_X3})
set(UVC_LIBRARIES uvcplugin)
endif()
target_link_libraries(${PROJECT_NAME} ${BASE_LIBRARIES} ${UVC_LIBRARIES})

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)
install(DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/configs/
        DESTINATION ${MY_OUTPUT_ROOT}/configs/)
install(TARGETS ${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/)
