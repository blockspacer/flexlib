#if 0
#include <map>
#include <string>

// use to stop optimizer discard unused variables
#define FORCE_LINKING(x) static void* volatile dummy = &x;

#if defined(WIN32) || defined(_WIN32)
#define EXPORTED  __declspec( dllexport )
#else // _WIN32
#define EXPORTED __attribute__((visibility("default")))
#endif

namespace plugin {

using PluginName = std::string;
using OptionName = std::string;
using OptionValue = std::string;

// both plugins (.so/.dll) and Cling scripts can use `extern`
// to modify |plugin_settings|
EXPORTED std::map<
 PluginName
 , std::map<OptionName, OptionValue> // plugin KV settings
> plugin_settings;

FORCE_LINKING(plugin::plugin_settings)

} // namespace plugin
#endif // 0
