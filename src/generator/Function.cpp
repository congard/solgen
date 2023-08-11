#include "Function.h"

namespace solgen {
const Args& Function::getArgs() const {
    return m_args;
}

Args& Function::args() {
    return m_args;
}

bool Function::hasArgs() const {
    return !m_args.empty();
}

std::size_t Function::getArgsCount() const {
    return m_args.size();
}

void Function::setOverride(bool isOverride) {
    m_isOverride = isOverride;
}

void Function::setConst(bool isConst) {
    m_isConst = isConst;
}

void Function::setVolatile(bool isVolatile) {
    m_isVolatile = isVolatile;
}

void Function::setRestrict(bool isRestrict) {
    m_isRestrict = isRestrict;
}

void Function::setStatic(bool isStatic) {
    m_isStatic = isStatic;
}

bool Function::isOverride() const {
    return m_isOverride;
}

bool Function::isConst() const {
    return m_isConst;
}

bool Function::isVolatile() const {
    return m_isVolatile;
}

bool Function::isRestrict() const {
    return m_isRestrict;
}

bool Function::isStatic() const {
    return m_isStatic;
}
}
