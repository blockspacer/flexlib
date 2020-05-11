#pragma once

#include <string>
#include <vector>
#include <map>

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
