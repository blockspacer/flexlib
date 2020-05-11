#include "flexlib/annotation_match_handler.hpp" // IWYU pragma: associated

#include "flexlib/options/ctp/options.hpp"

// __has_include is currently supported by GCC and Clang. However GCC 4.9 may have issues and
// returns 1 for 'defined( __has_include )', while '__has_include' is actually not supported:
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=63662
#if __has_include(<filesystem>)
#include <filesystem>
#else
#include <experimental/filesystem>
#endif // __has_include

#if __has_include(<filesystem>)
namespace fs = std::filesystem;
#else
namespace fs = std::experimental::filesystem;
#endif // __has_include

namespace flexlib {

AnnotationMatchHandler::AnnotationMatchHandler(
  AnnotationParser* annotationParser
  , AnnotationMethods* supportedAnnotationMethods
  , SaveFileHandler&& saveFileHandler)
  : annotationParser_(annotationParser)
  , annotationMethods_(supportedAnnotationMethods)
  , saveFileHandler_(std::move(saveFileHandler))
{
  DETACH_FROM_SEQUENCE(sequence_checker_);

  DCHECK(annotationMethods_);
  DCHECK(annotationParser_);
}

void AnnotationMatchHandler::matchHandler(
  clang::AnnotateAttr* annotateAttr
  , const clang_utils::MatchResult& matchResult
  , clang::Rewriter& rewriter
  , const clang::Decl* nodeDecl)
{
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  DCHECK(nodeDecl && !nodeDecl->isInvalidDecl());
  DCHECK(annotateAttr);

  DLOG(INFO)
    << "found annotation method: "
    << annotateAttr->getAnnotation().str();

  std::string resultWithoutMethod;
  DCHECK(annotationParser_);
  AnnotationMethods::const_iterator callback_iter
    = annotationParser_->parseToMethods(
        annotateAttr->getAnnotation().str(), resultWithoutMethod);

  if(callback_iter == annotationMethods_->end()) {
    LOG(WARNING)
      << "unable to handle unregistered annotation method:"
      << annotateAttr->getAnnotation().str();
    return;
  }

  callback_iter->second.Run(resultWithoutMethod
                            , annotateAttr
                            , matchResult
                            , rewriter
                            , nodeDecl);
}

void AnnotationMatchHandler::endSourceFileHandler(
  const clang::FileID& fileID
  , const clang::FileEntry* fileEntry
  , clang::Rewriter& rewriter)
{
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  CHECK(saveFileHandler_);
  saveFileHandler_.Run(fileID
                  , fileEntry
                  , rewriter);
}

} // namespace flexlib
