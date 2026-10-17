cmake_minimum_required(VERSION 3.14)
project(HolyBible CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(holybible src/main.cpp)

if(MSVC)
  target_compile_options(holybible PRIVATE /utf-8 /W4)
else()
  target_compile_options(holybible PRIVATE -Wall -Wextra)
endif()

# Copy the Bible text next to the program so it always finds it.
add_custom_command(TARGET holybible POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:holybible>/data
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_SOURCE_DIR}/data/kjv.tsv $<TARGET_FILE_DIR:holybible>/data/kjv.tsv)
