file(GLOB SOURCES "*.c")

get_filename_component(EXECUTABLE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(EXECUTABLE_NAME "${EXECUTABLE_NAME}.test")

add_executable(${EXECUTABLE_NAME} ${SOURCES} ${CMAKE_SOURCE_DIR}/tests/testmanager.c)

target_include_directories(${EXECUTABLE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/tests)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE engine ${X11_LIBRARIES} ${XCB_LIBRARIES})

