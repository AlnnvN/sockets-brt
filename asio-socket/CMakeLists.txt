cmake_minimum_required(VERSION 3.16)

project(bahiart VERSION 1.0)

find_library(PTHREAD pthread REQUIRED)
find_library(BOOST_SYSTEM boost_system REQUIRED)
find_library(BOOST_THREAD boost_thread REQUIRED)


add_library(
    networkmanager
    networkmanager.hpp
    networkmanager.cpp 
)

add_executable(bahiart main.cpp)

target_link_libraries(
    bahiart PUBLIC ${PTHREAD}
)
target_link_libraries(
    bahiart PUBLIC ${BOOST_SYSTEM}
)
target_link_libraries(
    bahiart PUBLIC ${BOOST_THREAD}
)
target_link_libraries(bahiart PUBLIC networkmanager)
