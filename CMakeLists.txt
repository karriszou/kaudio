cmake_minimum_required(VERSION 3.16)

set(NAME "kaudio")

project(${NAME})

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wl,-O2 -std=c++11")

file(GLOB_RECURSE SOURCES ./src/*.cpp ./src/*.c)

add_executable(${NAME}
  ${SOURCES}
)

target_link_libraries(${NAME} PUBLIC
  openal
)
