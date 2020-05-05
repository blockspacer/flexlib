#include "flexlib/matchers/annotation_matcher.hpp" // IWYU pragma: associated

#include "flexlib/clangUtils.hpp"

#if __has_include(<filesystem>)
#include <filesystem>
#else
#include <experimental/filesystem>
#endif // __has_include

#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/AST/ASTContext.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchersMacros.h>
#include <clang/AST/Type.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Sema/Sema.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Sema/Sema.h>
#include <clang/Lex/Lexer.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/ASTConsumers.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Driver/Options.h>
#include <clang/AST/AST.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/ASTConsumers.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Sema/Sema.h>
#include <clang/Lex/Lexer.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/ASTConsumers.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Driver/Options.h>
#include <clang/AST/AST.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/ASTConsumers.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>

#include <base/logging.h>

#if __has_include(<filesystem>)
namespace fs = std::filesystem;
#else
namespace fs = std::experimental::filesystem;
#endif // __has_include

namespace clang_utils {

AnnotationMatchOptions::AnnotationMatchOptions(
  std::string annotateName
  , AnnotationMatchCallback&& annotationMatchCallback
  , EndSourceFileActionCallback&& endSourceFileAction)
  : annotateName(annotateName)
  , annotationMatchCallback(std::move(annotationMatchCallback))
  , endSourceFileAction(std::move(endSourceFileAction))
{}

AnnotateMatchCallback::AnnotateMatchCallback(
  clang::Rewriter &rewriter
  , scoped_refptr<AnnotationMatchOptions> annotateOptions)
  : rewriter_(rewriter)
  , annotateOptions_(annotateOptions)
{
  DETACH_FROM_SEQUENCE(sequence_checker_);
}

void AnnotateMatchCallback::run(
  const MatchResult& matchResult)
{
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  DCHECK(annotateOptions_);

  DVLOG(9)
    << "AnnotateMatchCallback::run...";

  const clang::Decl* nodeDecl
    = matchResult.Nodes.getNodeAs<clang::Decl>(
      annotateOptions_->annotateName);
  if (!nodeDecl || nodeDecl->isInvalidDecl()) {
    DVLOG(10)
      << "skipped nodeDecl for "
      << annotateOptions_->annotateName;
    return;
  }

  // When there is a #include <vector> in the source file,
  // our find-decl will print out all the declarations
  // in that included file, because these included files are parsed
  // and consumed as a whole with our source file.
  // To fix this, we need to check if the declarations
  // are defined in our source file
  {
    clang::SourceManager& SM = rewriter_.getSourceMgr();
    const clang::FileID& mainFileID = SM.getMainFileID();
    const auto& FileID = SM.getFileID(nodeDecl->getLocation());
    if (FileID != mainFileID) {
      DVLOG(10)
        << "skipped decl in included file: "
        << nodeDecl->getLocation().printToString(SM).substr(0, 1000)
        << " for annotation name: "
        << annotateOptions_->annotateName;
      return;
    }
  }

  clang::AnnotateAttr* annotateAttr
    = nodeDecl->getAttr<clang::AnnotateAttr>();
  if (!annotateAttr) {
    DVLOG(10)
      << "skipped getAttr for "
      << annotateOptions_->annotateName;
    return;
  }

  DVLOG(9)
    << "found annotation: "
    << annotateAttr->getAnnotation().str();

  annotateOptions_->annotationMatchCallback.Run(
    annotateAttr, matchResult, rewriter_, nodeDecl);
}

AnnotateConsumer::AnnotateConsumer(
  clang::Rewriter& rewriter
  , scoped_refptr<AnnotationMatchOptions> annotateOptions)
  : annotateMatchCallback_(rewriter, annotateOptions)
  , annotateOptions_(annotateOptions)
{
  DETACH_FROM_SEQUENCE(sequence_checker_);

  DCHECK(annotateOptions_);

  using namespace clang::ast_matchers;

  auto hasAnnotateMatcher
    = clang::ast_matchers::hasAttr(clang::attr::Annotate);

  //In Clang, there are two basic types of AST classes:
  // Decl and Stmt, which have many subclasses
  // that covers all the AST nodes we will meet in a source file.
  auto finderMatcher
    = clang::ast_matchers::decl(hasAnnotateMatcher)
      .bind(annotateOptions->annotateName);

  matchFinder.addMatcher(finderMatcher, &annotateMatchCallback_);
}

void AnnotateConsumer::HandleTranslationUnit(
  clang::ASTContext &Context)
{
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  DVLOG(9)
    << "Started AST matcher...";

  matchFinder.matchAST(Context);
}

AnnotationMatchAction::AnnotationMatchAction(
  scoped_refptr<AnnotationMatchOptions> annotateOptions)
  : annotateOptions_(annotateOptions)
{
  DETACH_FROM_SEQUENCE(sequence_checker_);

  DCHECK(annotateOptions_);
}

AnnotationMatchAction::ASTConsumerPointer
  AnnotationMatchAction::CreateASTConsumer(
    clang::CompilerInstance& compilerInstance
    , StringRef filename)
{
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  ignore_result(filename);

  DVLOG(9)
    << "Created AST consumer...";

  rewriter_.setSourceMgr(
    compilerInstance.getSourceManager()
    , compilerInstance.getLangOpts());

  DCHECK(annotateOptions_);
  return std::make_unique<AnnotateConsumer>(
    rewriter_, annotateOptions_);
}

bool AnnotationMatchAction::BeginSourceFileAction(
  clang::CompilerInstance&)
{
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  DVLOG(9)
    << "Processing file: " << getCurrentFile().str();
  return true;
}

void AnnotationMatchAction::EndSourceFileAction()
{
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  ASTFrontendAction::EndSourceFileAction();

  clang::SourceManager& SM = rewriter_.getSourceMgr();

  const clang::FileID& mainFileID = SM.getMainFileID();

  const clang::FileEntry* fileEntry
    = SM.getFileEntryForID(mainFileID);
  if(!fileEntry) {
    NOTREACHED();
  }

  DCHECK(annotateOptions_);
  annotateOptions_->endSourceFileAction.Run(
    mainFileID, fileEntry, rewriter_);
}

AnnotationMatchFactory::AnnotationMatchFactory(
  scoped_refptr<AnnotationMatchOptions> annotateOptions)
  : FrontendActionFactory()
  , annotateOptions_(annotateOptions)
{
  DETACH_FROM_SEQUENCE(sequence_checker_);

  DCHECK(annotateOptions_);
}

clang::FrontendAction*
  AnnotationMatchFactory::create()
{
  DETACH_FROM_SEQUENCE(sequence_checker_);

  DCHECK(annotateOptions_);
  return new AnnotationMatchAction(annotateOptions_);
}

} // namespace clang_utils
