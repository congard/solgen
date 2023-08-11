#ifndef SOL2_GENERATOR_PARSER_H
#define SOL2_GENERATOR_PARSER_H

#include <unordered_map>
#include <vector>
#include <regex>

#include <clang-c/Index.h>

#include "generator/Class.h"
#include "Type.h"
#include "types.h"
#include "CmdOptions.h"
#include "Conf.h"

namespace solgen {
// Parse Tree Builder
class Parser {
public:
    explicit Parser(const CmdOptions &options, const Conf &conf);
    ~Parser();

    void init();
    void parse(const File &file);

    void addParam(const std::string &key, const std::string &value) {
        m_params.emplace_back(key, value);
    }

    void setFilter(const std::regex &filter) {
        m_filter = filter;
    }

    auto& getAllClasses() const {
        return m_allClasses;
    }

    auto& getEnums() const {
        return m_enums;
    }

    auto& getCmdOptions() const {
        return m_options;
    }

private:
    static CXChildVisitResult visitor(CXCursor c, CXCursor parent, CXClientData clientData);
    static CXChildVisitResult classVisitor(CXCursor c, CXCursor parent, CXClientData clientData);

    void visitClass(CXCursor c);

private:
    std::unordered_map<Type, Class> m_classes;

    // all classes, i.e. first-level classes and its children
    std::unordered_map<Type, Class*> m_allClasses;

    std::unordered_map<Type, Enum> m_enums;
    std::vector<std::pair<std::string, std::string>> m_params;
    std::regex m_filter;
    const CmdOptions &m_options;
    const Conf &m_conf;

private:
    CXIndex m_index;
    std::forward_list<CXTranslationUnit> m_units;
    std::vector<const char*> m_args;
};
}

#endif //SOL2_GENERATOR_PARSER_H
