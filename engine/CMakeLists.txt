cmake_minimum_required(VERSION 3.10)
project(engine)

# the compiler being used
message(STATUS "Compiler being used: ${CMAKE_C_COMPILER}")

# Include the headers from the includes directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes)

# Automatically add all source files from the src directory
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

# Generate shared library from sources
add_library(engine SHARED ${SOURCES})

# for consumers of this library, you can specify them after the library has been declared.
target_include_directories(engine
PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
)

find_package(Vulkan REQUIRED)
if (Vulkan_FOUND)
    message(STATUS "Found Vulkan: ${Vulkan_INCLUDE_DIRS}")
    include_directories(${Vulkan_INCLUDE_DIRS})
    target_link_libraries(engine PRIVATE ${Vulkan_LIBRARIES})
else()
    message(FATAL_ERROR "Vulkan not found")
endif()


find_package(glfw3 REQUIRED)
if (glfw3_FOUND)
    message(STATUS "Found GLFW: ${GLFW_INCLUDE_DIRS}")
    include_directories(${GLFW_INCLUDE_DIRS})
    target_link_libraries(engine PRIVATE glfw)
else()
    message(FATAL_ERROR "GLFW not found")
endif()

