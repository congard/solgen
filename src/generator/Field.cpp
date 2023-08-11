#include "Field.h"

#include <utility>

namespace solgen {
Field::Field(const Type &type, std::string name)
    : Construct(type, std::move(name)),
      m_isConst() {}

void Field::setConst(bool isConst) {
    m_isConst = isConst;
}

bool Field::isConst() const {
    return m_isConst;
}
}
