cmake_minimum_required(VERSION 3.21)

project(CVVerify VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CVVERIFY_BUILD_TESTS "Build CVVerify tests" ON)

message(STATUS "Configuring ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "Target platform: Windows first, cross-platform ready")

# Project structure will be filled incrementally during implementation.
