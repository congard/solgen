#include "format.h"

namespace solgen {
std::string format(std::string_view str, const std::map<std::string_view, std::string> &values) {
    std::string result;
    size_t pos = 0;
    size_t startPos = 0;
    char buff[64];
    int size = 0;

    while ((pos = str.find('$', pos)) != std::string::npos) {
        ++pos;

        while (pos < str.size() && str[pos] >= 'A' && str[pos] <= 'Z')
            buff[size++] = str[pos++];

        buff[size] = '\0';

        if (values.find(buff) != values.end()) {
            result += str.substr(startPos, pos - size - 1 - startPos);
            result += values.at(buff);
        }

        startPos = pos;
        size = 0;

        if (startPos == str.size() - 1)
            break;
    }

    if (startPos < str.size())
        result += str.substr(startPos);

    return result;
}
}