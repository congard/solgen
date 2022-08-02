#ifndef SOL2_GENERATOR_GENOPTIONS_H
#define SOL2_GENERATOR_GENOPTIONS_H

#include <forward_list>
#include <set>
#include <string>
#include <map>
#include <any>

#include "types.h"

namespace solgen {
class GenOptions {
public:
    static constexpr auto Ignore = "ignore";
    static constexpr auto ExplicitCast = "explicit_cast";
    static constexpr auto Factory = "factory";
    static constexpr auto Include = "include";
    static constexpr auto PropName = "prop_name";
    static constexpr auto Impl = "impl";
    static constexpr auto Dep = "dep";
    static constexpr auto End = "end";

    using Ignore_t = bool;
    using ExplicitCast_t = bool;
    using Factory_t = std::string;
    using Include_t = std::set<File>;
    using PropName_t = std::string;
    using Impl_t = std::string;
    using Dep_t = std::forward_list<std::string>;

public:
    std::map<std::string, std::any> options;

    inline auto& operator[](const std::string &key) { return options[key]; }

    inline auto find(const std::string &key) { return options.find(key); }
    inline auto begin() noexcept { return options.begin(); }
    inline auto end() noexcept { return options.end(); }

    inline auto find(const std::string &key) const { return options.find(key); }
    inline auto begin() const noexcept { return options.begin(); }
    inline auto end() const noexcept { return options.end(); }

    inline Ignore_t isIgnore() const { return get<Ignore_t>(Ignore, false); }
    inline ExplicitCast_t isExplicitCast() const { return get<ExplicitCast_t>(ExplicitCast, false); }
    inline Factory_t getFactory() const { return get<Factory_t>(Factory, ""); }
    inline PropName_t getPropName() const { return get<PropName_t>(PropName, ""); }
    inline Impl_t getImpl() const { return get<Impl_t>(Impl, ""); }

    template<typename T>
    inline T get(const char *key, T &&def) const {
        if (auto it = options.find(key); it == options.end()) {
            return def;
        } else {
            return std::any_cast<T>(it->second);
        }
    }

    void setVal(const std::string &key, const std::string &value);

    static bool isSwitcher(std::string_view key);
};
}

#endif //SOL2_GENERATOR_GENOPTIONS_H
