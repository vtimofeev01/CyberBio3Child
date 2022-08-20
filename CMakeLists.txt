cmake_minimum_required(VERSION 3.20)

project(BraiChain VERSION 1.0 LANGUAGES C CXX)
set(TARGET_NAME "BraiChain")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
#

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
#add_subdirectory(intel)

set(OpenCV_DIR /opt/intel/openvino_2021/opencv/cmake)
set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/opt/intel/openvino_2021/data_processing/gstreamer/lib/pkgconfig")
set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/opt/intel/openvino_2021/data_processing/dl_streamer/lib/pkgconfig")

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED core highgui imgproc imgcodecs videoio video gapi)
find_package(spdlog REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(TBB REQUIRED tbb)

pkg_check_modules(GSTREAMER gstreamer-1.0>=1.16 REQUIRED)
pkg_check_modules(GLIB2 glib-2.0 REQUIRED)
pkg_check_modules(GSTVIDEO gstreamer-video-1.0>=1.16 REQUIRED)

# use pkg-config if sample builds as standalone. Otherwise vars DLSTREAMER_INCLUDE_DIRS/etc set by top level cmake
if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${PROJECT_SOURCE_DIR})
    pkg_check_modules(DLSTREAMER dl-streamer REQUIRED)
endif()

file (GLOB MAIN_SRC *.c *.cpp)
file (GLOB MAIN_HEADERS *.h)
file (GLOB MAIN_HPP *.hpp)
add_executable(${TARGET_NAME} ${MAIN_SRC} ${MAIN_HEADERS} ${MAIN_HPP})

set_target_properties(${TARGET_NAME} PROPERTIES CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pthread -00")
set_target_properties(${TARGET_NAME} PROPERTIES CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pthread -O3 -mavx2 -ffast-math")
target_compile_options(${TARGET_NAME} PRIVATE -Wno-deprecated-declarations -Wno-tautological-pointer-compare)

target_include_directories(${TARGET_NAME}
        PRIVATE
        ${GSTVIDEO_INCLUDE_DIRS}
        ${GSTREAMER_INCLUDE_DIRS}
        ${GLIB2_INCLUDE_DIRS}
        ${DLSTREAMER_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
        ${YAML_CPP_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
        /lib/x86_64-linux-gnu/
        )

target_link_libraries(${TARGET_NAME}
        PRIVATE
        ${GSTVIDEO_LIBRARIES}
        ${GSTREAMER_LIBRARIES}
        ${GLIB2_LIBRARIES}
        ${DLSTREAMER_LIBRARIES}
        ${OpenCV_LIBS}
        ${spdlog_LIBS}
        ${YAML_CPP_LIBRARIES}
        ${TBB_IMPORTED_TARGETS}
        yaml-cpp
        /opt/intel/openvino_2021/data_processing/gstreamer/lib/libpaho-mqtt3c.so
        /opt/intel/openvino_2021/data_processing/gstreamer/lib/libpaho-mqtt3a.so
        /lib/x86_64-linux-gnu/libpthread.so.0
        )

