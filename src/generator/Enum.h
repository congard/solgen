#ifndef SOL2_GENERATOR_ENUM_H
#define SOL2_GENERATOR_ENUM_H

#include <forward_list>
#include <string_view>

#include "Generated.h"
#include "Construct.h"

namespace solgen {
class Generator;

class Enum: public Construct {
public:
    void setAbsFile(const std::string &absFile);
    const std::string& getAbsFile() const;

    void addKey(const std::string &key);

public:
    /**
     * Generates the body of the binding for the enum.
     * @param table The table name to write binding into.
     * @return The generated binding body as a std::string.
     */
    std::string generateBody(std::string_view table) const;

    /**
     * Generates binding for the enum.
     * @param table The table name to write binding into.
     * @return An instance of the `Generated`.
     */
    Generated generate() const;

private:
    std::string m_absFile;
    std::forward_list<std::string> m_keys;
};
}

#endif //SOL2_GENERATOR_ENUM_H
