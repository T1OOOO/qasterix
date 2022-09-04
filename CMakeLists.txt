cmake_minimum_required(VERSION 3.14)

project(QASTERIX VERSION 0.5.0
                 DESCRIPTION "Library for encode and decode asterix"
                 LANGUAGES CXX C)

#Version
add_definitions(-DQASTERIX_VERSION_STR="${QASTERIX_VERSION_MAJOR}.${QASTERIX_VERSION_MINOR}.${QASTERIX_VERSION_PATCH}"
                -DQASTERIX_VERSION_MAJOR=${QASTERIX_VERSION_MAJOR}
                -DQASTERIX_VERSION_MINOR=${QASTERIX_VERSION_MINOR}
                -DQASTERIX_VERSION_PATCH=${QASTERIX_VERSION_PATCH}
                )

get_directory_property(QASTERIX_HAS_PARENT PARENT_DIRECTORY)
if(QASTERIX_HAS_PARENT)
    set(QASTERIX_MASTER_PROJECT OFF)
else()
    set(QASTERIX_MASTER_PROJECT ON)
endif(QASTERIX_HAS_PARENT)

#Options
option(QASTERIX_BUILD_TEST "Build QAsterix tests" ${QASTERIX_MASTER_PROJECT})
option(QASTERIX_BUILD_EXAMPLE "Build QAsterix examples" ${QASTERIX_MASTER_PROJECT})
option(QASTERIX_BUILD_DOCS "Build QAsterix documentation" OFF)
option(QASTERIX_INSTALL_PACKAGING "Generate target for installing QAsterix" ${QASTERIX_MASTER_PROJECT})
option(QASTERIX_BUILD_SHARED "Build as shared library" OFF)

include(GNUInstallDirs)

#Module
set(QASTERIX_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH "${QASTERIX_MODULE_DIR}")

if (NOT DEFINED CMAKE_CXX_VISIBILITY_PRESET AND
    NOT DEFINED CMAKE_VISIBILITY_INLINES_HIDDEN)
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
  set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
endif ()

if(DEFINED QASTERIX_BUILD_SHARED)
    set(BUILD_SHARED_LIBS ${QASTERIX_SHARED_LIBS})
endif()

set(QT_CREATOR_SKIP_PACKAGE_MANAGER_SETUP ON)
set(QT_CREATOR_SKIP_CONAN_SETUP ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

#Qt
find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core Xml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Xml)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(header_path ${QASTERIX_SOURCE_DIR}/include/qasterix)
set(dependency_path ${QASTERIX_SOURCE_DIR}/3rdparty)
set(bitfield_path ${dependency_path}/bitfield-c/src)
set(source_path ${QASTERIX_SOURCE_DIR}/src)

include_directories(${header_path})
include_directories(${bitfield_path})

set(public_headers
    ${header_path}/AsterixLibrary
    ${header_path}/AsterixEncoder
    ${header_path}/AsterixDecoder
)

set(private_headers
    ${header_path}/internal/asterixlibrary.h
    ${header_path}/internal/asterixdecoder.h
    ${header_path}/internal/asterixencoder.h
)

set(asterix_headers
    ${public_headers}
    ${private_headers}
)

set(bitfield_sources
    ${bitfield_path}/bitfield/bitfield.h
    ${bitfield_path}/bitfield/8byte.h
    ${bitfield_path}/bitfield/bitfield.c
    ${bitfield_path}/bitfield/bitarray.c
    ${bitfield_path}/bitfield/8byte.c
)

set(asterix_sources
    ${source_path}/asterixlibrary.cpp
    ${source_path}/asterixconfigkeeper.cpp
    ${source_path}/asterixconfigkeeper.h
    ${source_path}/asterixdecoder.cpp
    ${source_path}/asterixencoder.cpp
    ${source_path}/asterixnames.h
)

add_library(qasterix)
add_library(qasterix::qasterix ALIAS qasterix)

include(GenerateExportHeader)
generate_export_header(qasterix EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/qasterix/export/qasterix_export.h)
target_compile_definitions(qasterix PUBLIC "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:QASTERIX_STATIC_DEFINE>")

list(APPEND public_headers "${CMAKE_CURRENT_SOURCE_DIR}/include/qasterix/export/qasterix_export.h")

set(sources
    ${public_headers}
    ${private_headers}
    ${bitfield_sources}
    ${asterix_sources}
)

source_group(TREE ${header_path} FILES ${asterix_headers})
source_group(TREE ${dependency_path} FILES ${bitfield_sources})
source_group(TREE ${source_path} FILES ${asterix_sources})

qt_add_resources(sources ${CMAKE_CURRENT_SOURCE_DIR}/config.qrc)

target_include_directories(qasterix
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_sources(qasterix PRIVATE ${sources})

set_target_properties(qasterix PROPERTIES
   PUBLIC_HEADER "${public_headers}"
   SOVERSION ${PROJECT_VERSION_MAJOR}
   VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
)

set_target_properties(qasterix PROPERTIES
   ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
   LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
   RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_link_libraries(qasterix PUBLIC Qt${QT_VERSION_MAJOR}::Core
                                      Qt${QT_VERSION_MAJOR}::Xml)

include(installs)

if(QASTERIX_BUILD_TEST)
   enable_testing()

   add_subdirectory(tests)
endif(QASTERIX_BUILD_TEST)

if(QASTERIX_BUILD_EXAMPLE)
   add_subdirectory(examples)
endif(QASTERIX_BUILD_EXAMPLE)

if(QASTERIX_BUILD_DOCS)
    include(docs)
endif(QASTERIX_BUILD_DOCS)

if (QASTERIX_INSTALL_PACKAGING)
   include(packaging)
endif ()


