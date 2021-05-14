#pragma once

#include <string>
#include <vector>
#include <map>

#include <basis/doctest_util.h>

namespace flexlib {

// EXAMPLE:
// foo(a=1,
//   b ,  c  =  3 )
// RESULT:
// functionArgument{"a", "1"}
// functionArgument{"b", ""}
// functionArgument{"c", "3"}
struct functionArgument {
  std::string name_;
  std::string value_;
};

struct args {
  std::vector<functionArgument> as_vec_;
  std::map<std::string, std::vector<std::string>> as_name_to_value_;
};

struct parsed_func_detail {
  std::string func_name_;
  //std::string func_name_normalized_; // no need to remove ws, we ignored all ws
  args args_;
};

struct parsed_func {
  std::string func_with_args_as_string_;
  parsed_func_detail parsed_func_;
};

functionArgument extract_func_arg(std::string const& inStr);

std::vector<parsed_func> split_to_funcs(std::string const& inStr);

} // namespace flexlib

// DISABLE_DOCTEST: custom macro
#if !defined(DISABLE_DOCTEST)

DOCTEST_TEST_SUITE("extract_func_arg") {
  using namespace flexlib;

  DOCTEST_TEST_CASE("extract_func_arg 1") {
    functionArgument arg_parsed
      = extract_func_arg(R"raw(a=1)raw");
    DOCTEST_CHECK(arg_parsed.name_ == "a");
    DOCTEST_CHECK(arg_parsed.value_ == "1");
  }
  DOCTEST_TEST_CASE("extract_func_arg 2") {
    functionArgument arg_parsed
      = extract_func_arg(R"raw(b)raw");
    DOCTEST_CHECK(arg_parsed.name_.empty());
    DOCTEST_CHECK(arg_parsed.value_ == "b");
  }
  DOCTEST_TEST_CASE("extract_func_arg 3") {
    functionArgument arg_parsed
      = extract_func_arg(R"raw(
        c  =  3)raw");
    /// \note will contain spaces (' ')
    /// both in argument name and argument value
    DOCTEST_CHECK(arg_parsed.name_ == R"raw(
        c  )raw");
    DOCTEST_CHECK(arg_parsed.value_ == "  3");
  }
  DOCTEST_TEST_CASE("extract_func_arg 4") {
    functionArgument arg_parsed
      = extract_func_arg(R"raw(a=1, , ,
      asd , db = "32"
      , hh = 3)raw");
    DOCTEST_CHECK(arg_parsed.name_ == "a");
    /// \note will parse only first argument
    /// (just splits string by first '=')
    DOCTEST_CHECK(arg_parsed.value_ == R"raw(1, , ,
      asd , db = "32"
      , hh = 3)raw");
  }
}
#endif // DISABLE_DOCTEST
