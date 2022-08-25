cmake_minimum_required(VERSION 3.20)

project(BrainChain VERSION 1.0 LANGUAGES C CXX)
set(TARGET_NAME "BrainChain")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(OpenCV_DIR /opt/intel/openvino_2021/opencv/cmake)
set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/opt/intel/openvino_2021/data_processing/gstreamer/lib/pkgconfig")
set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/opt/intel/openvino_2021/data_processing/dl_streamer/lib/pkgconfig")

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED core highgui imgproc imgcodecs videoio video gapi)
find_package(TBB REQUIRED tbb)


file (GLOB MAIN_SRC *.c *.cpp)
file (GLOB MAIN_HEADERS *.h)
file (GLOB MAIN_HPP *.hpp)
add_executable(${TARGET_NAME} ${MAIN_SRC} ${MAIN_HEADERS} ${MAIN_HPP})

#set_target_properties(${TARGET_NAME} PROPERTIES CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pthread -00")
set_target_properties(${TARGET_NAME} PROPERTIES CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pthread -O3 -mavx2 -ffast-math"")
set_target_properties(${TARGET_NAME} PROPERTIES CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pthread -O3 -mavx2 -ffast-math")
target_compile_options(${TARGET_NAME} PRIVATE -Wno-deprecated-declarations -Wno-tautological-pointer-compare)

target_include_directories(${TARGET_NAME}
        PRIVATE
        ${OpenCV_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(${TARGET_NAME}
        PRIVATE
        ${OpenCV_LIBS}
        ${TBB_IMPORTED_TARGETS}
        )

