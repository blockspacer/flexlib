﻿#include "flexlib/reflect/ReflectionCache.hpp" // IWYU pragma: associated

#if 0
namespace reflection {

ReflectionRegistry *ReflectionRegistry::instance;

ReflectionRegistry *ReflectionRegistry::getInstance() {
    if (!instance)
        instance = new ReflectionRegistry;
    return instance;
}

ReflectionCXXRecordRegistry::ReflectionCXXRecordRegistry(const reflectionID &id/*, const CXXRecordDecl *node*/, ClassInfoPtr classInfoPtr)
    : id_(id)/*, node_(node)*/, classInfoPtr_(classInfoPtr) {
}

} // namespace reflection
#endif // 0
