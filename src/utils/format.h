#ifndef SOL2_GENERATOR_FORMAT_H
#define SOL2_GENERATOR_FORMAT_H

#include <string>
#include <string_view>
#include <map>

namespace solgen {
/**
 * Formats the specified string `str` by finding `$KEY` and replacing
 * it with the value from `values`.
 * @note The key must be $UPPERCASE
 * @param str The string to format.
 * @param values A map containing the key-value pairs for replacement.
 * @return The formatted string.
 */
std::string format(std::string_view str, const std::map<std::string_view, std::string> &values);
}

#endif //SOL2_GENERATOR_FORMAT_H
