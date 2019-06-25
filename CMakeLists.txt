cmake_minimum_required(VERSION 2.8)

project(ChainHashTable)
add_executable(${PROJECT_NAME} "main.cpp")

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
