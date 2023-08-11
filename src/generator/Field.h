#ifndef SOL2_GENERATOR_FIELD_H
#define SOL2_GENERATOR_FIELD_H

#include "Construct.h"

namespace solgen {
class Field: public Construct {
public:
    Field(const Type &type, std::string name);

    void setConst(bool isConst);
    bool isConst() const;

private:
    bool m_isConst;
};
}

#endif //SOL2_GENERATOR_FIELD_H
