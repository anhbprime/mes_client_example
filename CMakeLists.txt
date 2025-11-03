cmake_minimum_required(VERSION 3.20)
project(mesClientExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(mes_client_example
    src/main.cpp
    src/Queue.hpp
    src/Messages.hpp
    src/MESClient.hpp
    src/workers/IOWorker.hpp
    src/workers/MESWorker.hpp
)

# src를 포함 경로로 추가 (main.cpp에서 #include "workers/IOWorker.hpp" 등이 동작하도록)
target_include_directories(mes_client_example PRIVATE src)

if (MSVC)
  target_compile_options(mes_client_example PRIVATE /W4 /utf-8 /permissive- /Zc:preprocessor)
else()
  target_compile_options(mes_client_example PRIVATE -Wall -Wextra -Wpedantic)
  target_link_libraries(mes_client_example PRIVATE pthread)
endif()
