cmake_minimum_required(VERSION 3.10)
project(engine)

# Set the compiler to Clang
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror=vla")

message(STATUS "Compiler being used: ${CMAKE_C_COMPILER}")

# Include the headers from the includes directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes)

# Automatically add all source files from the src directory
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

# Generate shared library from sources
add_library(engine SHARED ${SOURCES})

# For consumers of this library, specify include directories after the library has been declared.
target_include_directories(engine
PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Find Vulkan and link
find_package(Vulkan REQUIRED)
if (Vulkan_FOUND)
    message(STATUS "Found Vulkan: ${Vulkan_INCLUDE_DIRS}")
    target_include_directories(engine PRIVATE ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(engine PRIVATE ${Vulkan_LIBRARIES})
else()
    message(FATAL_ERROR "Vulkan not found")
endif()

# Find and link X11
find_package(X11 REQUIRED)
if (X11_FOUND)
    message(STATUS "Found X11: ${X11_INCLUDE_DIR}")
    target_include_directories(engine PRIVATE ${X11_INCLUDE_DIR})
    target_link_libraries(engine PRIVATE ${X11_LIBRARIES} X11-xcb) # Including X11-xcb here
else()
    message(FATAL_ERROR "X11 not found")
endif()

# Find and link against XCB using pkg-config
find_package(PkgConfig REQUIRED)
pkg_search_module(XCB REQUIRED xcb)
if (XCB_FOUND)
    message(STATUS "Found XCB: ${XCB_INCLUDE_DIRS}")
    target_include_directories(engine PRIVATE ${XCB_INCLUDE_DIRS})
    target_link_libraries(engine PRIVATE ${XCB_LIBRARIES})
else()
    message(FATAL_ERROR "XCB not found")
endif()

