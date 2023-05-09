set(base_SRCS
  AsyncLogging.cc
  Condition.cc
  CountDownLatch.cc
  Logging.cc
  LogStream.cc
  Thread.cc
 # ThreadPool.cc
)
add_library(wz_base ${base_SRCS})
target_link_libraries(wz_base pthread rt)

#add_library(muduo_base_cpp11 ${base_SRCS})
#target_link_libraries(muduo_base_cpp11 pthread rt)
#set_target_properties(muduo_base_cpp11 PROPERTIES COMPILE_FLAGS "-std=c++0x")

install(TARGETS wz_base DESTINATION lib)
#install(TARGETS muduo_base_cpp11 DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/wz/base)

add_subdirectory(test)
