#pragma once

// __has_include is currently supported by GCC and Clang. However GCC 4.9 may have issues and
// returns 1 for 'defined( __has_include )', while '__has_include' is actually not supported:
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=63662
#if __has_include(<filesystem>)
#include <filesystem>
#else
#include <experimental/filesystem>
#endif // __has_include

#if __has_include(<filesystem>)
namespace fs = std::filesystem;
#else
namespace fs = std::experimental::filesystem;
#endif // __has_include

#include <base/files/file_path.h>

namespace ctp {

/// \to use base::PathService
struct Options {
  // input files dir
  static fs::path src_path;

  // output files dir
  static fs::path res_path;

  // path to directory with plugins
  static base::FilePath pathToDirWithPlugins;

  // path to plugins configuration file
  static base::FilePath pathToDirWithPluginsConfigFile;

  /// \todo remove
  static std::vector<fs::path> ctp_scripts_search_paths;

  // paths to plugin files that
  // must be loaded independently from plugins configuration file
  static std::vector<base::FilePath> pathsToPluginFiles;
};

}
