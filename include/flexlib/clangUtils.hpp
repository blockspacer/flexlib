#pragma once

#include "reflect/ReflectAST.hpp"

#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecordLayout.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Lex/Preprocessor.h>

#include <base/callback_forward.h>

namespace MethodPrinter {

namespace Forwarding {

/// \note change only part after `1 <<`
/// 1 << 0, // 00001 == 1
/// 1 << 1, // 00010 == 2
/// 1 << 2, // 00100 == 4
/// 1 << 3, // 01000 == 8
/// 1 << 4, // 10000 == 16
enum Options
{
  NOTHING = 0
  , EXPLICIT
      = 1 << 1
  , VIRTUAL
      = 1 << 2
  , CONSTEXPR
      = 1 << 3
  , STATIC
      = 1 << 4
  , RETURN_TYPE
      = 1 << 5
  , ALL
      = MethodPrinter::Forwarding::Options::EXPLICIT
        | MethodPrinter::Forwarding::Options::VIRTUAL
        | MethodPrinter::Forwarding::Options::CONSTEXPR
        | MethodPrinter::Forwarding::Options::STATIC
        | MethodPrinter::Forwarding::Options::RETURN_TYPE
};

} // namespace Forwarding

namespace Trailing {

/// \note change only part after `1 <<`
/// 1 << 0, // 00001 == 1
/// 1 << 1, // 00010 == 2
/// 1 << 2, // 00100 == 4
/// 1 << 3, // 01000 == 8
/// 1 << 4, // 10000 == 16
enum Options
{
  NOTHING = 0
  , CONST
      = 1 << 1
  , NOEXCEPT
      = 1 << 2
  , PURE
      = 1 << 3
  , DELETED
      = 1 << 4
  , DEFAULT
      = 1 << 5
  , BODY
      = 1 << 6
  , ALL
      = MethodPrinter::Trailing::Options::CONST
        | MethodPrinter::Trailing::Options::NOEXCEPT
        | MethodPrinter::Trailing::Options::PURE
        | MethodPrinter::Trailing::Options::DELETED
        | MethodPrinter::Trailing::Options::DEFAULT
        | MethodPrinter::Trailing::Options::BODY
};

} // namespace Trailing

} // namespace MethodPrinter

namespace clang_utils {

extern const char kSeparatorWhitespace[];

extern const char kStructPrefix[];

extern const char kRecordPrefix[];

extern const char kSeparatorCommaAndWhitespace[];

enum class StrJoin
{
  KEEP_LAST_SEPARATOR
  // remove ending separator from NOT empty string
  , STRIP_LAST_SEPARATOR
  , TOTAL
};

std::string joinWithSeparator(
  const std::vector<std::string>& input
  , const std::string& separator
  , StrJoin join_logic
);

std::string startHeaderGuard(
  const std::string& guardName);

std::string endHeaderGuard(
  const std::string& guardName);

// we want to generate file names based on parsed C++ types.
// cause file names can not contain spaces ( \n\r)
// and punctuations (,.:') we want to
// replace special characters in filename to '_'
// BEFORE:
//   _typeclass_impl(
//     typeclass_instance(
//       target = "FireSpell",
//       "MagicTemplated<std::string, int>,"
//       "ParentTemplated_1<const char *>,"
//       "ParentTemplated_2<const int &>")
//   )
// AFTER:
//   FireSpell_MagicTemplated_std__string__int__ParentTemplated_1_const_char____ParentTemplated_2_const_int___.typeclass_instance.generated.hpp
void normalizeFileName(std::string &in);

void forEachDeclaredMethod(
  const std::vector<reflection::MethodInfoPtr>& methods
  , const base::RepeatingCallback<
      void(
        const reflection::MethodInfoPtr&
        , size_t)
    >& func);

std::string printMethodDecl(const clang::Decl* decl,
  clang::CXXRecordDecl const * node, clang::CXXMethodDecl* fct);

void expandLocations(clang::SourceLocation& startLoc,
      clang::SourceLocation& endLoc,
      clang::Rewriter& rewriter_);

// This function adapted from clang/lib/ARCMigrate/Transforms.cpp
// Suppose you want to use clang::Rewiter to replace
// int a = 3;  with "", but
// without findSemiAfterLocation you will get ";"
/// \usage
/// {
///   // gets us past the ';'.
///   endLoc = findSemiAfterLocation(endLoc, rewriter);
/// }
/// /// \note if result.replacer is nullptr, than we will keep old code
/// ReplaceText(
///   clang::SourceRange(startLoc, endLoc)
///   , replacement);
clang::SourceLocation findSemiAfterLocation(
  const clang::SourceLocation& loc
  , clang::Rewriter& rewriter);

/// \note prints up to return type
/// (without method name, arguments or body)
/// \note order matters:
/// explicit virtual constexpr static returnType
///   methodName(...) {}
/// \note to disallow some options you can pass
/// something like:
/// (MethodPrinter::Options::ALL
///  & ~MethodPrinter::Options::EXPLICIT
///  & ~MethodPrinter::Options::VIRTUAL)
/// \note to allow only some options you can pass
/// something like:
/// MethodPrinter::Options::NOTHING
/// | MethodPrinter::Options::CONST
/// | MethodPrinter::Options::NOEXCEPT);
std::string printMethodForwarding(
  const reflection::MethodInfoPtr& methodInfo
  , const std::string& separator = kSeparatorWhitespace
  // what method printer is allowed to print
  // |options| is a bitmask of |MethodPrinter::Options|
  , int options = MethodPrinter::Forwarding::Options::ALL
);

/// \note order matters:
/// methodName(...)
/// const noexcept override final [=0] [=deleted] [=default]
/// {}
/// \note to disallow some options you can pass
/// something like:
/// (MethodPrinter::Options::ALL
///  & ~MethodPrinter::Options::EXPLICIT
///  & ~MethodPrinter::Options::VIRTUAL)
/// \note to allow only some options you can pass
/// something like:
/// MethodPrinter::Options::NOTHING
/// | MethodPrinter::Options::CONST
/// | MethodPrinter::Options::NOEXCEPT);
std::string printMethodTrailing(
  const reflection::MethodInfoPtr& methodInfo
  , const std::string& separator = kSeparatorWhitespace
  // what method printer is allowed to print
  // |options| is a bitmask of |MethodPrinter::Trailing::Options|
  , int options = MethodPrinter::Trailing::Options::ALL
);

std::string extractTypeName(
  const std::string& input);

// wrap string into `#include "..."` or `#include <...>`
std::string buildIncludeDirective(
  const std::string& inStr
  , const std::string& quote = R"raw(")raw");

std::string forwardMethodParamNames(
  const std::vector<reflection::MethodParamInfo>& params);

// replaces clang matchResult with |replacement| in source code
void replaceWith(
  clang::Rewriter& rewriter
  , const clang::Decl* decl
  , const clang::ast_matchers::MatchFinder::MatchResult& matchResult
  , const std::string& replacement = ""
  , const bool skip_rewrite_not_main_file = false);

} // namespace clang_utils
