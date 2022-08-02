#ifndef SOL2_GENERATOR_TYPE_H
#define SOL2_GENERATOR_TYPE_H

#include <clang-c/Index.h>

#include <string>

namespace solgen {
class Type {
public:
    Type() = default;
    Type(CXType cxType);

    bool operator==(const Type &other) const;
    bool operator<(const Type &other) const;

    std::string getName() const;
    std::string getCanonicalName() const;

    Type getResultType() const;

    bool isInvalid() const;

public:
    CXType cxType;
};
}

namespace std {
template <>
struct hash<solgen::Type> {
    std::size_t operator()(const solgen::Type &k) const {
        if (auto resType = k.getResultType(); resType.isInvalid()) {
            return std::hash<std::string>()(k.getCanonicalName());
        } else {
            return std::hash<std::string>()(k.getCanonicalName()) ^ std::hash<solgen::Type>()(resType);
        }
    }
};

}

#endif
