#pragma once

#include "flexlib/clangUtils.hpp"

#include "flexlib/funcParser.hpp"

#if defined(CLING_IS_ON)
#include "ClingInterpreterModule.hpp"
#endif // CLING_IS_ON

#include "utils.hpp"

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

#include <base/macros.h>
#include <base/callback.h>
#include <base/logging.h>
#include <base/containers/flat_map.h>

namespace clang_utils {

struct SourceTransformResult {
  ///\brief may be used to replace orginal code.
  /// To keep orginal code set it as nullptr.
  const char* replacer = nullptr;
};

/**
  * \brief callback that will be called then parser
  *        found custom attribute.
**/
struct SourceTransformOptions {
  /**
    * currently executed function
    * (function name parsed from annotation)
  **/
  const flexlib::parsed_func& func_with_args;

  /**
    * see https://xinhuang.github.io/posts/2015-02-08-clang-tutorial-the-ast-matcher.html
  **/
  const clang::ast_matchers::MatchFinder::MatchResult& matchResult;

  /**
    * see https://devblogs.microsoft.com/cppblog/exploring-clang-tooling-part-3-rewriting-code-with-clang-tidy/
  **/
  clang::Rewriter& rewriter;

  /**
    * found by MatchFinder
    * see https://devblogs.microsoft.com/cppblog/exploring-clang-tooling-part-2-examining-the-clang-ast-with-clang-query/
  **/
  const clang::Decl* decl = nullptr;

  /**
    * All arguments extracted from attribute.
    * Example:
    * $apply(interface, foo_with_args(1, "2"))
    * becomes two `parsed_func` - `interface` and `foo_with_args`.
  **/
  const std::vector<flexlib::parsed_func>& all_func_with_args;
};

typedef
  base::RepeatingCallback<
    SourceTransformResult(const SourceTransformOptions& callback_args)
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
