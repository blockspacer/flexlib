#pragma once

#include "flexlib/clangUtils.hpp"

#include "flexlib/funcParser.hpp"

#if defined(CLING_IS_ON)
#include "ClingInterpreterModule.hpp"
#endif // CLING_IS_ON

#include "utils.hpp"

#include "flexlib/CXXCTP_STD/CXXCTP_STD.hpp"

#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/AST/ASTContext.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchersMacros.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/ASTConsumers.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>

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

namespace clang_utils {

typedef
  base::RepeatingCallback<
    cxxctp_callback_result(const cxxctp_callback_args& callback_args)
  >
  SourceTransformCallback;

typedef
  base::flat_map<
    std::string
    , SourceTransformCallback
  > SourceTransformRules;

class SourceTransformPipeline {
public:
  SourceTransformPipeline();

  SourceTransformRules sourceTransformRules;

private:

  SEQUENCE_CHECKER(sequence_checker_);

  DISALLOW_COPY_AND_ASSIGN(SourceTransformPipeline);
};


} // namespace clang_utils
