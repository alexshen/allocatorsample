cmake_minimum_required(VERSION 3.2)
project(allocator
    LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
add_executable(allocator
    large_allocator.cpp
    main.cpp
    os_memory.cpp)
