cmake_minimum_required(VERSION 3.10)
project(testbed)

# Include engine headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../engine/src)

# Set the source files for testbed
file(GLOB_RECURSE SOURCES "src/*.c")

# Generate executable for testbed
add_executable(testbed ${SOURCES})

# Link with engine library
target_link_libraries(testbed PRIVATE engine ${X11_LIBRARIES} ${XCB_LIBRARIES})

