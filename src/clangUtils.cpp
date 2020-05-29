#include "flexlib/clangUtils.hpp" // IWYU pragma: associated

#include <clang/Lex/Lexer.h>

#include <base/logging.h>
#include <base/strings/string_util.h>
#include <base/strings/string16.h>
#include <base/callback.h>

namespace clang_utils {

// extern
const char kSeparatorWhitespace[] = " ";

// extern
const char kStructPrefix[] = "struct ";

// extern
const char kRecordPrefix[] = "record ";

// extern
const char kSeparatorCommaAndWhitespace[] = ", ";

void forEachDeclaredMethod(
  const std::vector<reflection::MethodInfoPtr>& methods
  , const base::RepeatingCallback<
      void(
        const reflection::MethodInfoPtr&
        , size_t)
    >& func
){
  size_t index = 0;
  for(const reflection::MethodInfoPtr& method : methods) {
    func.Run(method, index);
    index++;
  }
}

std::string buildIncludeDirective(
  const std::string& inStr
  , const std::string& quote)
{
  std::string output;

  output += R"raw(#include )raw";

  output += quote;

  output += inStr;

  output += quote;

  return output;
}

std::string startHeaderGuard(
  const std::string& guardName)
{
  if(!guardName.empty()) {
    std::string output;

    output += R"raw(#if !defined()raw";
    output += guardName;
    output += R"raw()
)raw";
    output += R"raw(#define )raw";
    output += guardName;
    output += R"raw(
)raw";

    return output;
  } else {
    return "#pragma once\n";
  } // !guardName.empty()
  return "";
}

std::string endHeaderGuard(const std::string& guardName)
{
  if(!guardName.empty()) {
    std::string output;

    output += R"raw(
#endif // )raw";

    output += guardName;

    output += R"raw(
)raw";

    return output;
  }
  return "";
}

void normalizeFileName(std::string &in)
{
  std::replace_if(in.begin(), in.end(), ::ispunct, '_');
  std::replace_if(in.begin(), in.end(), ::isspace, '_');
}

std::string joinWithSeparator(
  const std::vector<std::string>& input
  , const std::string& separator
  , StrJoin join_logic
){
  DCHECK(!input.empty());
  DCHECK(!separator.empty());

  std::string result;

  for(const std::string& param: input) {
    DCHECK(!param.empty());
    result += param;
    result += separator;
  }

  switch (join_logic) {
    // joins vector<string1, string2> with separator ", "
    // into "string1, string2, " (note ending ", ")
    case StrJoin::KEEP_LAST_SEPARATOR: {
      // skip
      break;
    }
    // joins vector<string1, string2> with separator ", "
    // into "string1, string2"
    case StrJoin::STRIP_LAST_SEPARATOR: {
      if(!result.empty() && !separator.empty()) {
        const std::string::size_type trim_end_pos
          = result.length() - separator.length();
        DCHECK(trim_end_pos > 0);
        result = result.substr(0, trim_end_pos);
      }
      break;
    }

    case StrJoin::TOTAL:
    default: {
      NOTREACHED();
      return "";
    }
  }

  DCHECK(!result.empty())
    << "joinWithSeparator failed";
  return result;
}

