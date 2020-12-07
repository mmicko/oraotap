cmake_minimum_required(VERSION 3.5)
project(oraotap CXX)

set(CMAKE_CXX_STANDARD 14)

include_directories(3rdparty/cxxopts/include 3rdparty/wave/src)

aux_source_directory(src/ SOURCE_FILES)

add_library(wave
  3rdparty/wave/src/wave/header/data_header.h
  3rdparty/wave/src/wave/header/data_header.cc
  3rdparty/wave/src/wave/header/riff_header.h
  3rdparty/wave/src/wave/header/riff_header.cc
  3rdparty/wave/src/wave/header/fmt_header.h
  3rdparty/wave/src/wave/header/fmt_header.cc
  3rdparty/wave/src/wave/header/wave_header.h
  3rdparty/wave/src/wave/header/wave_header.cc

  3rdparty/wave/src/wave/header.h
  3rdparty/wave/src/wave/header.cc
  3rdparty/wave/src/wave/header_list.h
  3rdparty/wave/src/wave/header_list.cc

  3rdparty/wave/src/wave/error.h
  3rdparty/wave/src/wave/file.h
  3rdparty/wave/src/wave/file.cc
)

add_executable(oraotap ${SOURCE_FILES} ${INCLUDE_FILES})
target_link_libraries(oraotap PRIVATE wave)