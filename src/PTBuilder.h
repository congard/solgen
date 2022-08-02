#ifndef SOL2_GENERATOR_PTBUILDER_H
#define SOL2_GENERATOR_PTBUILDER_H

#include <unordered_map>
#include <algorithm>
#include <vector>
#include <regex>

#include <clang-c/Index.h>

#include "Class.h"
#include "Type.h"
#include "types.h"
#include "CmdOptions.h"
#include "Conf.h"

namespace solgen {
// Parse Tree Builder
class PTBuilder {
public:
    explicit PTBuilder(const CmdOptions &options, const Conf &conf);
    ~PTBuilder();

    void init();
    void parse(const File &file);

public:
    std::unordered_map<Type, Class> classes;
    std::unordered_map<Type, Class*> allClasses;
    std::vector<std::pair<std::string, std::string>> params;
    std::regex filter;
    const CmdOptions &options;
    const Conf &conf;

private:
    CXIndex m_index;
    std::forward_list<CXTranslationUnit> m_units;
    std::vector<const char*> m_args;
};
}

#endif //SOL2_GENERATOR_PTBUILDER_H