std::string printMethodDecl(
  const clang::Decl* decl
  , clang::CXXRecordDecl const * node
  , clang::CXXMethodDecl* fct)
{
  std::string methodDecl;
  clang::FunctionDecl *D = fct->getAsFunction();

  bool isCtor
    = llvm::dyn_cast_or_null<const clang::CXXConstructorDecl>(fct)
      != nullptr;

  bool isDtor
    = llvm::dyn_cast_or_null<const clang::CXXDestructorDecl>(fct)
      != nullptr;

  bool isOperator
    = fct->isOverloadedOperator();

  if(isCtor || isDtor || isOperator) {
    return "";
  }

  ///\todo https://stackoverflow.com/questions/56792760/how-to-get-the-actual-type-of-a-template-typed-class-member-with-clang/56796422#56796422https://stackoverflow.com/questions/56792760/how-to-get-the-actual-type-of-a-template-typed-class-member-with-clang/56796422#56796422
  if(fct->isTemplateDecl()) {
      /*  //FunctionTemplateDecl * FTD = fct->getDescribedFunctionTemplate();
  FunctionTemplateDecl *FTD =
   dyn_cast<FunctionTemplateDecl>(D->getFriendDecl())
if (const FunctionDecl *FD = D->getTemplateSpecializationInfo()platedDecl()) {
 for (unsigned I = 0,
      NumTemplateParams = FD->getNumTemplateParameterLists();
      I < NumTemplateParams; ++I)
printTemplateParameters(FD->getTemplateParameterList(I));
    methodDecl += " template<";
    fct->getTemplateParameterList()
    methodDecl += " template>";
  }*/
  }

  auto ctor = llvm::dyn_cast_or_null<
      const clang::CXXConstructorDecl>(decl);
  if (ctor != nullptr)
  {
    if(ctor->isExplicit()) {
      methodDecl += " explicit ";
    }
  }

  if(fct->isVirtual()) {
    methodDecl += " virtual ";
  }

  if(fct->isConstexpr()) {
    methodDecl += " constexpr ";
  }

  if(fct->isStatic()) {
    methodDecl += " static ";
  }

  methodDecl += " " + fct->getReturnType().getAsString() + " ";

  methodDecl += " __" + fct->getNameAsString() + " ";

  unsigned Indentation = 0;
  clang::LangOptions LO;
  clang::PrintingPolicy PrintPolicy(LO);

  methodDecl += " ( ";
  std::string Proto;
  llvm::raw_string_ostream POut(Proto);

  int pc = 0;
  for (auto param = D->param_begin(); param != D->param_end(); ++param)
  {
    if(param) {
      auto nextparam = param + 1;
      std::string result;
      llvm::raw_string_ostream os(result);
      (*param)->print(os, PrintPolicy);
      methodDecl += result;

      methodDecl += os.str();

      if(nextparam && nextparam != D->param_end()/*pc && pc <= D->getNumParams()*/) {
          methodDecl += ", ";
      }

      pc++;
    }
  }

  methodDecl += " ) ";

  if(fct->isConst()) {
      methodDecl += " const ";
  }

  // see https://github.com/flexferrum/autoprogrammer/blob/db902121dd492a2df2b7287e0dafd7173062bcc7/src/ast_reflector.cpp#L386
  clang::QualType fnQualType = fct->getType();

  const clang::FunctionProtoType* fnProtoType = nullptr;
  if (const clang::FunctionType *fnType =
          fnQualType->getAs<clang::FunctionType>())
  {
    if (D->hasWrittenPrototype()) {
      fnProtoType = llvm::dyn_cast<clang::FunctionProtoType>(fnType);
    }
  }

  // see https://github.com/FunkMonkey/libClang/blob/ab4702febef82409773f7c80ec02d53ddbb4d80e/lib/AST/DeclPrinter.cpp#L468
  if(clang::isNoexceptExceptionSpec(
    fnProtoType->getExceptionSpecType()))
  {
    methodDecl += " noexcept ";
  }

  if(fct->isPure()) {
    methodDecl += " = 0 ";
  }

  if(fct->isDeleted()) {
    methodDecl += " = delete ";
  }

  if(fct->isDefaulted()) {
    methodDecl += " = default ";
  }

  if(fct->isDefined() && fct->hasBody()) {
    std::string Proto;
    llvm::raw_string_ostream Out(Proto);
    fct->getBody()->
        printPretty(Out, 0, PrintPolicy, Indentation);
    methodDecl += Out.str();
  }

  return methodDecl;
}

void expandLocations(clang::SourceLocation& startLoc,
                     clang::SourceLocation& endLoc,
                     clang::Rewriter& rewriter_)
{
  // macros need a special handling, because we are interessted in the macro
  // instanciation location and not in the macro definition location
  clang::SourceManager& sm = rewriter_.getSourceMgr();

  for (size_t i = 0; startLoc.isMacroID(); i++)
  {
    DCHECK(i < std::numeric_limits<size_t>::max());

    // See skipToMacroArgExpansion()
    for (clang::SourceLocation L = startLoc; L.isMacroID();
       L = sm.getImmediateSpellingLoc(L)) {
      if (sm.isMacroArgExpansion(L)) {
        startLoc = L;
        break;
      }
    }

    startLoc =
      sm.isMacroArgExpansion(startLoc)
      // We're just interested in the start location
      ? sm.getImmediateSpellingLoc(startLoc)
      : sm.getImmediateExpansionRange( startLoc ).first;
  }
}

