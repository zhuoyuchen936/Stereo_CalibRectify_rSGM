cmake_minimum_required(VERSION 3.10)
project(stereo_calibrectify)

# 启用自动处理 Qt 特性（关键！）
set(CMAKE_AUTOMOC ON)   # 自动生成 moc 文件
set(CMAKE_AUTOUIC ON)   # 自动生成 ui_*.h 文件
set(CMAKE_AUTORCC ON)   # 自动处理资源文件（可选）

# 查找Qt和OpenCV
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
find_package(OpenCV REQUIRED)
# 查找 PCL 库
set(PCL_DIR "/mnt/d/ProgramFile/PCL_1_14/PCL\ 1.14.1/cmake")
message(STATUS "PCL_DIR = ${PCL_DIR}")  # 输出设置的路径
message(STATUS "PCL_FOUND = ${PCL_FOUND}")
message(STATUS "PCL_INCLUDE_DIRS = ${PCL_INCLUDE_DIRS}")
# if(NOT EXISTS ${PCL_INCLUDE_DIRS}/pcl/config.h)  # 检查头文件是否存在
#   pcl_report_not_found("PCL can not be found on this machine")
# endif()
find_package(PCL REQUIRED COMPONENTS common io)

# 指定 UI 文件路径（关键修改1）
set(UI_FILE "${CMAKE_SOURCE_DIR}/stereo_rectify.ui")
# 添加 UI 文件到项目（假设 stereo_rectify.ui 在项目根目录）
qt5_wrap_ui(UI_HEADERS ${UI_FILE})  # 生成 ui_stereo_rectify.h

# 添加可执行文件（包含 UI 头文件）
add_executable(${PROJECT_NAME} 
    main.cpp
    stereo_rectify.cpp
    stereo_sgm.cpp
    ${UI_HEADERS}  # 将生成的 UI 头文件加入构建
)

# 链接库
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    ${OpenCV_LIBS}
    ${PCL_LIBRARIES}
)

# 包含头文件
target_include_directories(${PROJECT_NAME} PRIVATE
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${PCL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}  # 包含构建目录下的生成文件
)