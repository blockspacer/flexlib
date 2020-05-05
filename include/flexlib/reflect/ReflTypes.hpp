﻿#pragma once

#include <clang/AST/DeclCXX.h>

#include <memory>
#include <map>
#include <string>

#include "flexlib/reflect/TypeInfo.hpp"

namespace reflection
{

struct ClassInfo;
using ClassInfoPtr = std::shared_ptr<ClassInfo>;

struct EnumInfo;
using EnumInfoPtr = std::shared_ptr<EnumInfo>;

struct NamespaceInfo;
using NamespaceInfoPtr = std::shared_ptr<NamespaceInfo>;

struct NamedDeclInfo
{
    std::string name;
    std::string namespaceQualifier;
    std::string scopeSpecifier;

    std::string GetFullQualifiedScope(bool includeGlobalScope = true) const
    {
        std::string result;
        if (!namespaceQualifier.empty())
            result = namespaceQualifier;

        const char* prefix = includeGlobalScope || !result.empty() ? "::" : "";

        if (!scopeSpecifier.empty())
            result += prefix + scopeSpecifier;

        return result;
    }
    std::string GetFullQualifiedName() const
    {
        auto fqScope = GetFullQualifiedScope();
        const char* prefix = !fqScope.empty() ? "::" : "";
        return !name.empty() ? fqScope + prefix + name : "";
    }
    std::string GetScopedName() const
    {
        return scopeSpecifier.empty() ? name : scopeSpecifier + "::" + name;
    }
};

struct MethodParamInfo
{
    std::string name;
    TypeInfoPtr type;
    std::string fullDecl;

    const clang::ParmVarDecl* decl;
};

struct SourceLocation
{
    std::string fileName = "";
    unsigned line = 0;
    unsigned column = 0;

};

struct LocationInfo
{
    SourceLocation location;
};

struct TemplateParamInfo
{
    std::string tplDeclName;
    std::string tplRefName;
    TemplateType::TplArgKind kind = TemplateType::TemplateTplArg;
    bool isParamPack = false;
};

enum class AccessType
{
    Public,
    Protected,
    Private,
    Undefined
};

enum class AssignmentOperType
{
    None,
    Generic,
    Copy,
    Move
};

enum class ConstructorType
{
    None,
    Generic,
    Default,
    Copy,
    Move,
    Convert
};

struct GenericDeclPart : public LocationInfo
{
    std::string content;
    AccessType accessType = AccessType::Undefined;
};

using NamedDeclInfoPtr = std::shared_ptr<NamedDeclInfo>;
struct MethodInfo : public NamedDeclInfo
{
    SourceLocation declLocation;
    SourceLocation defLocation;
    std::vector<MethodParamInfo> params;
    std::vector<TemplateParamInfo> tplParams;
    std::string fullPrototype;
    TypeInfoPtr returnType;
    std::string returnTypeAsString;
    AccessType accessType = AccessType::Undefined;
    std::string body;

    bool isConstexpr = false;
    bool isConst = false;
    bool isVirtual = false;
    bool isPure = false;
    bool isNoExcept = false;
    bool isRVRef = false;
    bool isCtor = false;
    bool isDtor = false;
    bool isOperator = false;
    bool isImplicit = false;
    bool isDeleted = false;
    bool isStatic = false;
    bool isExplicitCtor = false;
    bool isInlined = false;
    bool isClassScopeInlined = false;
    bool isDefined = false;
    bool isDefault = false;
    AssignmentOperType assignmentOperType = AssignmentOperType::None;
    ConstructorType constructorType = ConstructorType::None;

    bool isTemplate() const {return !tplParams.empty();}

    const clang::FunctionDecl* decl;
};


using MethodInfoPtr = std::shared_ptr<MethodInfo>;

struct MemberInfo : public NamedDeclInfo, public LocationInfo
{
    TypeInfoPtr type;
    AccessType accessType = AccessType::Undefined;
    bool isStatic = false;

    const clang::DeclaratorDecl* decl;
};

using MemberInfoPtr = std::shared_ptr<MemberInfo>;

struct TypedefInfo : public NamedDeclInfo, public LocationInfo
{
    TypeInfoPtr aliasedType;
    const clang::TypedefNameDecl* decl;
};

using TypedefInfoPtr = std::shared_ptr<TypedefInfo>;

struct ClassInfo : public NamedDeclInfo, public LocationInfo
{
    struct BaseInfo
    {
        TypeInfoPtr baseClass;
        AccessType accessType;
        bool isVirtual;
    };

