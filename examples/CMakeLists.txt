cmake_minimum_required(VERSION 3.14)

project(qasterixexample LANGUAGES CXX)

set(QT_CREATOR_SKIP_PACKAGE_MANAGER_SETUP ON)
set(QT_CREATOR_SKIP_CONAN_SETUP ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core Xml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Xml)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(sources main.cpp)

add_executable(qasterixexample)

target_sources(qasterixexample PRIVATE ${sources})

target_link_libraries(qasterixexample PUBLIC qasterix::qasterix
                                             Qt${QT_VERSION_MAJOR}::Core
                                             Qt${QT_VERSION_MAJOR}::Xml)
