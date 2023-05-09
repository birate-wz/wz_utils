# Install script for directory: /home/wz/wz_utils/base

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/wz/wz_utils/build/release-cpp11/release-install-cpp11")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/wz/wz_utils/build/release-cpp11/lib/libwz_base.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/wz/base" TYPE FILE FILES
    "/home/wz/wz_utils/base/AsyncLogging.h"
    "/home/wz/wz_utils/base/Atomic.h"
    "/home/wz/wz_utils/base/BlockingQueue.h"
    "/home/wz/wz_utils/base/Condition.h"
    "/home/wz/wz_utils/base/CountDownLatch.h"
    "/home/wz/wz_utils/base/LogStream.h"
    "/home/wz/wz_utils/base/Logging.h"
    "/home/wz/wz_utils/base/Mutex.h"
    "/home/wz/wz_utils/base/Thread.h"
    "/home/wz/wz_utils/base/ThreadPool.h"
    "/home/wz/wz_utils/base/cmd.h"
    "/home/wz/wz_utils/base/defer.h"
    "/home/wz/wz_utils/base/map.h"
    "/home/wz/wz_utils/base/noncopyable.h"
    "/home/wz/wz_utils/base/own_strings.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/wz/wz_utils/build/release-cpp11/base/test/cmake_install.cmake")

endif()

