#ifndef SOL2_GENERATOR_GENERATOR_H
#define SOL2_GENERATOR_GENERATOR_H

#include "../Parser.h"
#include "../CmdOptions.h"

#include <regex>
#include <set>

namespace solgen {
class Generator {
public:
    explicit Generator(const Parser &parser);

    void generate();

    void setFilter(const std::regex &filter);
    void setOutputNamespace(const std::string &name);

    const std::regex& getFilter() const;
    const CmdOptions& getCmdOptions() const;
    const Parser& getParser() const;

    auto& getTypeBases() {
        return m_completeBases;
    }

private:
    void buildRegenerationMap(const Class &clazz);
    bool isRegenerate(const File &file);

private:
    std::regex m_filter;
    std::string m_outNamespace {"solgen"};
    const Parser &m_builder;

private:
    std::unordered_map<Type, std::set<Type>> m_completeBases;

    struct Sources {
        std::string source;
        std::string sourceDecl;
        std::set<File> sourceIncludes;
    };

    std::unordered_map<File, Sources> m_sources;

    // <file, regenerate?>
    std::unordered_map<File, bool> m_regenerationMap;
};
}

#endif //SOL2_GENERATOR_GENERATOR_H
