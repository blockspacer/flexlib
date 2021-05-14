#include "flexlib/boost_command_line.hpp" // IWYU pragma: associated

#include <base/logging.h>
#include <base/command_line.h>
#include <base/base_switches.h>
#include <base/feature_list.h>
#include <base/strings/string_piece.h>
#include <base/files/file_util.h>
#include <base/check.h>

#include <boost/none.hpp>
#include <boost/optional/optional.hpp>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/errors.hpp>
#include <boost/program_options/option.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>

#include <limits>
#include <algorithm>
#include <exception>
#include <iterator>
#include <sstream>

#define LOG_PATH_NOT_DIRECTORY(severity, path) \
  LOG(severity) \
    << "path must be directory: " \
    << path;

#define LOG_PATH_MUST_BE_NOT_DIRECTORY(severity, path) \
  LOG(severity) \
    << "path must be NOT directory: " \
    << path;

#define VLOG_NOT_INITIALIZED(severity, key) \
  VLOG(severity) \
    << "command like argument " \
    << key \
    << " is not initialized";

#define LOG_PATH_NOT_EXIST(severity, path) \
  LOG(severity) \
    << "path must exist: " \
    << path;

namespace po = boost::program_options;

namespace std {

/// \todo code repeat
template<class T>
std::ostream& operator<<(
  std::ostream& stream, const std::vector<T>& data)
{
  std::copy(data.begin(), data.end(),
            std::ostream_iterator<T>(stream, " "));
  return
    stream;
}

} // namespace std

