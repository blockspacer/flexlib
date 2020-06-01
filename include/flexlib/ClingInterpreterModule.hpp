#pragma once

#if defined(CLING_IS_ON)

#include "utils.hpp"

#include <base/macros.h>
#include <base/callback.h>
#include <base/logging.h>

#include <cling/Interpreter/Interpreter.h>
#include <cling/Interpreter/Value.h>
#include <cling/Interpreter/InterpreterCallbacks.h>
#include <cling/MetaProcessor/MetaProcessor.h>

#include <string>
#include <vector>
#include <memory>

namespace cling_utils {

/// \note requires to wrap all arguments into struct or class
/// and cast them into |void*|
/// \note pass as |codeToCastArgumentFromVoid| string similar to:
/// *(const originalType*)
/// That way you will cast |void*| to |originalType|
/// Don't forget to |#include| header with |originalType|
/// declaration in file loaded by |loadFile|. It must contain
/// function that you want to call
std::string
  passCppPointerIntoInterpreter(
    void* argumentAsVoid
    , const std::string& codeToCastArgumentFromVoid);

class ClingInterpreter {
public:
  ClingInterpreter(const std::string& debug_id
                   , const std::vector<std::string>& interpreterArgs
                   , const std::vector<std::string>& includePaths);

  // Allows to load files by path like:
  // "../resources/cxtpl/CXTPL_STD.cpp",
  // "../resources/ctp_scripts/app_loop.cpp"
  cling::Interpreter::CompilationResult
    loadFile(const std::string& filePath);

  // |cling::Value| Will hold the result of the expression evaluation.
  cling::Interpreter::CompilationResult
    processCodeWithResult(
      const std::string& code
      , cling::Value& result);

  cling::Interpreter::CompilationResult
    executeCodeNoResult(const std::string& code);

  /// \note requires to wrap all arguments into struct or class
  /// and cast them into |void*|
  /// \note pass as |codeToCastArgumentFromVoid| string similar to:
  /// *(const originalType*)
  /// That way you will cast |void*| to |originalType|
  /// Don't forget to |#include| header with |originalType|
  /// declaration in file loaded by |loadFile|. It must contain
  /// function that you want to call
  /// \note use can use |processCodeWithResult| to call function
  /// without arguments
  cling::Interpreter::CompilationResult
    callFunctionByName(
      /// \note function name can have namespace
      /// example: "my_plugin::loadSettings"
      const std::string& funcName
      , void* argumentAsVoid
      , const std::string& codeToCastArgumentFromVoid
      , cling::Value& result);

private:
  std::string debug_id_;

  std::unique_ptr<cling::Interpreter> interpreter_;

  std::unique_ptr<cling::MetaProcessor> metaProcessor_;

  SEQUENCE_CHECKER(sequence_checker_);

  DISALLOW_COPY_AND_ASSIGN(ClingInterpreter);
};

} // namespace cling_utils

#endif // CLING_IS_ON
