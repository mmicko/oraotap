cmake_minimum_required(VERSION 3.5)
project(oraotap)

set(CMAKE_CXX_STANDARD 14)

include_directories(include/)

aux_source_directory(src/ SOURCE_FILES)

add_executable(oraotap ${SOURCE_FILES} ${INCLUDE_FILES})
