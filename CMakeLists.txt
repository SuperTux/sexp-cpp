# SExp - A S-Expression Parser for C++
# Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.0)
project(sexp)

include(mk/cmake/TinyCMMC.cmake)

include(GNUInstallDirs)
include(MaximumWarnings)
include(ClangTidy)

option(USE_LOCALE "Use C++ locale support" OFF)
option(USE_CXX17 "Use C++17 features" OFF)

if(USE_CXX17)
  # for std::from_chars()
  add_definitions(-DSEXP_USE_CXX17)
endif()

if(USE_LOCALE)
  add_definitions(-DSEXP_USE_LOCALE)
endif()

file(GLOB SEXP_SOURCES src/*.cpp)
file(GLOB SEXP_HEADER_SOURCES include/sexp/*.hpp)
add_library(sexp STATIC ${SEXP_SOURCES})
set_property(TARGET sexp
    PROPERTY CXX_STANDARD 17
    PROPERTY CXX_STANDARD_REQUIRED ON
    PROPERTY CXX_EXTENSIONS OFF)
set_target_properties(sexp PROPERTIES PUBLIC_HEADER "${SEXP_HEADER_SOURCES}")
target_compile_options(sexp PRIVATE ${WARNINGS_CXX_FLAGS})
target_include_directories(sexp SYSTEM PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/sexp.pc.in"
  "${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/sexp.pc"
  @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/sexp.pc"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

if(BUILD_TESTS)
  find_package(GTest REQUIRED)

  # build sexp tests
  file(GLOB TEST_SEXP_SOURCES tests/*.cpp)
  add_executable(test_sexp ${TEST_SEXP_SOURCES})
  set_property(TARGET test_sexp
    PROPERTY CXX_STANDARD 17
    PROPERTY CXX_STANDARD_REQUIRED ON
    PROPERTY CXX_EXTENSIONS OFF)
  target_compile_options(test_sexp PRIVATE ${WARNINGS_CXX_FLAGS})
  target_include_directories(test_sexp PUBLIC src/)
  target_link_libraries(test_sexp
    GTest::GTest
    GTest::Main
    sexp)

  # add 'make test' target, use 'make test ARGS="-V"' or 'ctest -V' for verbose
  enable_testing()
  add_test(NAME test_sexp
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND test_sexp)
endif()

if(BUILD_BENCHMARKS)
  find_package(benchmark REQUIRED)
  find_package(Threads REQUIRED)

  # build benchmarks
  file(GLOB BENCHMARKSOURCES benchmarks/*.cpp)
  foreach(SOURCE ${BENCHMARKSOURCES})
    get_filename_component(SOURCE_BASENAME ${SOURCE} NAME_WE)
    add_executable(${SOURCE_BASENAME} ${SOURCE})
    set_property(TARGET ${SOURCE_BASENAME}
      PROPERTY CXX_STANDARD 17
      PROPERTY CXX_STANDARD_REQUIRED ON
      PROPERTY CXX_EXTENSIONS OFF)
    target_link_libraries(${SOURCE_BASENAME}
      sexp
      benchmark::benchmark
      Threads::Threads
      )
    set_target_properties(${SOURCE_BASENAME} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/benchmarks/")
    target_compile_options(${SOURCE_BASENAME} PRIVATE -std=c++1y ${WARNINGS_CXX_FLAGS})
  endforeach()
endif()

include(ExportAndInstallLibrary)

# EOF #
