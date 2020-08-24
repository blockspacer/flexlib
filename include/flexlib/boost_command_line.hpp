﻿#pragma once

#include <base/logging.h>
#include <base/macros.h>
#include <base/sequence_checker.h>
#include <base/files/file_path.h>
#include <base/strings/string_piece.h>
#include <base/strings/string_piece_forward.h>
#include <base/strings/string_split.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <map>
#include <ostream>
#include <string>
#include <utility>
#include <vector>
#include <stddef.h>

namespace boost {
namespace program_options {
template <class charT> class basic_parsed_options;
} // namespace program_options
} // namespace boost

namespace cmd {

class UnergisteredOption
{
public:
  UnergisteredOption(
    const std::string& key
    , const std::vector<std::string>& values);

  UnergisteredOption(UnergisteredOption&& other);

  const std::string key;

  const std::vector<std::string> values;

  /// \note combines key with SINGLE value
  std::string KVToSting(
    const size_t value_index
    // separator for key and value concatenation
    , const std::string& separator = "") const;

  // useful for debug purposes
  /// \note combines key with ALL values WITHOUT separator
  // USAGE:
  // auto res = static_cast<std::string>(unergisteredOption);
  operator std::string() const;

private:
  DISALLOW_COPY_AND_ASSIGN(UnergisteredOption);
};

/// \note must be as simple as possible,
/// it is interface to communicate with boost::program_options
class BoostCmdParser {
public:
  // type will be used in
  // boost::program_options::basic_option
  // and boost::program_options::basic_parsed_options
  using charType
    = char;

  using variables_map
    = boost::program_options::variables_map;

  template<class charType>
  using basic_parsed_options
    = boost::program_options::basic_parsed_options<charType>;

  using options_description
    = boost::program_options::options_description;

  using options_description_easy_init
    = boost::program_options::options_description_easy_init;

  BoostCmdParser();

  ~BoostCmdParser();

  bool init(int argc, char* argv[]);

  // may be used to print help for ALL registered command-line options
  std::string optionsToString();

  options_description_easy_init&
  options();

  size_t count(const base::StringPiece& key);

  // get options not present in
  // boost::program_options::options_description_easy_init
  // i.e. cmd args passed by user, but unknown for program_options
  /// \note requires to set allow_unregistered()
  /// \note cache result of this function call
  /// if you use it in performance critical code
  std::vector<UnergisteredOption> unregisteredOptions();

  /// \note cache result of this function call
  /// if you use it in performance critical code
  template<typename T>
  const T& getAs(const base::StringPiece& key)
  {
    CHECK(!key.empty());
    CHECK(!vm_.empty());

    // Command-line argument can have alias.
    // We expect comma as delimiter.
    // Example: outdir,R
    std::vector<base::StringPiece> key_split
      = base::SplitStringPiece(
          key
          , ","
          , base::TRIM_WHITESPACE
          , base::SPLIT_WANT_NONEMPTY);
    variables_map::const_iterator optIter;
    for(const base::StringPiece& part: key_split) {
      optIter = vm_.find(part.as_string());
      if(optIter != vm_.end()) {
        DVLOG(9)
            << "Found associated command-line argument: "
            << part
            << " for key "
            << key;
        break;
      } else {
        DVLOG(9)
            << "Unable to find associated command-line argument: "
            << part
            << " for key "
            << key;
      }
    }

    CHECK(optIter != vm_.end())
        << "Unable to find any of command-line arguments: "
        << key;

    CHECK(vm_.count((*optIter).first))
        << "Unable to count any of command-line arguments: "
        << key;

    return
      (*optIter).second.as<T>();
  }

private:
  variables_map vm_{};

  std::unique_ptr<
    basic_parsed_options<charType>
    > parsed_options_;

  options_description desc_;

  options_description_easy_init
    options_;

  SEQUENCE_CHECKER(sequence_checker_);

  DISALLOW_COPY_AND_ASSIGN(BoostCmdParser);
};

// converts command-line argument to ABSOLUTE path
/// \note returns empty base::FilePath{} if path
/// is NOT valid directory
[[nodiscard]] /* do not ignore return value */
base::FilePath cmdKeyToDirectory(
  const char key[],
  cmd::BoostCmdParser& boostCmdParser);

// converts command-line argument to ABSOLUTE path
/// \note returns empty base::FilePath{} if path
/// is NOT valid file
[[nodiscard]] /* do not ignore return value */
base::FilePath cmdKeyToFile(
  const char key[],
  cmd::BoostCmdParser& boostCmdParser);

/// \note returns std::numeric_limits<int>::max() if
/// command-line argument is NOT specified or NOT convertable
/// to int
[[nodiscard]] /* do not ignore return value */
int cmdKeyToInt(
  const base::StringPiece& key,
  cmd::BoostCmdParser& boostCmdParser);

// converts command-line argument to path
// path may be NOT absolute
/// \note returns empty base::FilePath{}
/// if command-line argument
/// is NOT specified or NOT valid
[[nodiscard]] /* do not ignore return value */
base::FilePath getAsPath(
  const base::StringPiece& key,
  cmd::BoostCmdParser& boostCmdParser);

// calls |base::MakeAbsoluteFilePath| for each string in vector
/// \note On POSIX, |MakeAbsoluteFilePath| fails
/// if the path does not exist
std::vector<base::FilePath>
toFilePaths(const std::vector<std::string>& paths);

} // namespace cmd
