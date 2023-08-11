#ifndef SOL2_GENERATOR_MATCHES_H
#define SOL2_GENERATOR_MATCHES_H

#include <regex>
#include <string>
#include "../Type.h"

namespace solgen {
/**
 * Checks whether the provided type matches the specified filter.
 * @param filter The regular expression filter to be applied.
 * @param type The type to be checked against the filter.
 * @return `true` if the type matches the filter, `false` otherwise.
 */
bool matches(const std::regex &filter, const Type &type);

/**
 * Checks whether the provided string matches the specified filter.
 * @param filter The regular expression filter to be applied.
 * @param type The string to be checked against the filter.
 * @return `true` if the string matches the filter, `false` otherwise.
 */
bool matches(const std::regex &filter, const std::string &str);
}

#endif //SOL2_GENERATOR_MATCHES_H
