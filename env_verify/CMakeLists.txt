cmake_minimum_required(VERSION 3.10)
project(stereo_calibrectify)

# 启用自动处理 Qt 特性（关键！）
set(CMAKE_AUTOMOC ON)   # 自动生成 moc 文件
set(CMAKE_AUTOUIC ON)   # 自动生成 ui_*.h 文件
set(CMAKE_AUTORCC ON)   # 自动处理资源文件（可选）

# 查找Qt和OpenCV
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
find_package(OpenCV REQUIRED)

message(STATUS "PCL_INCLUDE_DIRS = ${PCL_INCLUDE_DIRS}")  # 输出设置的路径
message(STATUS "OpenCV_INCLUDE_DIRS = ${OpenCV_INCLUDE_DIRS}")  # 输出设置的路径

# find_package(PCL REQUIRED)
# set(CURL_ROOT "/usr/lib/x86_64-linux-gnu")
# message(STATUS "CURL_ROOT = ${CURL_ROOT}")  # 输出设置的路径
# find_package(CURL HINTS "/usr/lib/x86_64-linux-gnu")

# 添加 UI 文件到项目（假设 stereo_rectify.ui 在项目根目录）
qt5_wrap_ui(UI_HEADERS stereo_rectify.ui)  # 生成 ui_stereo_rectify.h

# 添加可执行文件（包含 UI 头文件）
add_executable(${PROJECT_NAME} 
    main.cpp
    ${UI_HEADERS}  # 将生成的 UI 头文件加入构建
)

# 链接库
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    ${OpenCV_LIBS}
    # ${PCL_LIBRARIES}
    # CURL::libcurl  # 添加此行链接 CURL 库
    
)

# 包含头文件
target_include_directories(${PROJECT_NAME} PRIVATE
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${PCL_INCLUDE_DIRS}
)

# set(CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath,/usr/lib/x86_64-linux-gnu")