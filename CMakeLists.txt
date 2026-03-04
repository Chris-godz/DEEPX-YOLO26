cmake_minimum_required(VERSION 3.14)
project(yolo26_demo_cpp DESCRIPTION "YOLO26 C++ Standalone Multi-channel Demo")

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
endif()

# Find OpenCV
find_package(OpenCV REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(yaml-cpp REQUIRED)

# Find dxrt library
find_package(PkgConfig REQUIRED)
# If dxrt is not in standard paths, we assume it's set via environment or CMAKE_PREFIX_PATH
find_library(DXRT_LIB dxrt REQUIRED)

# Set dx_app base directory for reusing utilities and postprocess
set(DX_APP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../dx-all-suite/dx-runtime/dx_app")

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${DX_APP_DIR}/extern/)
include_directories(${DX_APP_DIR}/src/utility/)
include_directories(${DX_APP_DIR}/src/postprocess/yolov26/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Common libraries for linking
set(COMMON_LIBS ${OpenCV_LIBS} ${DXRT_LIB} pthread Qt5::Widgets yaml-cpp)

# Define source files including dx_app utilities
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/engine.cpp
    src/workers.cpp
    ${DX_APP_DIR}/src/utility/common_util.cpp
    ${DX_APP_DIR}/src/postprocess/yolov26/yolov26_postprocess.cpp
)

# Common compiler flags
set(COMMON_COMPILE_FLAGS -Wall -Wextra -O3 )

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE ${COMMON_LIBS})

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE ${COMMON_COMPILE_FLAGS})
endif()
