cmake_minimum_required(VERSION 3.5)

project(TalkSync)

message(STATUS "src this is source dir " ${CMAKE_SOURCE_DIR})

# set different program names
SET(TALKSYNC "talksync")
SET(TALKSYNC_TEST "talksync_test")
set(LOGDESIGN_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/Log_Design)
set(LIBEVENT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libevent)
set(GOOGLETEST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest)
set(ENCFS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/Encfs)
set(LOGDESIGN "Log_Design_Test")
set(ENCFS_MAIN "encfs_main")
set(ENCFSCTL "encfsctl")
set(MAKEKEY "makekey")


# set property
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


# libevent
find_library(LIBEVENT_LIBRARIES
    NAMES event
    PATHS ${LIBEVENT_ROOT}/lib
)

# Log_Design
add_subdirectory(${LOGDESIGN_ROOT})
# set sub-project executable file output directory
set_target_properties(${LOGDESIGN} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Log_output"
)


# GoogleTest
add_subdirectory(${GOOGLETEST_ROOT})

# Encfs
add_subdirectory(${ENCFS_ROOT})
set_target_properties(${ENCFS_MAIN} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Encfs_output"
)
set_target_properties(${ENCFSCTL} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Encfs_output"
)
set_target_properties(${MAKEKEY} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Encfs_output"
)


# add header file
include_directories(${LIBEVENT_ROOT}/include
                    ${LOGDESIGN_ROOT}/include
                    ${GOOGLETEST_ROOT}/googletest/incldue
                    ${GOOGLETEST_ROOT}/googlemock/include
                    )

# 添加项目的源文件
add_executable(${TALKSYNC} ${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp)

# 链接libevent库
target_link_libraries(${TALKSYNC} 
                      ${LIBEVENT_LIBRARIES}
                      logbase
                      gmock
                      gtest
                      encfs
                      )

