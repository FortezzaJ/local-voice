cmake_minimum_required(VERSION 3.10)
project(localvoice-native)

set(CMAKE_CXX_STANDARD 17)

add_library(localvoice SHARED 
    native-lib.cpp
    native-bridge.cpp
)

find_library(log-lib log)

target_include_directories(localvoice PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(localvoice ${log-lib} dl)
