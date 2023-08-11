#ifndef SOL2_GENERATOR_CMDOPTIONS_H
#define SOL2_GENERATOR_CMDOPTIONS_H

#include <string>

#include "types.h"

namespace solgen {
// Common options of PTBuilder and Generator
struct CmdOptions {
    bool regenerate {false};
    bool regenerateDerived {false};
    bool printPaths {false};
    File outputDir;
};
}

#endif