#ifndef SOL2_GENERATOR_CONF_H
#define SOL2_GENERATOR_CONF_H

#include <unordered_map>

#include "generator/Options.h"
#include "types.h"

namespace solgen {
class Conf {
public:
    Conf() = default;

    void load(const File &file);
    const Options* getOptions(const std::string &canonicalName) const;

private:
    std::unordered_map<std::string, Options> m_options;
};
}

#endif // SOL2_GENERATOR_CONF_H
