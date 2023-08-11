#ifndef SOL2_GENERATOR_CONSTRUCT_H
#define SOL2_GENERATOR_CONSTRUCT_H

#include <string>
#include "Options.h"
#include "../Type.h"

namespace solgen {
/**
 * Base class for `Class`, `Enum`, `Function` and `Field`.
 */
class Construct {
public:
    Construct();
    Construct(const Type &type, std::string name);

    void setName(const std::string &name);
    const std::string& getName() const;

    void setType(const Type &type);
    const Type& getType() const;

    void setOptions(Options options);
    const Options& getOptions() const;

private:
    Type m_type;
    std::string m_name;
    Options m_options;
};
} // solgen

#endif //SOL2_GENERATOR_CONSTRUCT_H
