cmake_minimum_required(VERSION 3.16...3.25)

if(${CMAKE_VERSION} VERSION_LESS 3.16)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.16)
endif()

project(bahiart VERSION 1.0
                DESCRIPTION ""
                LANGUAGES CXX) 
      
add_library(tcpsocket tcpsocket.hpp tcpsocket.cpp)
add_library(udpsocket udpsocket.hpp udpsocket.cpp)

add_executable(bahiart main.cpp)
target_link_libraries(bahiart PRIVATE tcpsocket)
target_link_libraries(bahiart PRIVATE udpsocket)