    struct InnerDeclInfo
    {
        using DeclType = std::variant<ClassInfoPtr, EnumInfoPtr, TypedefInfoPtr>;

        auto AsClassInfo() const
        {
            ClassInfoPtr def;
            auto ptr = std::get_if<ClassInfoPtr>(&innerDecl);
            if (!ptr)
                return def;
            return *ptr;
        }

        auto AsEnumInfo() const
        {
            EnumInfoPtr def;
            auto ptr = std::get_if<EnumInfoPtr>(&innerDecl);
            if (!ptr)
                return def;
            return *ptr;
        }

        auto AsTypedefInfo() const
        {
            TypedefInfoPtr def;
            auto ptr = std::get_if<TypedefInfoPtr>(&innerDecl);
            if (!ptr)
                return def;
            return *ptr;
        }

        bool IsClass() const {return std::get_if<ClassInfoPtr>(&innerDecl);}
        bool IsEnum() const {return std::get_if<EnumInfoPtr>(&innerDecl);}
        bool IsTypedef() const {return std::get_if<TypedefInfoPtr>(&innerDecl);}

        DeclType innerDecl;
        AccessType acessType;
    };

    std::vector<std::string> compoundId;

    std::vector<BaseInfo> baseClasses;
    std::vector<MemberInfoPtr> members;
    std::vector<MethodInfoPtr> methods;
    std::vector<InnerDeclInfo> innerDecls;
    std::vector<GenericDeclPart> genericParts;
    std::vector<TemplateParamInfo> templateParams;

    bool isTrivial = false;
    bool isAbstract = false;
    bool isUnion = false;
    bool hasDefinition = false;

    const clang::CXXRecordDecl* decl;
};

struct EnumItemInfo : public LocationInfo
{
    std::string itemName;
    std::string itemValue;

    const clang::EnumConstantDecl* decl = nullptr;
};

struct EnumInfo : public NamedDeclInfo, public LocationInfo
{
    bool isScoped = false;
    std::vector<EnumItemInfo> items;

    const clang::EnumDecl* decl = nullptr;
};


struct NamespaceInfo : public NamedDeclInfo
{
    bool isRootNamespace = false;
    std::vector<NamespaceInfoPtr> innerNamespaces;
    std::vector<EnumInfoPtr> enums;
    std::vector<ClassInfoPtr> classes;
    std::vector<TypedefInfoPtr> typedefs;

    const clang::NamespaceDecl *decl = nullptr;
};

class NamespacesTree
{
public:

    auto GetRootNamespace() const {return m_rootNamespace;}
    NamespaceInfoPtr GetNamespace(const clang::DeclContext* decl)
    {
        auto p = m_namespaces.find(decl);
        if (p != m_namespaces.end())
            return p->second;

        if (decl->isTranslationUnit())
        {
            NamespaceInfoPtr nsInfo = std::make_shared<NamespaceInfo>();
            nsInfo->name = "";
            nsInfo->namespaceQualifier = "";
            nsInfo->scopeSpecifier = "";
            nsInfo->isRootNamespace = true;
            m_namespaces[decl] = nsInfo;
            m_rootNamespace = nsInfo;
            return nsInfo;
        }

        const clang::NamespaceDecl* nsDecl = clang::NamespaceDecl::castFromDeclContext(decl);
        if (nsDecl == nullptr)
            return NamespaceInfoPtr();

        const clang::DeclContext* parentContext = nullptr;
        do
        {
            parentContext = nsDecl->getParent();
        } while (!parentContext->isNamespace() && !parentContext->isTranslationUnit());

        auto parentNs = GetNamespace(parentContext);

        if (nsDecl->isAnonymousNamespace())
        {
            m_namespaces[decl] = parentNs;
            return parentNs;
        }

        NamespaceInfoPtr nsInfo = std::make_shared<NamespaceInfo>();
        nsInfo->name = nsDecl->getNameAsString();
        nsInfo->namespaceQualifier = parentNs->GetFullQualifiedName();
        nsInfo->scopeSpecifier = "";
        nsInfo->decl = nsDecl;
        m_namespaces[decl] = nsInfo;
        parentNs->innerNamespaces.push_back(nsInfo);

        return nsInfo;
    }

private:
    NamespaceInfoPtr m_rootNamespace;
    std::map<const clang::DeclContext*, NamespaceInfoPtr> m_namespaces;
};

} // reflection
