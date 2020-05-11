#pragma once

#include "flexlib/matchers/annotation_matcher.hpp"

#include <string>

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

namespace flexlib {

typedef
  base::RepeatingCallback<
    void(const std::string& processedAnnotaion
      , clang::AnnotateAttr*
      , const clang_utils::MatchResult&
      , clang::Rewriter&
      , const clang::Decl*)
  >
  AnnotationMethodCallback;

typedef
  base::flat_map<
    std::string
    , AnnotationMethodCallback
  > AnnotationMethods;

class AnnotationParser {
public:
  AnnotationParser(
    AnnotationMethods* supportedAnnotationMethods);

  bool tryRemovePrefix(
    const std::string& unprocessedAnnotation
    , std::string& resultWithoutPrefix
    , const std::string& prefix);

  AnnotationMethods::const_iterator parseToMethods(
    const std::string& unprocessedAnnotation
    , std::string& resultWithoutMethod);

private:
  AnnotationMethods* annotationMethods;

  SEQUENCE_CHECKER(sequence_checker_);

  DISALLOW_COPY_AND_ASSIGN(AnnotationParser);
};

} // namespace flexlib
