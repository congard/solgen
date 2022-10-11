#include "ArgHandler.h"

#include <regex>

namespace std_containers {
ArgHandler::Match match(const std::string &inType) {
    for (std::string type : {"vector", "map", "unordered_map"}) {
        // (?:const)? *(?:std::)?vector<(.+)> *&
        std::regex rx("(?:const)? *(?:std::)?" + type + "<(.+)> *&");
        std::sregex_iterator it = std::sregex_iterator(inType.begin(), inType.end(), rx);

        if (it != std::sregex_iterator()) {
            return {.type = "std::" + type + '<' + (*it)[1].str() + '>'};
        }
    }

    return {};
}
}