#include "GenOptions.h"

namespace solgen {
void GenOptions::setVal(const std::string &key, const std::string &value) {
    if (key == GenOptions::Include) {
        if (auto it = options.find(GenOptions::Include); it == options.end()) {
            options[GenOptions::Include] = std::any(GenOptions::Include_t {value});
        } else {
            std::any_cast<GenOptions::Include_t&>(options[GenOptions::Include]).insert(value);
        }
    } else if (key == GenOptions::Dep || key == GenOptions::Impl) {
        static_assert(std::is_same_v<GenOptions::Dep_t, GenOptions::Impl_t>);

        using T = std::forward_list<std::string>;

        if (auto it = options.find(key); it == options.end()) {
            options[key] = std::any(T {value});
        } else {
            std::any_cast<T&>(options[key]).emplace_front(value);
        }
    } else if (key == GenOptions::IgnoreName) {
        if (auto it = options.find(key); it == options.end()) {
            options[key] = std::any(IgnoreName_t {std::regex(value)});
        } else {
            std::any_cast<IgnoreName_t&>(options[key]).emplace_front(value);
        }
    } else {
        options[key] = std::any(value);
    }
}

bool GenOptions::isSwitcher(std::string_view key) {
    return key == Ignore || key == ExplicitCast;
}
}
