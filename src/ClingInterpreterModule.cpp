#include "flexlib/ClingInterpreterModule.hpp" // IWYU pragma: associated

#include <base/check.h>
#include <base/run_loop.h>
#include "base/files/file.h"
#include "base/files/file_util.h"
#include "base/base_paths.h"
#include "basis/path_provider.h"
#include "base/path_service.h"
/// \todo use boost outcome for error reporting
#include <base/logging.h>

#if defined(CLING_IS_ON)

#include "flexlib/options/ctp/options.hpp"

#include <basic/multiconfig/multiconfig.h>

#include <llvm/Support/raw_ostream.h>

namespace cling_utils {

std::string
  passCppPointerIntoInterpreter(
    void* argumentAsVoid
    , const std::string& codeToCastArgumentFromVoid)
{
  /// \note may be called on any sequence!
  std::ostringstream code_str;
  {
    code_str
      << codeToCastArgumentFromVoid << "("
      // Pass a pointer into cling as a string.
      << std::hex << std::showbase
      << reinterpret_cast<size_t>(argumentAsVoid) << ')';
  }
  return code_str.str();
}

ClingInterpreter::ClingInterpreter(
  const std::string& debug_id
  , const std::vector<std::string>& interpreterArgs
  , const std::vector<std::string>& includePaths)
  : debug_id_(debug_id)
{
  DETACH_FROM_SEQUENCE(sequence_checker_);

  CHECK(!interpreterArgs.empty())
    << "You must provide at least one argument"
       " to Cling interpreter";

  DCHECK(!debug_id_.empty());

  std::vector<
    const char* /// \note must manage pointer lifetime
  > args;

  std::transform(
    interpreterArgs.begin()
    , interpreterArgs.end()
    , std::back_inserter(args)
    , [](const std::string& value)
    {
        VLOG(9)
          << "added command-line argument for Cling interpreter: "
          << value;
        DCHECK(!value.empty());
        /// \note must manage pointer lifetime
        return value.c_str();
    });

  CHECK(!args.empty())
    << "You must provide at least one argument"
       " to Cling interpreter";

  ::base::FilePath file_exe_{};

  if (!base::PathService::Get(base::FILE_EXE, &file_exe_)) {
    NOTREACHED();
    // stop app execution with EXIT_FAILURE
    return;
  }

  /// \note returns empty string if the path is not ASCII.
  const std::string maybe_base_exe_name_
    = file_exe_.BaseName().RemoveExtension().MaybeAsASCII();

  MULTICONF_String(llvm_dir
    , /* default value */ LLVMDIR
    , BUILTIN_MULTICONF_LOADERS
    , /* name of configuration group */ maybe_base_exe_name_);

  /// \note will cache configuration values,
  /// so use `resetAndReload` if you need to update configuration values.
  CHECK_OK(basic::MultiConf::GetInstance().resetAndReload())
    << "Wrong configuration.";

  /// \todo replace resetAndReload with reloadOptionWithName
  ///CHECK_OK(basic::MultiConf::GetInstance().reloadOptionWithName("llvm_dir"
  ///  , /* name of configuration group */ maybe_base_exe_name_))
  ///  << "Wrong configuration.";

  /// \note required to refresh configuration cache
  base::RunLoop().RunUntilIdle();

  {
    const std::string& llvm_dir_str = llvm_dir.GetValue();
    LOG(INFO)
      << "You can"
      << (llvm_dir_str.empty() ? " set" : " change")
      << " path to LLVM folder: "
      << llvm_dir.optionFormatted()
      << (llvm_dir_str.empty() ? "" : " Using path to llvm: ")
      << llvm_dir_str;

    interpreter_
      = std::make_unique<cling::Interpreter>(
          args.size()
          , &(args[0])
          , llvm_dir_str.c_str());
  }

  for(const std::string& it: includePaths) {
    interpreter_->AddIncludePath(it.c_str());
  }

  interpreter_->enableDynamicLookup(true);

  metaProcessor_
    = std::make_unique<cling::MetaProcessor>(
        *interpreter_, llvm::outs());

  {
    cling::Interpreter::CompilationResult compilationResult
      = interpreter_->process("#define CLING_IS_ON 1");
    DCHECK(compilationResult
           == cling::Interpreter::Interpreter::kSuccess);
  }
}

cling::Interpreter::CompilationResult
  ClingInterpreter::loadFile(
    const std::string& filePath)
{
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  DCHECK(!filePath.empty());

  cling::Interpreter::CompilationResult compilationResult;

  LOG(INFO)
    << "started C++ file loading using Cling...";

  {
    DCHECK(metaProcessor_);

    int res = metaProcessor_->process(
      // input_line - the user input
      ".L " + filePath
      // compRes - whether compilation was successful
      , compilationResult
      // result - the cling::Value as result
      // of the execution of the last statement
      , nullptr
      // disableValuePrinting - whether to suppress echoing of the
      // expression result
      , true);

    ///\returns 0 on success or the indentation of the next input line should
    /// have in case of multi input mode.
    ///\returns -1 if quit was requiested.
    DCHECK(res != -1)
      << "quit was requiested by cling MetaProcessor";
  }

  if(compilationResult != cling::Interpreter::Interpreter::kSuccess)
  {
    DVLOG(9)
      << "ERROR in cling interpreter: "
      << debug_id_
      << " ERROR while running cling code in path: "
      << filePath.substr(0, 10000);
  }

  return compilationResult;
}

cling::Interpreter::CompilationResult
  ClingInterpreter::processCodeWithResult(
    const std::string& code
    , cling::Value& result)
{
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  DCHECK(!code.empty());

  LOG(INFO)
    << "started C++ code processing using Cling: "
    << code.substr(0, 10000);

  cling::Interpreter::CompilationResult compilationResult;

  {
    DCHECK(interpreter_);

    ///\see https://github.com/root-project/cling/blob/master/include/cling/Interpreter/Interpreter.h
    ///
    ///\brief Compiles the given input.
    ///
    /// This interface helps to run everything that cling can run. From
    /// declaring header files to running or evaluating single statements.
    /// Note that this should be used when there is no idea of what kind of
    /// input is going to be processed. Otherwise if is known, for example
    /// only header files are going to be processed it is much faster to run the
    /// specific interface for doing that - in the particular case - declare().
    compilationResult
      = interpreter_->process(
          code.c_str()
          , &result);
  }

  if(compilationResult != cling::Interpreter::Interpreter::kSuccess)
  {
    DVLOG(9)
      << "ERROR in cling interpreter: "
      << debug_id_
      << " ERROR while running cling code: "
      << code.substr(0, 10000);
  }

  return compilationResult;
}

cling::Interpreter::CompilationResult
  ClingInterpreter::callFunctionByName(
    const std::string& funcName
    , void* argumentAsVoid
    , const std::string& codeToCastArgumentFromVoid
    , cling::Value& result)
{
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  LOG(INFO)
    << "started C++ function execution using Cling...";

  cling::Interpreter::CompilationResult compilationResult;

  std::ostringstream code_str;

  {
    // scope begin
    code_str << "[](){";
    code_str << "return ";
    // func begin
    code_str << funcName << "( ";
    // func arguments
    code_str << passCppPointerIntoInterpreter(
      argumentAsVoid
      // Pass a pointer into cling as a string.
      , codeToCastArgumentFromVoid
    );
    // func end
    code_str << " );" << ";";
    // scope end
    code_str << "}();";
  }

  {
    DCHECK(interpreter_);

    ///\see https://github.com/root-project/cling/blob/master/include/cling/Interpreter/Interpreter.h
    ///
    ///\brief Compiles the given input.
    ///
    /// This interface helps to run everything that cling can run. From
    /// declaring header files to running or evaluating single statements.
    /// Note that this should be used when there is no idea of what kind of
    /// input is going to be processed. Otherwise if is known, for example
    /// only header files are going to be processed it is much faster to run the
    /// specific interface for doing that - in the particular case - declare().
    compilationResult
      = processCodeWithResult(
          code_str.str().c_str()
          , result);
  }

  if(compilationResult != cling::Interpreter::Interpreter::kSuccess)
  {
    DVLOG(9)
      << "ERROR in cling interpreter: "
      << debug_id_
      << " ERROR while running cling code: "
      << code_str.str().substr(0, 10000);
  }

  return compilationResult;
}

cling::Interpreter::CompilationResult
  ClingInterpreter::executeCodeNoResult(
    const std::string& code)
{
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  DCHECK(!code.empty());

  LOG(INFO)
    << "started C++ code execution using Cling...";

  cling::Interpreter::CompilationResult compilationResult;

  {
    DCHECK(interpreter_);

    ///\see https://github.com/root-project/cling/blob/master/include/cling/Interpreter/Interpreter.h
    ///
    ///\brief Compiles input line and runs.
    ///
    /// The interface is the fastest way to compile and run a statement or
    /// expression. It just wraps the input into a function definition and runs
    /// that function, without any other "magic".
    compilationResult
      = interpreter_->execute(
          code.c_str());
  }

  if(compilationResult != cling::Interpreter::Interpreter::kSuccess)
  {
    DVLOG(9)
      << "ERROR in cling interpreter: "
      << debug_id_
      << " ERROR while running cling code: "
      << code.substr(0, 10000);
  }

  return compilationResult;
}

} // namespace cling_utils

#endif // CLING_IS_ON
