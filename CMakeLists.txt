cmake_minimum_required( VERSION 3.16.0 )

project( openBibleViewer )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC ON)

set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" )

MESSAGE( STATUS "CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH} )
MESSAGE( STATUS "CMAKE_INCLUDE_PATH: " ${CMAKE_INCLUDE_PATH} )

#### debug or release ####
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
endif()

message(STATUS "Using build type: ${CMAKE_BUILD_TYPE}")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    option( OBV_NO_DEBUG_OUTPUT "Do not show Debug output" ON )
else()
    option( OBV_NO_DEBUG_OUTPUT "Do not show Debug output" OFF )
    option( OBV_DEBUG "Do not show Debug output" ON )
endif()

#### options ####
option( OBV_PORTABLE_VERSION "Build a portable version" OFF )
option( OBV_SWORD_SUPPORT "Build with sword support" OFF )
option( OBV_USE_QUAZIP "Use QuaZip" OFF)

#we have to use toStdWString() on linux
if(MSVC)
    add_definitions(-DUNICODE -D_UNICODE)
    option( OBV_USE_WSTRING "Use toStdWString and fromWCharArray" OFF)
else()
    option( OBV_USE_WSTRING "Use toStdWString and fromWCharArray" ON)
endif()

#### constants ####
set(OBV_VERSION_NUMBER "0.9.1")
set(OBV_VERSION_STATUS "stable")
set(OBV_BUILD_DATE "2026-08-26")

#### packages ####
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Xml Network Sql PrintSupport WebEngineWidgets Qml Core5Compat)

find_package(Xapian REQUIRED)

# optional quazip, we are using by default unzip or 7za.exe on windows
IF(OBV_USE_QUAZIP)
  find_package( QuaZip REQUIRED)
endif()

if(UNIX)
  find_package(ZLIB REQUIRED)
endif()

## Sword
if(OBV_SWORD_SUPPORT)
    include (FindSword)
    if (NOT SWORD_NOT_FOUND)
        ADD_DEFINITIONS(-DBUILD_WITH_SWORD)
    else()
        set(OBV_SWORD_SUPPORT FALSE)
    endif()
endif(OBV_SWORD_SUPPORT)

if(OBV_SWORD_SUPPORT)
  set(SW_LIBS ${CURL_LIBRARIES} ${ICU_LIBRARIES} ${ICU_I18N_LIBRARIES} ${SWORD_LIBRARY})
  include_directories(
    ${SWORD_INCLUDE_DIR}
  )
else(OBV_SWORD_SUPPORT)
    set(SW_LIBS "")
endif(OBV_SWORD_SUPPORT)

message("=> ${ZLIB_INCLUDE_DIRS}")
include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${openBibleViewer_BINARY_DIR}/src
  ${ZLIB_INCLUDE_DIRS}
)

add_subdirectory( src/extern )
add_subdirectory( src/core )

# sources
include(sources)

if(APPLE)
  include(osx)
elseif(WIN32)
  include(windows)
else()
  include(linux)
endif()
