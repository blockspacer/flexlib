﻿#pragma once

#if defined(CLING_IS_ON)

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <condition_variable>

#include <base/macros.h>
#include <base/callback_forward.h>
#include <base/logging.h>
#include <base/memory/ref_counted.h>
#include <base/sequence_checker.h>
#include <base/threading/sequenced_task_runner_handle.h>
#include <base/time/default_tick_clock.h>
#include <base/time/time.h>
#include <base/trace_event/memory_dump_provider.h>
#include <base/trace_event/memory_usage_estimator.h>
#include <base/compiler_specific.h>
#include <base/synchronization/atomic_flag.h>

#include <cling/Interpreter/Interpreter.h>
#include <cling/Interpreter/Value.h>
#include "cling/Interpreter/CIFactory.h"
#include "cling/Interpreter/Interpreter.h"
#include "cling/Interpreter/InterpreterCallbacks.h"
//#include "cling/Interpreter/IncrementalExecutor.h"
//#include "cling/Interpreter/IncrementalParser.h"
#include "cling/Interpreter/Transaction.h"
#include "cling/Interpreter/Value.h"
#include "cling/Interpreter/CValuePrinter.h"
#include "cling/MetaProcessor/MetaProcessor.h"
#include <cling/Utils/Casting.h>
#include "cling/Interpreter/LookupHelper.h"
#include "cling/Utils/AST.h"
#include <cling/Interpreter/Interpreter.h>
#include <cling/Interpreter/Value.h>
#include "cling/Interpreter/CIFactory.h"
#include "cling/Interpreter/Interpreter.h"
#include "cling/Interpreter/InterpreterCallbacks.h"
#include "cling/Interpreter/Transaction.h"
#include "cling/Interpreter/Value.h"
#include "cling/Interpreter/CValuePrinter.h"
#include "cling/MetaProcessor/MetaProcessor.h"
#include <cling/Utils/Casting.h>
#include "cling/Interpreter/LookupHelper.h"
#include "cling/Utils/AST.h"

#include "utils.hpp"

namespace cling_utils {

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

//class InterpreterModule {
//public:
//    InterpreterModule(const std::string& id,
//      const std::vector<std::string>& moduleFiles);
//
//    ~InterpreterModule();
//
//    void createInterpreter();
//
//    void prepare();
//
//    void run();
//
//public: // TODO
//
//    std::string id_;
//    std::vector<std::string> moduleFiles_;
//    std::unique_ptr<cling::Interpreter> interpreter_;
//    std::unique_ptr<cling::MetaProcessor> metaProcessor_;
//
//    static std::shared_ptr<cxxctp::utils::DispatchQueue>
//      receivedMessagesQueue_;
//
//    /// \note module loading order is important
//    static std::map<std::string, std::vector<std::string>> moduleToSources;
//
//    //static cling::MetaProcessor* m_metaProcessor1 = nullptr;
//    static std::map<std::string, std::unique_ptr<InterpreterModule>>
//      interpMap;
//
//    static std::mutex clingReadyMutex;
//    static std::condition_variable clingReadyCV;
//    static bool isClingReady;
//
//    static std::vector<std::string> extra_args;
//};
//
//void reloadClingModule(const std::string& module_id,
//  const std::vector<std::string>& sources);
//
//// NOTE: run under mutex
//void reloadAllCling();
//
//[[ noreturn ]] void cling_func();
//
//void removeClingModule(const std::string& module_id);
//
//void processCode(cling::Interpreter& interp, const std::string& code);
//
//void executeCode(cling::Interpreter& interp, const std::string& code);
//
//void add_default_cling_args(std::vector<std::string> &args);

} // namespace cling_utils

#endif // CLING_IS_ON