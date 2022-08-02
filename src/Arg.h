#ifndef SOL2_GENERATOR_ARG_H
#define SOL2_GENERATOR_ARG_H

#include <forward_list>

#include "Type.h"
#include "types.h"

namespace solgen {
struct Arg {
    Name name;
    Type type;
    bool hasDefault;
};

using Args = std::forward_list<Arg>;
}

#endif //SOL2_GENERATOR_ARG_H
