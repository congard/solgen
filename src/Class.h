#ifndef SOL2_GENERATOR_CLASS_H
#define SOL2_GENERATOR_CLASS_H

#include <unordered_map>
#include <forward_list>

#include "Arg.h"
#include "Enum.h"
#include "Field.h"
#include "Function.h"
#include "GenOptions.h"
#include "Access.h"
#include "Type.h"
#include "types.h"

namespace solgen {
class Class {
public:
    struct Constructor {
        Args args; // NOTE: reversed
        GenOptions options;
        Access access;
    };

    struct Base {
        Class *cl;
        Access access;
    };

    Name name;
    Type type;
    File file;
    File absFile;
    GenOptions options;
    std::forward_list<Base> bases; // first-level bases
    std::forward_list<Constructor> ctors;
    std::forward_list<Enum> enums;
    std::forward_list<Field> fields;
    std::forward_list<Class> classes;
    std::unordered_map<std::string, std::forward_list<Function>> functions;
    bool isAbstract;

public:
    template<typename T>
    bool isIgnore(T &obj) {
        if (obj.options.isIgnore())
            return true;

        // check if object is ignored by name at class level
        // only applicable for fields and functions
        if constexpr (std::is_same_v<T, Field> || std::is_same_v<T, Function>) {
            if (auto it = options.find(GenOptions::IgnoreName); it != options.end()) {
                auto &regexes = std::any_cast<GenOptions::IgnoreName_t&>(it->second);

                for (std::regex &regex : regexes) {
                    if (std::sregex_iterator(obj.name.begin(), obj.name.end(), regex) != std::sregex_iterator()) {
                        return true;
                    }
                }
            }
        }

        return false;
    }
};
}

#endif //SOL2_GENERATOR_CLASS_H
