﻿#include "flexlib/reflect/ReflectAST.hpp" // IWYU pragma: associated

#include <clang/AST/ASTContext.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/RecordLayout.h>
#include <clang/AST/Attr.h>

#include <iostream>

#include "flexlib/reflect/ast_utils.hpp"

#include <boost/algorithm/string/replace.hpp>

#include <base/logging.h>
#include <base/check.h>

namespace reflection
{

using namespace clang;


template<typename Cont>
auto FindExisting(
  const Cont& cont, const std::string& qualifiedName)
{
  auto p = std::find_if(begin(cont), end(cont),
    [&qualifiedName](auto& item)
    {
      return item->decl->getQualifiedNameAsString()
        == qualifiedName;
    }
  );

  return p == end(cont) ? typename Cont::value_type() : *p;
}

AccessType ConvertAccessType(
  clang::AccessSpecifier access)
{
  AccessType result = AccessType::Undefined;
  switch (access)
  {
    case clang::AS_public: {
      result = AccessType::Public;
      break;
    }
    case clang::AS_protected: {
      result = AccessType::Protected;
      break;
    }
    case clang::AS_private:{
      result = AccessType::Private;
      break;
    }
    case clang::AS_none: {
      break;
    }
  }

  return result;
}

EnumInfoPtr AstReflector::ReflectEnum(
  const clang::EnumDecl *decl, NamespacesTree* nsTree)
{
  const DeclContext* nsContext
    = decl->getEnclosingNamespaceContext();

  NamespaceInfoPtr ns;
  EnumInfoPtr enumInfo;
  if (nsTree != nullptr)
  {
    ns = nsTree->GetNamespace(nsContext);
    enumInfo = FindExisting(ns->enums, decl->getQualifiedNameAsString());
  }

  if (enumInfo) {
    return enumInfo;
  }

  ///\todo
  // const NamedDecl* parentDecl = FindEnclosingOpaqueDecl(decl);

  enumInfo = std::make_shared<EnumInfo>();
  enumInfo->decl = decl;
  DCHECK(m_astContext);
  enumInfo->location = GetLocation(decl, m_astContext);

  SetupNamedDeclInfo(decl, enumInfo.get(), m_astContext);

  enumInfo->isScoped = decl->isScoped();

  for (const EnumConstantDecl* itemDecl : decl->enumerators())
  {
    EnumItemInfo item;
    item.itemName = itemDecl->getNameAsString();
    item.itemValue = itemDecl->getInitVal().toString(10);
    item.location = GetLocation(itemDecl, m_astContext);
    enumInfo->items.push_back(std::move(item));
  }

  if (ns) {
    ns->enums.push_back(enumInfo);
  }

  return enumInfo;
}

TypedefInfoPtr AstReflector::ReflectTypedef(
  const TypedefNameDecl* decl, NamespacesTree* nsTree)
{
  const DeclContext* nsContext = decl->getDeclContext();

  NamespaceInfoPtr ns;
  TypedefInfoPtr typedefInfo;
  if (nsTree != nullptr)
  {
    ns = nsTree->GetNamespace(nsContext);
    typedefInfo = FindExisting(
      ns->typedefs, decl->getQualifiedNameAsString());
  }

  if (typedefInfo) {
    return typedefInfo;
  }

  // const NamedDecl* parentDecl = FindEnclosingOpaqueDecl(decl);

  typedefInfo = std::make_shared<TypedefInfo>();
  typedefInfo->decl = decl;
  typedefInfo->location = GetLocation(decl, m_astContext);

  SetupNamedDeclInfo(decl, typedefInfo.get(), m_astContext);
  typedefInfo->aliasedType
    = TypeInfo::Create(decl->getUnderlyingType(), m_astContext);

  if (ns) {
    ns->typedefs.push_back(typedefInfo);
  }

  return typedefInfo;
}

ClassInfoPtr AstReflector::ReflectClass(
  const CXXRecordDecl* decl, NamespacesTree* nsTree, bool recursive)
{
  DCHECK(decl);

  const DeclContext* nsContext
    = decl->getEnclosingNamespaceContext();

  NamespaceInfoPtr ns;
  ClassInfoPtr classInfo;

  if (decl->hasDefinition()) {
    decl = decl->getDefinition();
  }

  if (nsTree)
  {
    ns = nsTree->GetNamespace(nsContext);
    classInfo = FindExisting(
      ns->classes, decl->getQualifiedNameAsString());
  }

  if (classInfo) {
    return classInfo;
  }

  classInfo = std::make_shared<ClassInfo>();
  classInfo->decl = decl;

  DCHECK(m_astContext);
  SetupNamedDeclInfo(decl, classInfo.get(), m_astContext);
  classInfo->isUnion = decl->isUnion();
  classInfo->location = GetLocation(decl, m_astContext);

  // see https://github.com/goto40/rpp/blob/ec8a4c4a3ac32dccee8c4e8ba97be8c2ba1c8f88/src/parser/struct_parser.cpp#L113
  if(decl->getDescribedClassTemplate()) {
    clang::TemplateDecl* templateDecl =
      decl->getDescribedClassTemplate();
    clang::TemplateParameterList *tp =
      templateDecl->getTemplateParameters();
    for(clang::NamedDecl *p: *tp) {
      classInfo->templateParams.push_back(
        TemplateParamInfo{
          p->getNameAsString(),
          p->getQualifiedNameAsString(),
        }
      );
    }
  }

  if (decl->hasDefinition())
  {
    DCHECK(m_astContext);

    DCHECK(!decl->isInvalidDecl());

    DCHECK(llvm::dyn_cast_or_null<clang::RecordDecl>(decl));

    FullSourceLoc fullLocation
      = m_astContext->getFullLoc(decl->getBeginLoc());
    DCHECK(fullLocation.isValid());

    /// Get or compute information about the layout
    /// of the specified record (struct/union/class),
    /// which indicates its size and field position information.
    const clang::ASTRecordLayout& layout
      = m_astContext->getASTRecordLayout(decl);

    classInfo->ASTRecordSize
      = layout.getSize().getQuantity();

    classInfo->ASTRecordNonVirtualAlignment
      = layout.getNonVirtualAlignment().getQuantity();

    // If the class is final, then we know that the pointer points to an
    // object of that type and can use the full alignment.
    if (decl->hasAttr<clang::FinalAttr>()) {
      classInfo->ASTRecordNonVirtualAlignment
        = layout.getAlignment().getQuantity();
    }

    DCHECK(nsTree);
    ReflectImplicitSpecialMembers(decl, classInfo.get(), nsTree);

    for (auto methodDecl : decl->methods())
    {
        MethodInfoPtr methodInfo = ReflectMethod(methodDecl, nsTree);
        classInfo->methods.push_back(methodInfo);
    }

    classInfo->isAbstract = decl->isAbstract();
    classInfo->isTrivial = decl->isTrivial();
    classInfo->hasDefinition = true;
    for (auto& base : decl->bases())
    {
        ClassInfo::BaseInfo baseInfo;
        baseInfo.isVirtual = base.isVirtual();
        baseInfo.accessType = ConvertAccessType(base.getAccessSpecifier());
        baseInfo.baseClass = TypeInfo::Create(base.getType(), m_astContext);
        classInfo->baseClasses.push_back(std::move(baseInfo));
    }

    for (auto& d : decl->decls())
    {
        const clang::TagDecl* tagDecl = llvm::dyn_cast_or_null<TagDecl>(d);
        const clang::NamedDecl* namedDecl = llvm::dyn_cast_or_null<NamedDecl>(d);

        ClassInfo::InnerDeclInfo declInfo;
        const CXXRecordDecl* innerRec = nullptr;
        const TypedefNameDecl* typeAliasDecl = nullptr;
        const FieldDecl* fieldDecl = nullptr;
        const VarDecl* varDecl = nullptr;
        bool processed = true;
        if (tagDecl && tagDecl->isEnum())
        {
            auto ei = ReflectEnum(llvm::dyn_cast<EnumDecl>(tagDecl), nullptr);
            declInfo.innerDecl = ei;
        }
        else if ((innerRec = llvm::dyn_cast_or_null<CXXRecordDecl>(tagDecl)))
        {
            if(recursive) {
              auto ci = ReflectClass(innerRec, nullptr);
              declInfo.innerDecl = ci;
            }
        }
        else if ((typeAliasDecl = llvm::dyn_cast_or_null<TypedefNameDecl>(namedDecl)))
        {
            auto ti = ReflectTypedef(typeAliasDecl, nullptr);
            declInfo.innerDecl = ti;
        }
        else if ((fieldDecl = llvm::dyn_cast_or_null<FieldDecl>(d)))
        {
            if (fieldDecl->isAnonymousStructOrUnion())
                continue;

            auto memberInfo = std::make_shared<MemberInfo>();
            SetupNamedDeclInfo(fieldDecl, memberInfo.get(), m_astContext);
            memberInfo->type = TypeInfo::Create(fieldDecl->getType(), m_astContext);
            memberInfo->accessType = ConvertAccessType(fieldDecl->getAccess());
            memberInfo->decl = fieldDecl;
            classInfo->members.push_back(memberInfo);
            processed = false;
        }
        else if ((varDecl = llvm::dyn_cast_or_null<VarDecl>(d)))
        {
            auto memberInfo = std::make_shared<MemberInfo>();
            SetupNamedDeclInfo(varDecl, memberInfo.get(), m_astContext);
            memberInfo->type = TypeInfo::Create(varDecl->getType(), m_astContext);
            memberInfo->isStatic = varDecl->isStaticDataMember();
            memberInfo->accessType = ConvertAccessType(varDecl->getAccess());
            memberInfo->decl = varDecl;
            classInfo->members.push_back(memberInfo);
            processed = false;
        }
        else
        {
            processed = false;
        }

        if (processed)
        {
            declInfo.acessType = ConvertAccessType(tagDecl ? tagDecl->getAccess() : namedDecl->getAccess());
            classInfo->innerDecls.push_back(std::move(declInfo));
        }
    }
  }

  if (ns) {
    ns->classes.push_back(classInfo);
  }

  return classInfo;
}

void AstReflector::ReflectImplicitSpecialMembers(
  const CXXRecordDecl* decl
  , ClassInfo* classInfo
  , NamespacesTree* nsTree)
{
  DCHECK(decl);
  DCHECK(classInfo);

  auto setupImplicitMember
    = [this, decl, classInfo]
      (const std::string& name)
  {
      MethodInfoPtr methodInfo
        = std::make_shared<MethodInfo>();
      methodInfo->scopeSpecifier
        = classInfo->GetFullQualifiedName();
      methodInfo->namespaceQualifier
        = classInfo->namespaceQualifier;
      methodInfo->name = name;
      methodInfo->accessType = AccessType::Public;
      methodInfo->declLocation = classInfo->location;
      methodInfo->defLocation = classInfo->location;
      methodInfo->isImplicit = true;

      return methodInfo;
  };

  auto setupConstructor
    = [this, decl, classInfo, &setupImplicitMember]()
  {
    MethodInfoPtr methodInfo
      = setupImplicitMember(classInfo->name);
    methodInfo->isCtor = true;

    return methodInfo;
  };

  if (!decl->hasUserProvidedDefaultConstructor()
      && decl->hasDefaultConstructor())
  {
    auto ctorInfo = setupConstructor();
    ctorInfo->constructorType = ConstructorType::Default;
    classInfo->methods.push_back(ctorInfo);
  }

  if (!decl->hasUserDeclaredCopyConstructor()
      && decl->hasCopyConstructorWithConstParam())
  {
    DCHECK(m_astContext);
    auto rt = m_astContext->getRecordType(decl);
    auto constRefT = m_astContext->getLValueReferenceType(m_astContext->getConstType(rt));
    auto ctorInfo = setupConstructor();
    ctorInfo->constructorType = ConstructorType::Copy;

    MethodParamInfo paramInfo;
    paramInfo.name = "val";
    paramInfo.type = TypeInfo::Create(constRefT, m_astContext);
    paramInfo.fullDecl = EntityToString(&constRefT, m_astContext) + " val";
    ctorInfo->params.push_back(std::move(paramInfo));

    classInfo->methods.push_back(ctorInfo);
  }

  if (!decl->hasUserDeclaredMoveConstructor()
      && decl->hasMoveConstructor())
  {
    DCHECK(m_astContext);
    auto rt = m_astContext->getRecordType(decl);
    auto constRefT
      = m_astContext->getRValueReferenceType(
        m_astContext->getConstType(rt));
    auto ctorInfo = setupConstructor();
    ctorInfo->constructorType = ConstructorType::Move;

    MethodParamInfo paramInfo;
    paramInfo.name = "val";
    paramInfo.type
      = TypeInfo::Create(constRefT, m_astContext);
    paramInfo.fullDecl
      = EntityToString(&constRefT, m_astContext) + " val";
    ctorInfo->params.push_back(std::move(paramInfo));

    classInfo->methods.push_back(ctorInfo);
  }

  if (!decl->hasUserDeclaredDestructor()
      && decl->hasSimpleDestructor())
  {
    auto dtorInfo = setupImplicitMember("~" + classInfo->name);
    dtorInfo->isDtor = true;
    classInfo->methods.push_back(dtorInfo);
  }

  if (!decl->hasUserDeclaredCopyAssignment()
      && decl->hasCopyAssignmentWithConstParam())
  {
    DCHECK(m_astContext);
    auto rt = m_astContext->getRecordType(decl);
    auto constRefT = m_astContext->getLValueReferenceType(m_astContext->getConstType(rt));
    auto operInfo = setupImplicitMember("operator=");
    operInfo->isOperator = true;
    operInfo->assignmentOperType = AssignmentOperType::Copy;

    MethodParamInfo paramInfo;
    paramInfo.name = "val";
    paramInfo.type = TypeInfo::Create(constRefT, m_astContext);
    paramInfo.fullDecl = EntityToString(&constRefT, m_astContext) + " val";
    operInfo->params.push_back(std::move(paramInfo));

    classInfo->methods.push_back(operInfo);
  }

  if (!decl->hasUserDeclaredMoveConstructor()
      && decl->hasMoveAssignment())
  {
    DCHECK(m_astContext);
    auto rt = m_astContext->getRecordType(decl);
    auto constRefT = m_astContext->getRValueReferenceType(m_astContext->getConstType(rt));
    auto operInfo = setupImplicitMember("operator=");
    operInfo->isOperator = true;
    operInfo->assignmentOperType = AssignmentOperType::Move;

    MethodParamInfo paramInfo;
    paramInfo.name = "val";
    paramInfo.type = TypeInfo::Create(constRefT, m_astContext);
    paramInfo.fullDecl = EntityToString(&constRefT, m_astContext) + " val";
    operInfo->params.push_back(std::move(paramInfo));

    classInfo->methods.push_back(operInfo);
  }
}

MethodInfoPtr AstReflector::ReflectMethod(
  const FunctionDecl* decl, NamespacesTree* nsTree)
{
  const clang::CXXMethodDecl* cxxDecl
    = llvm::dyn_cast_or_null<const clang::CXXMethodDecl>(decl);

  MethodInfoPtr methodInfo
    = std::make_shared<MethodInfo>();

  const DeclContext* nsContext
    = decl->getEnclosingNamespaceContext();

  NamespaceInfoPtr ns = nullptr;
  if (nsTree)
  {
    ns = nsTree->GetNamespace(nsContext);
  }
  SetupNamedDeclInfo(decl, methodInfo.get(), m_astContext);

  QualType fnQualType = decl->getType();

  while (const ParenType *parenType
    = llvm::dyn_cast<ParenType>(fnQualType))
  {
      fnQualType = parenType->getInnerType();
  }

  const FunctionProtoType* fnProtoType = nullptr;
  if (const FunctionType *fnType
    = fnQualType->getAs<FunctionType>())
  {
    if (decl->hasWrittenPrototype()) {
      fnProtoType
        = llvm::dyn_cast<FunctionProtoType>(fnType);
    }
  }

  const clang::CXXConstructorDecl* ctor
    = llvm::dyn_cast_or_null<const clang::CXXConstructorDecl>(decl);

  methodInfo->isCtor = ctor != nullptr;
  if (ctor != nullptr)
  {
    if (ctor->isDefaultConstructor()) {
      methodInfo->constructorType = ConstructorType::Default;
    }
    else if (ctor->isCopyConstructor()) {
      methodInfo->constructorType = ConstructorType::Copy;
    }
    else if (ctor->isMoveConstructor()) {
      methodInfo->constructorType = ConstructorType::Move;
    }
    else if (ctor->isConvertingConstructor(true)) {
      methodInfo->constructorType = ConstructorType::Convert;
    }
    methodInfo->isExplicitCtor = ctor->isExplicit();
  }
  methodInfo->isDtor
    = llvm::dyn_cast_or_null<const clang::CXXDestructorDecl>(decl)
        != nullptr;
  methodInfo->isOperator = decl->isOverloadedOperator();
  methodInfo->isDeleted = decl->isDeleted();
  methodInfo->isConst = cxxDecl && cxxDecl->isConst();
  methodInfo->isImplicit = decl->isImplicit();
  if (fnProtoType) {
    methodInfo->isNoExcept
      = clang::isNoexceptExceptionSpec(
          fnProtoType->getExceptionSpecType());
  }
  methodInfo->isConstexpr
    = cxxDecl && cxxDecl->isConstexpr();
  methodInfo->isPure = decl->isPure();
  methodInfo->isStatic
    = cxxDecl && cxxDecl->isStatic();
  methodInfo->isVirtual
    = cxxDecl && cxxDecl->isVirtual();
  methodInfo->accessType
    = ConvertAccessType(decl->getAccess());
  DCHECK(m_astContext);
  methodInfo->fullPrototype
    = EntityToString(decl, m_astContext);
  methodInfo->decl = decl;
  methodInfo->cxxDecl = cxxDecl;
  methodInfo->returnType
    = TypeInfo::Create(decl->getReturnType(), m_astContext);
  methodInfo->isInlined = decl->isInlined();
  methodInfo->isDefined = decl->isDefined();
  methodInfo->isDefault = decl->isDefaulted();

  const clang::Stmt* body = decl->getBody();
  if (body != nullptr)
  {
    auto& srcMgr = m_astContext->getSourceManager();
    clang::SourceLocation locStart = body->getBeginLoc();
    clang::SourceLocation locEnd = body->getEndLoc();
    auto len
      = srcMgr.getFileOffset(locEnd)
          - srcMgr.getFileOffset(locStart);

    auto buff = srcMgr.getCharacterData(locStart);
    std::string content(buff, buff + len);
    methodInfo->body = std::move(content);
    methodInfo->isDefined = true;
    methodInfo->isClassScopeInlined
      = decl->getDefinition() == decl->getFirstDecl();
  }

  methodInfo->declLocation
    = GetLocation(decl, m_astContext);
  auto defDecl = decl->getDefinition();
  if (defDecl != nullptr) {
      methodInfo->defLocation
        = GetLocation(defDecl, m_astContext);
  }

  DVLOG(9)
    << "MethodParamInfo parameters().size(): "
    << methodInfo->name
    << decl->parameters().size();

  for (const clang::ParmVarDecl* param: decl->parameters())
  {
    DCHECK(param);
    MethodParamInfo paramInfo;
    paramInfo.name = param->getNameAsString();
    DVLOG(9)
      << "MethodParamInfo name "
      << paramInfo.name;
    paramInfo.type
      = TypeInfo::Create(param->getType(), m_astContext);
    DCHECK(paramInfo.type);
    paramInfo.fullDecl
      = EntityToString(param, m_astContext);
    paramInfo.decl = param;
    DCHECK(paramInfo.decl);

    DVLOG(9)
      << "MethodParamInfo fullDecl "
      << paramInfo.fullDecl;

    methodInfo->params.push_back(std::move(paramInfo));
  }

  /// \todo
  const FunctionDecl* tplInst
    = decl->getTemplateInstantiationPattern();
  //    if (tplInst != nullptr)
  //        tplInst->dump();

  //    tplInst = decl->getClassScopeSpecializationPattern();
  //    if (tplInst != nullptr)
  //        tplInst->dump();

  //    tplInst = decl->getInstantiatedFromMemberFunction();
  //    if (tplInst != nullptr)
  //        tplInst->dump();

  DVLOG(9)
    << "returning methodInfo for "
    << methodInfo->name;

  return methodInfo;
}

void AstReflector::SetupNamedDeclInfo(
  const NamedDecl* decl
  , NamedDeclInfo* info
  , const clang::ASTContext* astContext)
{
  info->name = decl->getNameAsString();

  auto declContext = decl->getDeclContext();
  const clang::NamespaceDecl* encNs = nullptr;
  /// \todo
  // = clang::NamespaceDecl::castFromDeclContext(declContext->isNamespace() ? declContext : declContext->getEnclosingNamespaceContext());
  /// \todo
  // auto encNsCtx = clang::NamespaceDecl::castToDeclContext(encNs);

  std::string scopeQualifier = "";

  for (const DeclContext* parentCtx = declContext
       ; parentCtx != nullptr
       ; parentCtx = parentCtx->getParent())
  {
    const NamedDecl* namedScope
      = llvm::dyn_cast_or_null<const NamedDecl>(
          Decl::castFromDeclContext(parentCtx));
    if (namedScope == nullptr) {
      continue;
    }

    if (parentCtx->isNamespace())
    {
      encNs
        = clang::NamespaceDecl::castFromDeclContext(parentCtx);
      break;
    }

    auto scopeName = namedScope->getNameAsString();
    if (scopeName.empty()) {
      continue;
    }

    if (scopeQualifier.empty()) {
      scopeQualifier = scopeName;
    } else {
      scopeQualifier = scopeName + "::" + scopeQualifier;
    }
  }

  info->scopeSpecifier = scopeQualifier;

  if (encNs != nullptr && !encNs->isTranslationUnit())
  {
    clang::PrintingPolicy policy(astContext->getLangOpts());
    SetupDefaultPrintingPolicy(policy);

    llvm::raw_string_ostream os(info->namespaceQualifier);
    encNs->printQualifiedName(os, policy);
  }

  boost::algorithm::replace_all(
    info->namespaceQualifier, "(anonymous)::", "");
  boost::algorithm::replace_all(
    info->namespaceQualifier, "::(anonymous)", "");
  boost::algorithm::replace_all(
    info->namespaceQualifier, "(anonymous)", "");
}

} // reflection
