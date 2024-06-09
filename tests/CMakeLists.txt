cmake_minimum_required(VERSION 3.10)
project(tests)

# Include engine headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../engine/src)

# Set the source files for testbed
file(GLOB_RECURSE SOURCES "*/*.c")

foreach(testsourcefile ${SOURCES})
    # Cut off the file extension and directory path
    get_filename_component(testname ${testsourcefile} NAME_WE)
    # Append ".test" extension
    set(testname "${testname}.test")
    # Generate executable for each test source file
  message(STATUS "BUILDING TEST : ${testname}")
    add_executable(${testname} ${testsourcefile})
    # Link with engine library and any other required libraries
    target_link_libraries(${testname} PRIVATE engine ${X11_LIBRARIES} ${XCB_LIBRARIES})
endforeach(testsourcefile ${SOURCES})

