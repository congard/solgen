#ifndef SOL2_GENERATOR_STRINGUTILS_H
#define SOL2_GENERATOR_STRINGUTILS_H

#include <iostream>
#include <clang-c/CXString.h>

namespace solgen {
inline std::ostream& operator<<(std::ostream& stream, const CXString& str) {
    stream << clang_getCString(str);
    clang_disposeString(str);
    return stream;
}

inline std::string cx2str(const CXString& str) {
    if (!str.data)
        return {};
    std::string stdstr = clang_getCString(str);
    clang_disposeString(str);
    return stdstr;
}
}

#endif
