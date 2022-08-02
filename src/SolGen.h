#ifndef SOL2_GENERATOR_SOLGEN_H
#define SOL2_GENERATOR_SOLGEN_H

#include "PTBuilder.h"
#include "CmdOptions.h"

#include <forward_list>
#include <regex>
#include <set>

namespace solgen {
class SolGen {
public:
    explicit SolGen(const CmdOptions &options);

    void generate(const PTBuilder &builder);

public:
    std::regex filter;
    std::string outNamespace {"solgen"};
    const CmdOptions &options;

private:
    std::string genCtorsCode(Class *cl) const;
    std::string genOperatorsCode(Class *cl) const;
    std::string genBasesCode(const Type &type);
    std::string genFieldsCode(Class *cl) const;
    std::string genFunctionsCode(Class *cl) const;
    std::string genDependenciesReg(Class *cl) const;

private:
    std::unordered_map<Type, std::set<Type>> m_completeBases;

    struct Sources {
        std::string source;
        std::string sourceDecl;
        std::set<File> sourceIncludes;
    };

    std::unordered_map<File, Sources> m_sources;
};
}

#endif //SOL2_GENERATOR_SOLGEN_H
