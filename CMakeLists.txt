CMAKE_MINIMUM_REQUIRED (VERSION 3.17)

PROJECT (PMT)

INCLUDE (FindPkgConfig)

PKG_SEARCH_MODULE (SDL2 REQUIRED sdl2 IMPORTED_TARGET)
PKG_SEARCH_MODULE (FFTW3 REQUIRED fftw3 IMPORTED_TARGET)
PKG_SEARCH_MODULE (SPDLog REQUIRED spdlog IMPORTED_TARGET)
PKG_SEARCH_MODULE (OpenGL REQUIRED gl IMPORTED_TARGET)

ADD_EXECUTABLE (
    pmt
    ./src/main.cpp
    ./src/sdl2.cpp
)

SET_PROPERTY(TARGET pmt PROPERTY CXX_STANDARD 17)

TARGET_LINK_LIBRARIES (
    pmt
    PkgConfig::SPDLog
    PkgConfig::SDL2
    PkgConfig::FFTW3
    PkgConfig::OpenGL
)
