cmake_minimum_required(VERSION 3.5)
project(TalkSync)

message(STATUS "src this is source dir " ${CMAKE_SOURCE_DIR})

# set different program names
SET(TALKSYNC "talksync")
SET(TALKSYNC_TEST "talksync_test")

# set property
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


# 编译libevent
# 设置libevent路径
set(LIBEVENT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libevent)
# 添加libevent库
find_library(LIBEVENT_LIBRARIES
    NAMES event
    PATHS ${LIBEVENT_ROOT}/lib
)
# 添加libevent头文件路径
include_directories(${LIBEVENT_ROOT}/include)

# 编译Log_Design
set(LOG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/Log_Design)
# 添加libevent库
find_library(LOGDESIGN_LIBRARIES
    NAMES log
    PATHS ${LOG_ROOT}/lib
)
# 添加libevent头文件路径
include_directories(${LOG_ROOT}/include)




# 添加项目的源文件
add_executable(${TALKSYNC} ${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp)

# 链接libevent库
target_link_libraries(${TALKSYNC} 
                      ${LIBEVENT_LIBRARIES}
                      ${LOGDESIGN_LIBRARIES})

