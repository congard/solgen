#ifndef SOL2_GENERATOR_FUNCTION_H
#define SOL2_GENERATOR_FUNCTION_H

#include <forward_list>

#include "Arg.h"
#include "GenOptions.h"
#include "Type.h"
#include "types.h"

namespace solgen {
struct Function {
    Type type; // return type
    Name name;
    Args args; // NOTE: reversed
    GenOptions options;
    bool isOverride {false};
    bool isConst {false};
    bool isVolatile {false};
    bool isRestrict {false};
    bool isStatic {false};
};
}

#endif //SOL2_GENERATOR_FUNCTION_H
