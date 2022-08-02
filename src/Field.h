#ifndef SOL2_GENERATOR_FIELD_H
#define SOL2_GENERATOR_FIELD_H

#include "GenOptions.h"
#include "Type.h"
#include "types.h"

namespace solgen {
struct Field {
    Name name;
    Type type;
    GenOptions options;
    bool isConst;
};
}

#endif //SOL2_GENERATOR_FIELD_H