clang::SourceLocation findSemiAfterLocation(
  const clang::SourceLocation& loc
  , clang::Rewriter& rewriter)
{
  clang::SourceLocation result;

  clang::SourceManager &SM = rewriter.getSourceMgr();

  if (loc.isMacroID()) {
    if (!clang::Lexer::isAtEndOfMacroExpansion(
          loc, SM, rewriter.getLangOpts(),
          // If non-null and function returns true, it is set to
          // end location of the macro.
          &result))
      return clang::SourceLocation();
  }

  result
    = clang::Lexer::getLocForEndOfToken(
       loc, /*Offset=*/0, SM, rewriter.getLangOpts());

  // Break down the source location.
  std::pair<clang::FileID, unsigned> locInfo
    = SM.getDecomposedLoc(result);

  // Try to load the file buffer.
  bool invalidTemp = false;
  llvm::StringRef file
    = SM.getBufferData(locInfo.first, &invalidTemp);
  if (invalidTemp) {
    return clang::SourceLocation();
  }

  const char* tokenBegin
    = file.data() + locInfo.second;

  // Lex from the start of the given location.
  clang::Lexer lexer(
    SM.getLocForStartOfFile(locInfo.first),
    rewriter.getLangOpts(),
    file.begin(),
    tokenBegin,
    file.end());

  clang::Token tok;
  lexer.LexFromRawLexer(tok);
  if (tok.isNot(clang::tok::semi)) {
    return clang::SourceLocation();
  }

  return tok.getLocation().isInvalid()
    ? result
    : tok.getLocation();
}

std::string printMethodForwarding(
  const reflection::MethodInfoPtr& methodInfo
  , const std::string& separator
  // what method printer is allowed to print
  // |options| is a bitmask of |MethodPrinter::Options|
  , int options
){
  DCHECK(methodInfo);

  std::string result;

  const bool allowExplicit
    = (options & MethodPrinter::Forwarding::Options::EXPLICIT);

  const bool allowVirtual
    = (options & MethodPrinter::Forwarding::Options::VIRTUAL);

  const bool allowConstexpr
    = (options & MethodPrinter::Forwarding::Options::CONSTEXPR);

  const bool allowStatic
    = (options & MethodPrinter::Forwarding::Options::STATIC);

  const bool allowReturnType
    = (options & MethodPrinter::Forwarding::Options::RETURN_TYPE);

  if(allowExplicit
     && methodInfo->isExplicitCtor)
  {
    result += "explicit";
    result += separator;
  } else {
    DVLOG(20)
      << (methodInfo->isExplicitCtor
          ? "method is explicit, "
          "but explicit is not printed "
          "due to provided options"
          : "method is not explicit");
  }

  if(allowVirtual
     && methodInfo->isVirtual)
  {
    result += "virtual";
    result += separator;
  } else {
    DVLOG(20)
      << (methodInfo->isVirtual
          ? "method is virtual, "
          "but virtual is not printed "
          "due to provided options"
          : "method is not virtual");
  }

  if(allowConstexpr
     && methodInfo->isConstexpr)
  {
    result += "constexpr";
    result += separator;
  } else {
    DVLOG(20)
      << (methodInfo->isConstexpr
          ? "method is constexpr, "
          "but constexpr is not printed "
          "due to provided options"
          : "method is not constexpr");
  }

  if(allowStatic
     && methodInfo->isStatic)
  {
    result += "static";
    result += separator;
  } else {
    DVLOG(20)
      << (methodInfo->isStatic
          ? "method is static, "
          "but static is not printed "
          "due to provided options"
          : "method is not static");
  }

  DCHECK(allowReturnType
         ? !methodInfo->isCtor
         : true)
    << "constructor can not have return type, method: "
    << methodInfo->name;

  DCHECK(allowReturnType
         ? !methodInfo->isDtor
         : true)
    << "destructor can not have return type, method: "
    << methodInfo->name;

  if(allowReturnType
     && methodInfo->returnType)
  {
    DCHECK(!methodInfo->returnType->getPrintedName().empty());
    result += methodInfo->returnType->getPrintedName();
    result += separator;
  } else {
    DVLOG(20)
      << (methodInfo->returnType
          ? "method has return type, "
          "but return type is not printed "
          "due to provided options"
          : "method without return type");
  }

  if(result.empty()) {
    VLOG(9)
      << "printMethodForwarding returned nothing for method: "
      << methodInfo->name;
  }

  return result;
}

