cmake_minimum_required(VERSION 3.0.2)
project(base C CXX)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

set(CXX_FLAGS
 -g
 # -DVALGRIND
 -DCHECK_PTHREAD_RETURN_VALUE
 -D_FILE_OFFSET_BITS=64
 -Wall
 -Wextra
 -Werror
 -Wconversion
 -Wno-unused-parameter
 -Wold-style-cast
 -Woverloaded-virtual
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 -march=native
 # -MMD
 -std=c++17
 -rdynamic
 )

 set(base_SRCS
  AsyncLogging.cc
  Condition.cc
  CountDownLatch.cc
  CurrentThread.cc
  Date.cc
  Exception.cc
  FileUtil.cc
  LogFile.cc
  Logging.cc
  LogStream.cc
  ProcessInfo.cc
  Timestamp.cc
  Thread.cc
  ThreadPool.cc
  TimeZone.cc
  )
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

add_library(base ${base_SRCS})
target_link_libraries(base pthred rt)

add_subdirectory(tests)
