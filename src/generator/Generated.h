#ifndef SOL2_GENERATOR_GENERATED_H
#define SOL2_GENERATOR_GENERATED_H

#include <set>
#include "../types.h"

namespace solgen {
struct Generated {
    std::set<File> sourceIncludes;
    std::string sourceDeclarations;
    std::string source;
};
}

#endif //SOL2_GENERATOR_GENERATED_H
