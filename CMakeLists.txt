cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

# set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(rsl-util VERSION 0.1 LANGUAGES CXX)

include(GNUInstallDirs)

add_library(rsl-util INTERFACE)
add_library(rsl::util ALIAS rsl-util)

# temporary compiler-specific flags to enable reflection
target_compile_features(rsl-util INTERFACE cxx_std_26)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(rsl-util INTERFACE "-stdlib=libc++" "-freflection-latest")
  target_link_options(rsl-util INTERFACE "-stdlib=libc++")
  target_link_libraries(rsl-util INTERFACE "c++abi")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(rsl-util INTERFACE "-freflection")
endif()


target_include_directories(rsl-util INTERFACE
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

option(BUILD_TESTING "Enable tests" ON)
option(BUILD_EXAMPLES "Enable examples" ON)
option(RSL_UTIL_INSTALL "Generate install target for rsl-util" ON)
option(RSL_UTIL_SINGLE_HEADER "Generate single-header files in include/rsl/single" ON)

if (RSL_UTIL_INSTALL)
  set(CMAKE_INSTALLDIR_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/rsl-util)

  set_target_properties(rsl-util PROPERTIES EXPORT_NAME util)
  install(TARGETS rsl-util EXPORT rsl-util-targets)
  install(EXPORT rsl-util-targets
    NAMESPACE rsl::
    DESTINATION ${CMAKE_INSTALLDIR_CMAKEDIR}
  )

  include(CMakePackageConfigHelpers)
  configure_package_config_file(cmake/rsl-util-config.cmake.in
    ${PROJECT_BINARY_DIR}/rsl-util-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALLDIR_CMAKEDIR}/rsl-util-config.cmake
  )
  write_basic_package_version_file(${PROJECT_BINARY_DIR}/rsl-util-config-version.cmake
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
  )
  install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
  install(FILES
    ${PROJECT_BINARY_DIR}/rsl-util-config-version.cmake
    ${PROJECT_BINARY_DIR}/rsl-util-config.cmake
    DESTINATION ${CMAKE_INSTALLDIR_CMAKEDIR}
  )
endif()

if (RSL_UTIL_SINGLE_HEADER)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)

  set(_single_dir ${CMAKE_CURRENT_LIST_DIR}/standalone/rsl)
  file(MAKE_DIRECTORY ${_single_dir})

  # Glob only files (LIST_DIRECTORIES false skips _impl/, serializer/, style/, single/)
  file(GLOB _rsl_headers LIST_DIRECTORIES false ${CMAKE_CURRENT_LIST_DIR}/include/rsl/*)

  foreach(_header IN LISTS _rsl_headers)
    get_filename_component(_name ${_header} NAME)
    execute_process(
      COMMAND ${Python3_EXECUTABLE}
              ${CMAKE_CURRENT_LIST_DIR}/scripts/construct_single_header.py
              ${_header}
              ${_single_dir}/${_name}
      RESULT_VARIABLE _result
      OUTPUT_QUIET
    )
    if(NOT _result EQUAL 0)
      message(WARNING "rsl-util: failed to generate single header for ${_name}")
    else()
      message(STATUS "rsl-util: generated single header standalone/rsl/${_name}")
    endif()
  endforeach()
endif()

if (BUILD_TESTING)
  message(STATUS "Building unit tests")

  enable_testing()
  add_executable(rsl-util-test "")
  add_subdirectory(test)

  find_package(GTest REQUIRED)
  target_link_libraries(rsl-util-test PRIVATE rsl-util)
  target_link_libraries(rsl-util-test PRIVATE GTest::gtest)

  include(GoogleTest)
  gtest_discover_tests(rsl-util-test)
endif()

if (BUILD_EXAMPLES)
  add_subdirectory(example)
endif()

