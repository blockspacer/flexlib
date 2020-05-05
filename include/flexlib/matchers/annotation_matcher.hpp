#pragma once

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

using MatchResult
  = clang::ast_matchers::MatchFinder::MatchResult;

typedef
  base::RepeatingCallback<
    void(clang::AnnotateAttr*
         , const MatchResult&
         , clang::Rewriter&
         , const clang::Decl* nodeDecl)
  > AnnotationMatchCallback;

typedef
  base::RepeatingCallback<
    void(const clang::FileID&
         , const clang::FileEntry*
         , clang::Rewriter&)
  > EndSourceFileActionCallback;

class AnnotationMatchOptions
  : public base::RefCountedThreadSafe<AnnotationMatchOptions>
{
 public:
  AnnotationMatchOptions(
    std::string annotateName
    , AnnotationMatchCallback&& annotationMatchCallback
    , EndSourceFileActionCallback&& endSourceFileAction);

  // name of |clang::AnnotateAttr| to find
  std::string annotateName;

  // may be used to rewrite matched clang declaration
  AnnotationMatchCallback annotationMatchCallback;

  // may be used to save result after clang-rewrite
  EndSourceFileActionCallback endSourceFileAction;

private:
 friend class base::RefCountedThreadSafe<AnnotationMatchOptions>;
 ~AnnotationMatchOptions() = default;

  DISALLOW_COPY_AND_ASSIGN(AnnotationMatchOptions);
};

// Called when the |Match| registered for |clang::AnnotateAttr|
// was successfully found in the AST.
class AnnotateMatchCallback
  : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
  AnnotateMatchCallback(
    clang::Rewriter &rewriter
    , scoped_refptr<AnnotationMatchOptions> annotateOptions);

  void run(const MatchResult& Result) override;

private:
  clang::Rewriter& rewriter_;

  scoped_refptr<AnnotationMatchOptions> annotateOptions_;

  SEQUENCE_CHECKER(sequence_checker_);

  DISALLOW_COPY_AND_ASSIGN(AnnotateMatchCallback);
};

// The ASTConsumer will read AST.
// It provides many interfaces to be overridden when
// certain type of AST node has been parsed,
// or after all the translation unit has been parsed.
class AnnotateConsumer
  : public clang::ASTConsumer
{
public:
  explicit AnnotateConsumer(
    clang::Rewriter &Rewriter
    , scoped_refptr<AnnotationMatchOptions> annotateOptions);

  ~AnnotateConsumer() override = default;

  // HandleTranslationUnit() called only after
  // the entire source file is parsed.
  // Translation unit effectively represents an entire source file.
  void HandleTranslationUnit(clang::ASTContext &Context) override;

private:
  clang::ast_matchers::MatchFinder matchFinder;

  AnnotateMatchCallback annotateMatchCallback_;

  scoped_refptr<AnnotationMatchOptions> annotateOptions_;

  SEQUENCE_CHECKER(sequence_checker_);

  DISALLOW_COPY_AND_ASSIGN(AnnotateConsumer);
};

// We choose an ASTFrontendAction because we want to analyze
// the AST representation of the source code
class AnnotationMatchAction
  : public clang::ASTFrontendAction
{
public:
  using ASTConsumerPointer = std::unique_ptr<clang::ASTConsumer>;

  explicit AnnotationMatchAction(
    scoped_refptr<AnnotationMatchOptions> annotateOptions);

  ASTConsumerPointer CreateASTConsumer(
    // pass a pointer to the CompilerInstance because
    // it contains a lot of contextual information
    clang::CompilerInstance&
    , llvm::StringRef filename) override;

  bool BeginSourceFileAction(
    // pass a pointer to the CompilerInstance because
    // it contains a lot of contextual information
    clang::CompilerInstance&) override;

  void EndSourceFileAction() override;

private:
  // Rewriter lets you make textual changes to the source code
  clang::Rewriter rewriter_;

  scoped_refptr<AnnotationMatchOptions> annotateOptions_;

  SEQUENCE_CHECKER(sequence_checker_);

  DISALLOW_COPY_AND_ASSIGN(AnnotationMatchAction);
};

// frontend action will only consume AST and find all declarations
struct AnnotationMatchFactory
  : public clang::tooling::FrontendActionFactory
{
  AnnotationMatchFactory(
    scoped_refptr<AnnotationMatchOptions> annotateOptions);

  clang::FrontendAction* create() override;

private:
  scoped_refptr<AnnotationMatchOptions> annotateOptions_;

  SEQUENCE_CHECKER(sequence_checker_);

  DISALLOW_COPY_AND_ASSIGN(AnnotationMatchFactory);
};

} // namespace clang_utils
