#include "Construct.h"

#include <utility>

namespace solgen {
Construct::Construct()
    : m_type(), m_name() {}

Construct::Construct(const Type &type, std::string name)
    : m_type(type), m_name(std::move(name)) {}

void Construct::setName(const std::string &name) {
    m_name = name;
}

const std::string &Construct::getName() const {
    return m_name;
}

void Construct::setType(const Type &type) {
    m_type = type;
}

const Type& Construct::getType() const {
    return m_type;
}

void Construct::setOptions(Options options) {
    m_options = std::move(options);
}

const Options& Construct::getOptions() const {
    return m_options;
}
} // solgen