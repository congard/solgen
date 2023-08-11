#include "matches.h"

namespace solgen {
bool matches(const std::regex &filter, const Type &type) {
    std::string typeName = type.getName();
    return matches(filter, typeName);
}

bool matches(const std::regex &filter, const std::string &str) {
    return std::sregex_iterator(str.begin(), str.end(), filter) != std::sregex_iterator();
}
}
