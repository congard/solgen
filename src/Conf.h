#ifndef SOL2_GENERATOR_CONF_H
#define SOL2_GENERATOR_CONF_H

#include <unordered_map>

#include "GenOptions.h"
#include "types.h"

namespace solgen {
class Conf {
public:
    Conf() = default;

    void load(const File &file);
    const GenOptions* getOptions(const std::string &canonicalName) const;

private:
    std::unordered_map<std::string, GenOptions> m_options;
};
}

#endif // SOL2_GENERATOR_CONF_H
