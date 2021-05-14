#include "flexlib/per_plugin_settings.hpp" // IWYU pragma: associated

#include "base/logging.h"
#include "base/check.h"

namespace flexlib {

PerPluginSettings* PerPluginSettings::instance;

PerPluginSettings* PerPluginSettings::getInstance()
{
  if (!instance) {
    instance = new PerPluginSettings;
  }

  return instance;
}

PerPluginSettings::PerPluginSettings() {}

void PerPluginSettings::setPluginSettings(
  const PerPluginSettings::PluginName& pluginName
  , PerPluginSettings::OptionKV&& optionKV)
{
  CHECK(!pluginName.empty())
    << "error: empty plugin name";
  CHECK(!optionKV.empty())
    << "error: empty plugin settings KV";

  if(storage_.find(pluginName) != storage_.end()) {
    LOG(WARNING)
      << "redefinition of plugin settings: "
      << pluginName;
  }

  storage_[pluginName] = std::move(optionKV);
}

PerPluginSettings::OptionValue PerPluginSettings::getPluginOption(
  const PerPluginSettings::PluginName& pluginName
  , const PerPluginSettings::OptionName& optionName) const
{
  const auto storageIt = storage_.find(pluginName);
  if(storageIt == storage_.end()) {
    LOG(WARNING)
      << "unable to get plugin settings: "
      << pluginName;
    return PerPluginSettings::OptionValue{};
  }

  const PerPluginSettings::OptionKV& optionKV
    = storageIt->second;

  auto optionIt = optionKV.find(optionName);
  if(optionIt == optionKV.end()) {
    LOG(WARNING)
      << "unable to get plugin option: "
      << optionName
      << " for plugin: "
      << pluginName;
    return PerPluginSettings::OptionValue{};
  }

  return optionIt->second;
}

bool PerPluginSettings::setPluginOption(
  const PerPluginSettings::PluginName& pluginName
  , const PerPluginSettings::OptionName& optionName
  , const PerPluginSettings::OptionValue& optionValue)
{
  const auto storageIt = storage_.find(pluginName);
  if(storageIt == storage_.end()) {
    LOG(WARNING)
      << "unable to set plugin option: "
      << pluginName;
    return false;
  }

  storage_[pluginName][optionName] = optionValue;

  return true;
}

const PerPluginSettings::OptionKV&
  PerPluginSettings::getAllPluginOptions(
    const PerPluginSettings::PluginName& pluginName) const
{
  const auto storageIt = storage_.find(pluginName);
  if(storageIt == storage_.end()) {
    LOG(ERROR)
      << "unable to get plugin options: "
      << pluginName;
    CHECK(false);
  }

  return storageIt->second;
}

bool PerPluginSettings::hasPluginOptions(
  const PerPluginSettings::PluginName& pluginName) const
{
  const auto storageIt = storage_.find(pluginName);
  return storageIt != storage_.end();
}

} // namespace flexlib
