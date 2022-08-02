#include "GenOptions.h"

namespace solgen {
void GenOptions::setVal(const std::string &key, const std::string &value) {
    if (key == GenOptions::Include) {
        if (auto it = options.find(GenOptions::Include); it == options.end()) {
            options[GenOptions::Include] = std::any(GenOptions::Include_t {value});
        } else {
            std::any_cast<GenOptions::Include_t&>(options[GenOptions::Include]).insert(value);
        }
    } else if (key == GenOptions::Dep) {
        if (auto it = options.find(GenOptions::Dep); it == options.end()) {
            options[GenOptions::Dep] = std::any(GenOptions::Dep_t {value});
        } else {
            std::any_cast<GenOptions::Dep_t&>(options[GenOptions::Dep]).emplace_front(value);
        }
    } else {
        options[key] = std::any(value);
    }
}

bool GenOptions::isSwitcher(std::string_view key) {
    return key == Ignore || key == ExplicitCast;
}
}