namespace cmd {

UnergisteredOption::
  operator std::string() const
{
  //CHECK(key);
  //CHECK(values);
  std::string result = key;
  for (const auto &piece : values) {
    result += piece;
  }
  return
    result;
}

UnergisteredOption::UnergisteredOption(
  const std::string& _key
  , const std::vector<std::string>& _values)
  : key(_key) /// \note copy
  , values(_values) /// \note copy
{}

UnergisteredOption::UnergisteredOption(
  UnergisteredOption &&other)
  : key(other.key)
  , values(other.values)
{}

std::string
UnergisteredOption::KVToSting(
  const size_t value_index
  , const std::string &separator) const
{
  //CHECK(key);
  //CHECK(values);
  CHECK(value_index >=0 && value_index < values.size());
  VLOG(9)
    << "KVToSting. key: "
    << key
    << " separator: "
    << separator
    << " values: "
    << values
    << " value: "
    << (values)[value_index];
  return
    key + separator + values[value_index];
}

BoostCmdParser::BoostCmdParser()
  : desc_("Allowed options")
  , options_(desc_.add_options())
{
  DETACH_FROM_SEQUENCE(sequence_checker_);
}

BoostCmdParser::~BoostCmdParser()
{
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
}

size_t BoostCmdParser::count(const base::StringPiece &key)
{
  CHECK(!key.empty());
  CHECK(key.find_first_of(',')
        == base::StringPiece::npos);
  return
    vm_.count(key.as_string());
}

BoostCmdParser::options_description_easy_init&
BoostCmdParser::options()
{
  return
    options_;
}

std::string BoostCmdParser::optionsToString()
{
  std::stringstream stream;
  desc_.print(stream);
  return
    stream.str();
}

std::vector<UnergisteredOption>
BoostCmdParser::unregisteredOptions()
{
  CHECK(parsed_options_);
  CHECK(!vm_.empty());

  std::vector<UnergisteredOption> result;

  using boost::program_options::basic_option;

  for (const basic_option<charType>& opt
       : parsed_options_->options)
  {
    CHECK(opt.string_key.find_first_of(',')
          == base::StringPiece::npos);
    const bool isOptionSpecified
      = vm_.find(opt.string_key) != vm_.end();
    if (!isOptionSpecified)
    {
      VLOG(9)
        << "found UnergisteredOption with key: "
        << opt.string_key
        << " and values: "
        << opt.value;
      result.push_back(
        UnergisteredOption(
          opt.string_key
          , opt.value
          )
        );
    }
  }

  return
    result;
}

bool BoostCmdParser::init(int argc, char* argv[])
{
  DCHECK(argc > 0);

  DCHECK(!desc_.options().empty())
    << "you must register some command line options";

  static const int default_po_style
    = po::command_line_style::default_style
      // Allow "--long_name" style.
      | po::command_line_style::allow_long
      // Allow "-<single character" style.
      | po::command_line_style::allow_short
      // Ignore the difference in case for all options.
      | po::command_line_style::case_insensitive
      // Allow "-" in short options.
      | po::command_line_style::allow_dash_for_short
      // Allow long options with single option starting character, e.g -foo=10
      | po::command_line_style::allow_long_disguise;

  try {
    // see https://theboostcpplibraries.com/boost.program_options
    parsed_options_ =
      std::make_unique<po::basic_parsed_options<char> >(
        po::command_line_parser(argc, argv).
          options(desc_)
        // see https://www.boost.org/doc/libs/1_73_0/doc/html/boost/program_options/command_line_style/style_t.html
          .style(default_po_style)
          .allow_unregistered()
          .run()
        );

    po::store(*parsed_options_, vm_);

    po::notify(vm_);

    DCHECK(!vm_.empty());
  }
  catch(std::exception& e) {
    // log all command-line arguments
    {
      LOG(ERROR)
        << "app arguments:";
      for(int i = 0; i < argc; ++i) {
        LOG(ERROR)
            << " "
            << argv[i]
            << " ";
      }
    }
    LOG(ERROR)
        << "ERROR: "
        << e.what();
    return
      false;
  }
  catch(...) {
    LOG(ERROR) << "ERROR: Exception of unknown type!";
    return
      false;
  }

  return
    true;
}

int cmdKeyToInt(
  const base::StringPiece &key,
  cmd::BoostCmdParser& boostCmdParser)
{
  CHECK(!key.empty());

  int result = std::numeric_limits<int>::max();

  CHECK(key.find_first_of(',')
        == base::StringPiece::npos);
  if(!boostCmdParser.count(key)) {
    VLOG(9)
        << "Unable to find command-line argument: "
        << key;
    return
      result;
  }

  const boost::optional<int>& value
    = boostCmdParser.getAs<
    boost::optional<int>
    >(key.as_string());

  if(value.is_initialized()) {
    result = value.value();
  } else {
    VLOG_NOT_INITIALIZED(9, key)
  }

  return
    result;
}

base::FilePath getAsPath(
  const base::StringPiece &key,
  cmd::BoostCmdParser& boostCmdParser)
{
  CHECK(!key.empty());

  base::FilePath result{};

  CHECK(key.find_first_of(',')
        == base::StringPiece::npos);
  if(!boostCmdParser.count(key)) {
    VLOG(9)
        << "Unable to find command-line argument: "
        << key;
    return
      result;
  }

  const boost::optional<std::string>& value
    = boostCmdParser.getAs<
    boost::optional<std::string>
    >(key.as_string());

  if(value.is_initialized()
     && !value.value().empty())
  {
    result = base::FilePath{value.value()};
  } else {
    VLOG_NOT_INITIALIZED(9, key)
  }

  return
    result;
}

base::FilePath cmdKeyToDirectory(
  const char key[],
  cmd::BoostCmdParser& boostCmdParser)
{
  base::FilePath dirPath
    = getAsPath(key, boostCmdParser);

  VLOG(9)
    << key
    << " equals to "
    << dirPath;

  if(dirPath.empty()) {
    return
      base::FilePath{};
  }

  /// \note On POSIX, |MakeAbsoluteFilePath| fails
  /// if the path does not exist
  dirPath
    = base::MakeAbsoluteFilePath(dirPath);
  DCHECK(!dirPath.empty())
    << "unable to find absolute path to "
    << dirPath;

  if (!base::PathExists(dirPath)) {
    LOG_PATH_NOT_EXIST(WARNING, dirPath)
    return
      base::FilePath{};
  }

  // we expect dir, NOT file
  if(!base::DirectoryExists(dirPath)) {
    LOG_PATH_NOT_DIRECTORY(WARNING, dirPath)
    return
      base::FilePath{};
  }

  return
    dirPath;
}

base::FilePath cmdKeyToFile(
  const char key[],
  cmd::BoostCmdParser& boostCmdParser)
{
  base::FilePath filePath
    = getAsPath(key, boostCmdParser);

  VLOG(9)
    << key
    << " equals to "
    << filePath;

  if(filePath.empty()) {
    return
      base::FilePath{};
  }

  /// \note On POSIX, |MakeAbsoluteFilePath| fails
  /// if the path does not exist
  filePath = base::MakeAbsoluteFilePath(filePath);
  DCHECK(!filePath.empty())
    << "unable to find absolute path to "
    << filePath;

  if (!base::PathExists(filePath)) {
    LOG_PATH_NOT_EXIST(WARNING, filePath)
    return
      base::FilePath{};
  }

  // we expect file, NOT dir
  if (base::DirectoryExists(filePath)) {
    LOG_PATH_MUST_BE_NOT_DIRECTORY(WARNING, filePath)
    return
      base::FilePath{};
  }

  base::File::Info fileInfo;
  bool hasInfo
    = base::GetFileInfo(filePath, &fileInfo);
  if(!hasInfo) {
    LOG(WARNING)
        << "unable to get source file information: "
        << filePath;
    return
      base::FilePath{};
  }

  return
    filePath;
}

[[nodiscard]] /* do not ignore return value */
std::vector<base::FilePath>
toFilePaths(const std::vector<std::string>& paths)
{
  std::vector<base::FilePath> result;
  for (const std::string& it: paths)
  {
    result.push_back(
      /// \note On POSIX, |MakeAbsoluteFilePath| fails
      /// if the path does not exist
      base::MakeAbsoluteFilePath(base::FilePath{it}));
  }
  return
    result;
}

} // namespace cmd