std::string printMethodTrailing(
  const reflection::MethodInfoPtr& methodInfo
  , const std::string& separator
  // what method printer is allowed to print
  // |options| is a bitmask of |MethodPrinter::Trailing::Options|
  , int options
){
  DCHECK(methodInfo);

  std::string result;

  const bool allowConst
    = (options & MethodPrinter::Trailing::Options::CONST);

  const bool allowNoexcept
    = (options & MethodPrinter::Trailing::Options::NOEXCEPT);

  const bool allowPure
    = (options & MethodPrinter::Trailing::Options::PURE);

  const bool allowDeleted
    = (options & MethodPrinter::Trailing::Options::DELETED);

  const bool allowDefault
    = (options & MethodPrinter::Trailing::Options::DEFAULT);

  const bool allowBody
    = (options & MethodPrinter::Trailing::Options::BODY);

  if(allowConst
     && methodInfo->isConst)
  {
    result += "const";
    result += separator;
  } else {
    DVLOG(20)
      << (methodInfo->isConst
          ? "method is const, "
          "but const is not printed "
          "due to provided options"
          : "method is not const");
  }

  if(allowNoexcept
     && methodInfo->isNoExcept)
  {
    result += "noexcept";
    result += separator;
  } else {
    DVLOG(20)
      << (methodInfo->isNoExcept
          ? "method is noexcept, "
          "but noexcept is not printed "
          "due to provided options"
          : "method is not noexcept");
  }

  if(allowPure
     && methodInfo->isPure)
  {
    result += "= 0";
    result += separator;
  } else {
    DVLOG(20)
      << (methodInfo->isPure
          ? "method is pure (= 0), "
          "but (= 0) is not printed "
          "due to provided options"
          : "method is not pure (= 0)");
  }

  if(allowDeleted
     && methodInfo->isDeleted)
  {
    result += "= delete";
    result += separator;
  } else {
    DVLOG(20)
      << (methodInfo->isDeleted
          ? "method is deleted (= delete), "
          "but (= delete) is not printed "
          "due to provided options"
          : "method is not deleted (= delete)");
  }

  if(allowDefault
     && methodInfo->isDefault)
  {
    result += "= default";
    result += separator;
  } else {
    DVLOG(20)
      << (methodInfo->isDefault
          ? "method is default (= default), "
          "but (= default) is not printed "
          "due to provided options"
          : "method is not default (= default)");
  }

  const bool canHaveBody =
     methodInfo->isDefined
     && methodInfo->isClassScopeInlined;

  if(allowBody
     && canHaveBody)
  {
    DCHECK(!methodInfo->body.empty());
    result += methodInfo->body;
    DVLOG(20)
      << "created body for method"
      << methodInfo->name;
  }
  else if(allowBody)
  {
    // no body example: methodType methodName(methodArgs);
    result += ";";
    DVLOG(20)
      << "created empty body for method"
      << methodInfo->name;
  } else {
    DVLOG(20)
      << (canHaveBody
          ? "method can have body, "
          "but body is not printed "
          "due to provided options"
          : "method can't have body");
  }

  if(result.empty()) {
    VLOG(9)
      << "printMethodTrailing returned nothing for method: "
      << methodInfo->name;
  }

  return result;
}

// Before:
// struct only_for_code_generation::Spell SpellTraits
// After:
// only_for_code_generation::Spell SpellTraits
std::string extractTypeName(
  const std::string& input)
{
  {
    DCHECK(base::size(kStructPrefix));
    if(base::StartsWith(input, kStructPrefix
         , base::CompareCase::INSENSITIVE_ASCII))
    {
      return input.substr(base::size(kStructPrefix) - 1
                     , std::string::npos);
    }
  }

  {
    DCHECK(base::size(kRecordPrefix));
    const std::string prefix = "record ";
    if(base::StartsWith(input, kRecordPrefix
         , base::CompareCase::INSENSITIVE_ASCII))
    {
      return input.substr(base::size(kRecordPrefix) - 1
                     , std::string::npos);
    }
  }

  return input;
}

