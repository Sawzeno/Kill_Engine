cmake_minimum_required(VERSION 3.10)
project(KILL_ENGINE)

# Include subdirectories
MESSAGE(STATUS  "ENGINE")
add_subdirectory(engine)

MESSAGE(STATUS  "TESTBED")
add_subdirectory(testbed)
