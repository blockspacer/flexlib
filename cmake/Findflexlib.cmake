get_filename_component(CURRENT_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
list(APPEND CMAKE_MODULE_PATH ${CURRENT_CMAKE_DIR})

#include(CMakeFindDependencyMacro) # use find_package instead

# NOTE: some packages may be optional (platform-specific, etc.)
# find_package(... REQUIRED)
find_package(chromium_base REQUIRED)
# see https://doc.magnum.graphics/corrade/corrade-cmake.html#corrade-cmake-subproject
find_package(Corrade REQUIRED PluginManager)
find_package(Cling)

list(REMOVE_AT CMAKE_MODULE_PATH -1)

if(NOT TARGET CONAN_PKG::flexlib)
  message(FATAL_ERROR "Use flexlib from conan")
endif()
set(flexlib_LIB CONAN_PKG::flexlib)
# conan package has '/include' dir
set(flexlib_HEADER_DIR
  ${CONAN_FLEXLIB_ROOT}/include
)
# used by https://docs.conan.io/en/latest/developing_packages/workspaces.html
if(TARGET flexlib)
  # name of created target
  set(flexlib_LIB flexlib)
  set(flexlib_HEADER_DIR
    ${CONAN_FLEXLIB_ROOT}/include
  )
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/cmake/flexlib-config.cmake")
  # uses Config.cmake or a -config.cmake file
  # see https://gitlab.kitware.com/cmake/community/wikis/doc/tutorials/How-to-create-a-ProjectConfig.cmake-file
  # BELOW MUST BE EQUAL TO find_package(... CONFIG REQUIRED)
  # NOTE: find_package(CONFIG) not supported with EMSCRIPTEN, so use include()
  include(${CMAKE_CURRENT_LIST_DIR}/cmake/flexlib-config.cmake)
endif()
message(STATUS "flexlib_HEADER_DIR=${flexlib_HEADER_DIR}")
