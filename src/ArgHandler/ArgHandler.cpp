#include "ArgHandler.h"

#include "std_containers.hpp"

bool ArgHandler::Match::isValid() const {
    return !type.empty();
}

ArgHandler::Match ArgHandler::handle(const std::string &type) {
    using Matcher = Match (*)(const std::string&);

    constexpr Matcher matchers[] = {
        &std_containers::match
    };

    for (Matcher matcher : matchers) {
        if (auto match = matcher(type); match.isValid()) {
            return match;
        }
    }

    return {};
}
