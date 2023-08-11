#ifndef SOL2_GENERATOR_ARG_H
#define SOL2_GENERATOR_ARG_H

#include <vector>

#include "../Type.h"

namespace solgen {
struct Arg {
    std::string name;
    Type type;
    bool hasDefault;
};

using Args = std::vector<Arg>;

enum class ArgIgnoreOption {
    None, Name, Type
};

/**
 * Generates a comma-separated list of arguments:
 * $TYPE $NAME, ...
 * @param args The arguments to be included in the list
 * @param ignore Whether to ignore certain arguments
 * @return The generated comma-separated list of arguments
 */
std::string argsToString(const Args &args, ArgIgnoreOption ignore = ArgIgnoreOption::Name);
}

#endif //SOL2_GENERATOR_ARG_H
