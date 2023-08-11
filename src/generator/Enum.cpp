#include "Enum.h"
#include "../utils/format.h"

namespace solgen {
constexpr auto enumUsertype = R"(
template<> void registerLuaUsertype<$TYPE>(sol::table &table, void *userdata) {
    if (table["$NAME"].valid())
        return;

    $ENUM
}
)";

constexpr auto declaration = "template<> void registerLuaUsertype<$TYPE>(sol::table &table, void *userdata);";
constexpr auto newEnum = R"($TABLE.new_enum("$NAME", $KEYS);)";

void Enum::setAbsFile(const std::string &absFile) {
    m_absFile = absFile;
}

const std::string& Enum::getAbsFile() const {
    return m_absFile;
}

void Enum::addKey(const std::string &key) {
    m_keys.emplace_front(key);
}

std::string Enum::generateBody(std::string_view table) const {
    std::string code;

    for (auto &key : m_keys) {
        code += "\n        " + format(R"("$NAME", $ETYPE::$KEY,)", {
            {"NAME", key},
            {"ETYPE", getType().getCanonicalName()},
            {"KEY", key}
        });
    }

    code.erase(code.size() - 1); // erase comma

    return format(newEnum, {
        {"TABLE",   table.data()},
        {"NAME",    getName()},
        {"KEYS",    code}
    });
}

Generated Enum::generate() const {
    Generated result;
    result.sourceIncludes.insert(m_absFile);
    result.sourceDeclarations = "\n" + format(declaration, {{"TYPE", getType().getCanonicalName()}});
    result.source = format(enumUsertype, {
        {"TYPE", getType().getCanonicalName()},
        {"NAME", getName()},
        {"ENUM", generateBody("table")}
    });
    return result;
}
}
