#include "Type.h"
#include "StringUtils.h"

namespace solgen {
Type::Type(CXType cxType): cxType(cxType) {}

bool Type::operator==(const Type &other) const {
    // NOTE: clang_equalTypes(cxType, other.cxType) for some reason returns
    // false for the same type as CXCursor_ClassDecl and CXCursor_CXXBaseSpecifier
    // possible because of modifiers?
    return std::hash<Type>()(*this) == std::hash<Type>()(other);
}

bool Type::operator<(const Type &other) const {
    return std::hash<Type>()(*this) < std::hash<Type>()(other);
}

std::string Type::getName() const {
    return cx2str(clang_getTypeSpelling(cxType));
}

std::string Type::getCanonicalName() const {
    return cx2str(clang_getTypeSpelling(clang_getCanonicalType(cxType)));
}

Type Type::getResultType() const {
    return clang_getResultType(cxType);
}

bool Type::isInvalid() const {
    return cxType.kind == CXType_Invalid;
}
}
