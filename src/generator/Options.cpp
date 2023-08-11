#include "Options.h"

namespace solgen {
void Options::setVal(const std::string &key, const std::string &value) {
    if (key == Options::Include) {
        if (auto it = options.find(Options::Include); it == options.end()) {
            options[Options::Include] = std::any(Options::Include_t {value});
        } else {
            std::any_cast<Options::Include_t&>(options[Options::Include]).insert(value);
        }
    } else if (key == Options::Dep || key == Options::Impl) {
        static_assert(std::is_same_v<Options::Dep_t, Options::Impl_t>);

        using T = std::forward_list<std::string>;

        if (auto it = options.find(key); it == options.end()) {
            options[key] = std::any(T {value});
        } else {
            std::any_cast<T&>(options[key]).emplace_front(value);
        }
    } else if (key == Options::IgnoreName) {
        if (auto it = options.find(key); it == options.end()) {
            options[key] = std::any(IgnoreName_t {std::regex(value)});
        } else {
            std::any_cast<IgnoreName_t&>(options[key]).emplace_front(value);
        }
    } else {
        options[key] = std::any(value);
    }
}

bool Options::isSwitcher(std::string_view key) {
    return key == Ignore || key == ExplicitCast;
}
}
