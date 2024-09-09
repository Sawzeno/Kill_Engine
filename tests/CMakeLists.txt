cmake_minimum_required(VERSION 3.10)
project(my_project)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../engine/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(darray)
add_subdirectory(filesystem)
add_subdirectory(linearallocator)
add_subdirectory(hashtable)
add_subdirectory(kstring)
