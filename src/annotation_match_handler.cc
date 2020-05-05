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

namespace cxxctp {

AnnotationMatchHandler::AnnotationMatchHandler(
  AnnotationParser* annotationParser
  , AnnotationMethods* supportedAnnotationMethods)
  : annotationParser_(annotationParser)
  , annotationMethods(supportedAnnotationMethods)
{
  DETACH_FROM_SEQUENCE(sequence_checker_);

  DCHECK(annotationMethods);
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

  DLOG(INFO) << "annotateAttr->getAnnotation()"
               << annotateAttr->getAnnotation().str();

  std::string resultWithoutMethod;
  DCHECK(annotationParser_);
  AnnotationMethods::const_iterator callback_iter
    = annotationParser_->parseToMethods(
        annotateAttr->getAnnotation().str(), resultWithoutMethod);

  if(callback_iter == annotationMethods->end()) {
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

  std::string full_file_path = fileEntry->getName();
  DLOG(INFO) << "full_file_path is " << full_file_path;
  const std::string filename = fs::path(full_file_path).filename();
  DLOG(INFO) << "filename is " << filename;

  DLOG(INFO) << "** EndSourceFileAction for: "
               << fileEntry->getName().str();
  const std::string full_file_ext = fs::path(full_file_path).extension();

  const fs::path out_path = fs::absolute(ctp::Options::res_path
    / (filename + ".generated" + full_file_ext));

  bool shouldFlush = true; // TODO: make optional for some files
  if (shouldFlush) {
      /*const std::string file_ext = full_file_path.substr(
          filename.find_last_of(".") + 1);*/
      if(!full_file_path.empty() && !full_file_ext.empty()) {
          DLOG(INFO) << "full_file_ext = " << full_file_ext;
          //full_file_path.erase(full_file_path.length() - full_file_ext.length(), full_file_ext.length());
          std::error_code error_code;
          llvm::raw_fd_ostream outFile(out_path.string(), error_code, llvm::sys::fs::F_None);
          rewriter.getEditBuffer(fileID).write(outFile);
          outFile.close();
      }
  }
}

} // namespace cxxctp
