﻿#include "flexlib/clangUtils.hpp" // IWYU pragma: associated

#include <base/logging.h>

namespace clang_utils {

std::string printMethodDecl(const clang::Decl* decl, clang::CXXRecordDecl const * node, clang::CXXMethodDecl* fct) {
    std::string methodDecl;
    clang::FunctionDecl *D = fct->getAsFunction();

    bool isCtor = llvm::dyn_cast_or_null<const clang::CXXConstructorDecl>(fct) != nullptr;
    bool isDtor = llvm::dyn_cast_or_null<const clang::CXXDestructorDecl>(fct) != nullptr;
    bool isOperator = fct->isOverloadedOperator();

    if(isCtor || isDtor || isOperator) {
        return "";
    }

    printf("called printMethodDecl\n");

    printf("found function %s in CXXRecordDecl %s\n",
           fct->getNameAsString().c_str(),
           node->getName().str().c_str());

    // TODO: https://stackoverflow.com/questions/56792760/how-to-get-the-actual-type-of-a-template-typed-class-member-with-clang/56796422#56796422https://stackoverflow.com/questions/56792760/how-to-get-the-actual-type-of-a-template-typed-class-member-with-clang/56796422#56796422
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

    //printf("%s", methodDecl.c_str());

    methodDecl += " __" + fct->getNameAsString() + " ";

    //printf("%s", methodDecl.c_str());

    unsigned Indentation = 0;
    //bool PrintInstantiation = false;
    clang::LangOptions LO;
    clang::PrintingPolicy PrintPolicy(LO);
    //PrintPolicy.AnonymousTagLocations = false;
    //PrintPolicy.SuppressTagKeyword = true;

    methodDecl += " ( ";
    // prams | join(', ')
    std::string Proto;
    llvm::raw_string_ostream POut(Proto);

    // see https://github.com/flexferrum/flex_lib/blob/322adb7acdfbc0e541292708a2bd66b305b05c52/tools/codegen/src/ast_reflector.cpp#L174
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

            /*methodDecl += param->getType().getAsString() + " ";
        methodDecl += param->getNameAsString() + " ";*/

            if(nextparam && nextparam != D->param_end()/*pc && pc <= D->getNumParams()*/) {
                methodDecl += ", ";
            }

            //paramInfo.type = TypeInfo::Create(param->getType(), m_astContext);
            //paramInfo.fullDecl = EntityToString(param, m_astContext);
            //methodInfo->params.push_back(std::move(paramInfo));
            pc++;
        }
    }

    methodDecl += " ) ";

    //printf("%s", methodDecl.c_str());

    if(fct->isConst()) {
        methodDecl += " const ";
    }

    // see https://github.com/flexferrum/autoprogrammer/blob/db902121dd492a2df2b7287e0dafd7173062bcc7/src/ast_reflector.cpp#L386
    clang::QualType fnQualType = fct->getType();

    const clang::FunctionProtoType* fnProtoType = nullptr;
    if (const clang::FunctionType *fnType =
            fnQualType->getAs<clang::FunctionType>())
    {
        if (D->hasWrittenPrototype())
            fnProtoType = llvm::dyn_cast<clang::FunctionProtoType>(fnType);
    }

    // see https://github.com/FunkMonkey/libClang/blob/ab4702febef82409773f7c80ec02d53ddbb4d80e/lib/AST/DeclPrinter.cpp#L468
    if(isNoexceptExceptionSpec(fnProtoType->getExceptionSpecType())) {
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

    //methodDecl += "\n";

    printf("%s\n", methodDecl.c_str());

    return methodDecl;
}

void expandLocations(clang::SourceLocation& startLoc,
                     clang::SourceLocation& endLoc,
                     clang::Rewriter& rewriter_)
{
  // macros need a special handling, because we are interessted in the macro
  // instanciation location and not in the macro definition location
  clang::SourceManager& sm = rewriter_.getSourceMgr();

  for (size_t i = 0; startLoc.isMacroID(); i++) {
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

      // Get the start/end expansion locations
      //std::pair< clang::SourceLocation, clang::SourceLocation >
      //  expansionRange =
      //    sm.isMacroArgExpansion(startLoc)
      //    ? sm.getImmediateSpellingLoc(startLoc)
      //    : sm.getImmediateExpansionRange( startLoc );

      // We're just interested in the start location
      //startLoc = expansionRange.first;

      // will not be executed
      //endLoc = expansionRange.second;
  }
}

} // namespace clang_utils
