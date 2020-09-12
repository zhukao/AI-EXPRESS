cmake_minimum_required(VERSION 2.8)

project(FilterMethod)

message("config types: ${CMAKE_CONFIGURATION_TYPES}")

option(RELEASE_LIB "build version of release" ON)
if (${RELEASE_LIB})
    set(CMAKE_BUILD_TYPE Release)
else ()
    set(CMAKE_BUILD_TYPE Debug)
endif ()

#[[
set(ARCH ${architecture})
if (${PLATFORM_ANDROID})
    add_definitions(-DENABLE_OMP)
    add_definitions(-DHR_POSIX)
    add_definitions(-DHR_ANDROID)
    list(APPEND CMAKE_C_FLAGS "-Wall -Wno-unknown-pragmas -fPIC")
    string(REGEX REPLACE "vfpv3-d16" "neon" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
elseif (${PLATFORM_LINUX})
    add_definitions(-DHR_POSIX)
    add_definitions(-DHR_LINUX)
    list(APPEND CMAKE_C_FLAGS "-Wall -Wno-unknown-pragmas -fPIC -fexceptions")
    if (NOT (${arch} STREQUAL "default") AND NOT (${arch} STREQUAL "x86"))
        if (${arch} STREQUAL "arm_hi3519")
            list(APPEND CMAKE_C_FLAGS " -mcpu=cortex-a17.cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4 ")
        elseif (${arch} STREQUAL "armhf")
            list(APPEND CMAKE_C_FLAGS " -marm -mfloat-abi=hard -mfpu=neon ")
        elseif (${arch} STREQUAL "aarch64")
            if (${vendor} STREQUAL "nvidia-tx2")
                list(APPEND CMAKE_C_FLAGS " --ffast-math -march=armv8-a+crypto -mcpu=cortex-a57+crypto ")
            elseif (${vendor} STREQUAL "linaro")
                # x2 soc
                list(APPEND CMAKE_C_FLAGS " -march=armv8-a ")
            else ()
                list(APPEND CMAKE_C_FLAGS " -march=armv8-a ")
            endif ()
        elseif (${arch} STREQUAL "armv8l")
            # x2 32 soc
        else ()
            list(APPEND CMAKE_C_FLAGS " -marm -mfloat-abi=softfp -mfpu=neon ")
        endif ()
    else ()   # linux on x86?
        #list(APPEND CMAKE_C_FLAGS " -msse4.2 ")
    endif ()
elseif (${PLATFORM_WIN})
    add_definitions(-DHR_WIN)
    add_definitions(-D_MBCS)
    add_definitions(-DHOBOT_EXPORTS)
    add_definitions(-DHOBOT_FACEPOST_DLL_EXPORTS)
    list(APPEND CMAKE_C_FLAGS " -msse4.2 ")
elseif (${PLATFORM_MAC})
    add_definitions(-DHR_POSIX)
    add_definitions(-DHR_MAC)
    list(APPEND CMAKE_C_FLAGS " -Wall -Wno-unknown-pragmas -fPIC -msse4.2 -fexceptions")
else ()
    message(FATAL_ERROR "invoke cmake with -DPLATFORM_MAC=TRUE|-DPLATFORM_LINUX=TRUE|-DPLATFORM_ANDROID=TRUE|-DPLATFORM_WIN=TRUE")
    return()
endif ()
]]

message("CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}" )

option(USE_ENCRYPT "whether to use hobot aes encrypt" OFF)
if (NOT ${USE_ENCRYPT})
    add_definitions(-DNO_ENCRYPT)
    message("not use hobot aes encrypt")
else ()
    message("use hobot aes encrypt")
endif()

option(USE_OMP "whether to use omp" OFF)
if (NOT ${USE_OMP})
    add_definitions(-DNO_OMP)
    message("not use omp..")
else ()
    message("use omp..")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp" )
endif ()

option(IPC_VER "whether to use omp" OFF)
if (NOT ${IPC_VER})
    add_definitions(-DNOIPC_VER)
    message("not ipc ver..")
else ()
    message("ipc ver..")
    add_definitions(-DUSE_ZLOG)
    message("use zlog....")
    add_definitions(-DNO_OMP)
    message("not use omp....")
endif ()

set(CMAKE_CXX_STANDARD 11)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -fPIC -O3")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DEBUG -g -O0 -fPIC")
endif()

include_directories(
        include/
)

set(SOURCE_FILES
        src/FilterMethod.cpp
        src/filter_data_type.cpp
)

add_library(FilterMethod STATIC ${SOURCE_FILES})

add_subdirectory(test)

if(NOT ${PARENT_BUILD})
set(OUTPUT_ROOT ${CMAKE_SOURCE_DIR}/output/)
endif()

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/${PROJECT_NAME}/)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/include)
install(TARGETS ${PROJECT_NAME}
        DESTINATION ${MY_OUTPUT_ROOT}/lib)
