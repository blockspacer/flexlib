#pragma once

#include "flexlib/annotation_parser.hpp"
#include "flexlib/matchers/annotation_matcher.hpp"

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

#include <base/bind.h>
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

#include <string>

namespace flexlib {

class AnnotationMatchHandler {
public:
  using SaveFileHandler
    = base::RepeatingCallback<
        void(const clang::FileID& fileID
        , const clang::FileEntry* fileEntry
        , clang::Rewriter& rewriter)
      >;

  AnnotationMatchHandler(
    AnnotationParser* annotationParser
    , AnnotationMethods* supportedAnnotationMethods
    , SaveFileHandler&& saveFileHandler);

  // may be used to rewrite matched clang declaration
  void matchHandler(
    clang::AnnotateAttr*
    , const clang_utils::MatchResult&
    , clang::Rewriter&
    , const clang::Decl*);

  // may be used to save result after clang-rewrite
  void endSourceFileHandler(
    const clang::FileID&
    , const clang::FileEntry*
    , clang::Rewriter&);

private:
  SEQUENCE_CHECKER(sequence_checker_);

  AnnotationParser* annotationParser_;

  AnnotationMethods* annotationMethods_;

  SaveFileHandler saveFileHandler_;

  DISALLOW_COPY_AND_ASSIGN(AnnotationMatchHandler);
};

} // namespace flexlib
