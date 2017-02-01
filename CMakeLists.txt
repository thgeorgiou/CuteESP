cmake_minimum_required(VERSION 3.6.3)
project(ESP_Flashing_Utility)

# C++14
set(CMAKE_CXX_STANDARD 14)

# Include folder
include_directories(inc/)

# QT Support
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5SerialPort REQUIRED)

# Generate UI headers
qt5_wrap_ui(UI_HEADERS inc/MainWindow.ui)

# Source files
file(GLOB SOURCE_FILES "src/*.cpp")

# Executable
add_executable(ESP_Flashing_Utility ${SOURCE_FILES} inc/MainWindow.h)
target_link_libraries(ESP_Flashing_Utility Qt5::Core Qt5::Widgets Qt5::SerialPort)

