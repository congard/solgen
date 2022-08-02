#ifndef SOL2_GENERATOR_ENUM_H
#define SOL2_GENERATOR_ENUM_H

#include <forward_list>

#include "GenOptions.h"
#include "Type.h"
#include "types.h"

namespace solgen {
struct Enum {
    Type type;
    std::forward_list<Name> keys;
    GenOptions options;
};
}

#endif //SOL2_GENERATOR_ENUM_H
