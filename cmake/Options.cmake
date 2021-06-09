include_guard( DIRECTORY )

set(FLEXLIB_SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/flexlib/)

# used by https://docs.conan.io/en/latest/developing_packages/workspaces.html
get_filename_component(LOCAL_BUILD_ABSOLUTE_ROOT_PATH
  "${PACKAGE_flexlib_SRC}"
  ABSOLUTE)
if(EXISTS "${LOCAL_BUILD_ABSOLUTE_ROOT_PATH}")
  # path to Find*.cmake file
  list(PREPEND CMAKE_MODULE_PATH "${LOCAL_BUILD_ABSOLUTE_ROOT_PATH}/cmake")
endif()

option(ENABLE_TESTS "Enable tests" OFF)

option(ENABLE_VALGRIND
  "Enable valgrind" OFF)

# see https://github.com/Ericsson/codechecker/blob/master/tools/report-converter/README.md#undefined-behaviour-sanitizer
# NOTE: Compile with -g and -fno-omit-frame-pointer
# to get proper debug information in your binary.
# NOTE: Run your program with environment variable UBSAN_OPTIONS=print_stacktrace=1.
# see https://github.com/google/sanitizers/wiki/SanitizerCommonFlags
option(ENABLE_UBSAN
  "Enable Undefined Behaviour Sanitizer" OFF)

# see https://github.com/google/sanitizers/wiki/AddressSanitizerLeakSanitizer
# see https://github.com/Ericsson/codechecker/blob/master/tools/report-converter/README.md#address-sanitizer
# NOTE: Compile with -g and -fno-omit-frame-pointer
# to get proper debug information in your binary.
# NOTE: use ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=suppressions=suppr.txt
# NOTE: You need the ASAN_OPTIONS=symbolize=1
# to turn on resolving addresses in object code
# to source code line numbers and filenames.
# This option is implicit for Clang but it won't do any harm.
# see https://github.com/google/sanitizers/wiki/SanitizerCommonFlags
option(ENABLE_ASAN
  "Enable Address Sanitizer" OFF)

# see https://github.com/Ericsson/codechecker/blob/master/tools/report-converter/README.md#memory-sanitizer
# NOTE: Compile with -g and -fno-omit-frame-pointer
# to get proper debug information in your binary.
option(ENABLE_MSAN
  "Enable Memory Sanitizer" OFF)

# see https://github.com/Ericsson/codechecker/blob/master/tools/report-converter/README.md#thread-sanitizer
# NOTE: Compile with -g
# to get proper debug information in your binary.
option(ENABLE_TSAN
  "Enable Thread Sanitizer" OFF)

set(${PROJECT_NAME}_BUILD_SHARED_LIBS
  FALSE CACHE BOOL
  "Use .so/.dll")

if(${PROJECT_NAME}_BUILD_SHARED_LIBS)
  set(CORE_LIB_TYPE SHARED)
  message(FATAL_ERROR "flexlib supports only static builds (for now)")
else()
  set(CORE_LIB_TYPE STATIC)
endif(${PROJECT_NAME}_BUILD_SHARED_LIBS)

set(ENABLE_CLING FALSE CACHE BOOL "ENABLE_CLING")
message(STATUS "ENABLE_CLING=${ENABLE_CLING}")

set(ENABLE_CLANG_FROM_CONAN FALSE CACHE BOOL "ENABLE_CLANG_FROM_CONAN")
message(STATUS "ENABLE_CLANG_FROM_CONAN=${ENABLE_CLANG_FROM_CONAN}")

if(ENABLE_CLANG_FROM_CONAN AND ENABLE_CLING)
  message(FATAL_ERROR
    "don't use both ENABLE_CLING and ENABLE_CLANG_FROM_CONAN at the same time. cling already provides clang libtooling")
endif()

set(CUSTOM_PLUGINS
    "${CMAKE_CURRENT_SOURCE_DIR}/custom_plugins.cmake"
    CACHE STRING
    "Path to custom plugins")
message(STATUS "CUSTOM_PLUGINS=${CUSTOM_PLUGINS}")
if(EXISTS ${CUSTOM_PLUGINS})
  include(${CUSTOM_PLUGINS})
endif()
