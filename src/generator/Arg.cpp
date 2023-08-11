#include "Arg.h"

namespace solgen {
std::string argsToString(const Args &args, ArgIgnoreOption ignore) {
    if (args.empty())
        return {};

    auto it = args.begin();

    auto typeStr = [&]() -> std::string {
        if (ignore == ArgIgnoreOption::Type) return "";
        return it->type.getCanonicalName();
    };

    auto nameStr = [&]() -> std::string {
        if (ignore == ArgIgnoreOption::Name) return "";
        if (ignore == ArgIgnoreOption::Type) return it->name;
        return ' ' + it->name;
    };

    std::string result = typeStr() + nameStr();
    ++it;

    while (it != args.end()) {
        result += ", " + typeStr() + nameStr();
        ++it;
    }

    return result;
}
}
