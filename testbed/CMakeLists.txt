cmake_minimum_required(VERSION 3.10)
project(testbed)

# Include the testbed headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes)

# Include engine headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../engine/includes)

# Set the source files for testbed
file(GLOB_RECURSE SOURCES "src/*.c")

# Generate executable for testbed
add_executable(testbed ${SOURCES})

# Link with engine library
target_link_libraries(testbed engine)

