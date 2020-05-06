#include "flexlib/options/ctp/options.hpp" // IWYU pragma: associated

namespace ctp {

fs::path Options::src_path{};

fs::path Options::res_path{};

/// \todo remove
std::vector<fs::path> Options::ctp_scripts_search_paths{};

base::FilePath Options::pathToDirWithPlugins{};

base::FilePath Options::pathToDirWithPluginsConfigFile{};

std::vector<base::FilePath> Options::pathsToExtraPluginFiles{};
}