std::string forwardMethodParamNames(
  const std::vector<reflection::MethodParamInfo>& params)
{
  std::string out;
  size_t paramIter = 0;
  const size_t methodParamsSize = params.size();
  for(const auto& param: params) {
    reflection::TypeInfoPtr pType = param.type;
    if(pType->canBeMoved() || pType->getIsRVReference()) {
      out += "std::move(";
    }
    out += param.name;
    if(pType->canBeMoved() || pType->getIsRVReference()) {
      out += ")";
    }
    paramIter++;
    if(paramIter != methodParamsSize) {
      out += kSeparatorCommaAndWhitespace;
    } // paramIter != methodParamsSize
  } // params endfor
  return out;
}

// replaces clang matchResult with |replacement| in source code
void replaceWith(
  clang::Rewriter& rewriter
  , const clang::Decl* decl
  , const clang::ast_matchers::MatchFinder::MatchResult& matchResult
  , const std::string& replacement
  , const bool skip_rewrite_not_main_file)
{
  clang::SourceManager &SM
    = rewriter.getSourceMgr();

  clang::PrintingPolicy printingPolicy(
    rewriter.getLangOpts());

  const clang::LangOptions& langOptions
    = rewriter.getLangOpts();

  clang::SourceLocation startLoc
    = decl->getLocStart();
  // Note Stmt::getLocEnd() returns the source location prior to the
  // token at the end of the line.  For instance, for:
  // var = 123;
  //      ^---- getLocEnd() points here.
  clang::SourceLocation endLoc
    = decl->getLocEnd();

  // When there is a #include <vector> in the source file,
  // our find-decl will print out all the declarations
  // in that included file, because these included files are parsed
  // and consumed as a whole with our source file.
  // To fix this, we need to check if the declarations
  // are defined in our source file
  if(skip_rewrite_not_main_file) {
    const clang::FileID& mainFileID = SM.getMainFileID();
    const auto& FileID
      = SM.getFileID(decl->getLocation());
    if (FileID != mainFileID) {
      DVLOG(10)
        << "skipped rewriting of decl in included file: "
        << decl
             ->getLocation().printToString(SM).substr(0, 1000);
      return;
    }
  }

  clang_utils::expandLocations(
    startLoc, endLoc, rewriter);

  const clang::CXXRecordDecl *node =
    matchResult.Nodes.getNodeAs<
      clang::CXXRecordDecl>("bind_gen");

  // EXAMPLE:
  // template<typename impl = FooImpl>
  //   class my_annotation_attr()
  //     PimplMethodCallsInjector
  //   {}
  // we want to get range including template
  if(node->getDescribedClassTemplate()) {
    clang::SourceRange varSourceRange
      = node->getDescribedClassTemplate()->getSourceRange();
    DCHECK(varSourceRange.isValid());
    clang::CharSourceRange charSourceRange(
      varSourceRange,
      true // IsTokenRange
    );
    startLoc = charSourceRange.getBegin();
    DCHECK(startLoc.isValid());
    endLoc = charSourceRange.getEnd();
    DCHECK(endLoc.isValid());
  }

//#define DEBUG_PRINT_SOURCE_RANGE 1
#if defined(DEBUG_PRINT_SOURCE_RANGE)
  if(node->getDescribedClassTemplate())
  {
    clang::SourceRange varSourceRange
      = node->getDescribedClassTemplate()->getSourceRange();
    clang::CharSourceRange charSourceRange(
      varSourceRange,
      true // IsTokenRange
    );
    clang::SourceLocation initStartLoc
      = charSourceRange.getBegin();
    if(varSourceRange.isValid()) {
      llvm::StringRef sourceText
        = clang::Lexer::getSourceText(
            charSourceRange
            , SM, langOptions, 0);
      DCHECK(initStartLoc.isValid());
      DVLOG(9) << sourceText.str();
      DCHECK(false); // TODO
    }
  }
#endif // DEBUG_PRINT_SOURCE_RANGE

  {
    // gets us past the ';'.
    endLoc = clang_utils::findSemiAfterLocation(
      endLoc, rewriter);
  }

  /// \note if result.replacer is nullptr, than we will keep old code
  rewriter.ReplaceText(
    clang::SourceRange(startLoc, endLoc)
    , replacement);
}

} // namespace clang_utils
