#pragma once

#include <map>
#include <string>

namespace flexlib {

class PerPluginSettings {
public:
  using PluginName = std::string;
  using OptionName = std::string;
  using OptionValue = std::string;
  using OptionKV = std::map<OptionName, OptionValue>;

  static PerPluginSettings *getInstance();

  void setPluginSettings(
    const PluginName&
    , OptionKV&&);

  OptionValue getPluginOption(
    const PluginName&
    , const OptionName&) const;

  bool setPluginOption(
    const PluginName&
    , const OptionName&
    , const OptionValue&);

  bool hasPluginOptions(
    const PluginName&) const;

  const OptionKV& getAllPluginOptions(
    const PluginName&) const;

private:
  static PerPluginSettings *instance;

  PerPluginSettings();

  std::map<
    PluginName
    , std::map<
        OptionName,
        OptionValue
      >
  > storage_{};
};

} // namespace flexlib
