cmake_minimum_required(VERSION 3.0.0)
project(intel8080 VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(intel8080 src/main.cpp src/intel8080.cpp)
add_executable(post src/post.cpp src/intel8080.cpp)

add_test(NAME post
    COMMAND post)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
