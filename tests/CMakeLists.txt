cmake_minimum_required(VERSION 3.14)

project(qasterixtest LANGUAGES CXX)

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

find_package(GTest REQUIRED)

set(sources
    asterixtest.h
    asterixtest.cpp
    main.cpp)

add_executable(qasterixtest)

target_sources(qasterixtest PRIVATE
    ${sources})

add_test(NAME encode_test COMMAND qasterixtest)

target_link_libraries(qasterixtest PUBLIC qasterix::qasterix
                                          Qt${QT_VERSION_MAJOR}::Core
                                          Qt${QT_VERSION_MAJOR}::Xml
                                          GTest::GTest)
