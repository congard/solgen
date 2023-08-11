#ifndef SOL2_GENERATOR_FUNCTION_H
#define SOL2_GENERATOR_FUNCTION_H

#include "Arg.h"
#include "Construct.h"

namespace solgen {
class Function: public Construct {
public:
    const Args& getArgs() const;
    Args& args();

    bool hasArgs() const;
    std::size_t getArgsCount() const;

    void setOverride(bool isOverride);
    void setConst(bool isConst);
    void setVolatile(bool isVolatile);
    void setRestrict(bool isRestrict);
    void setStatic(bool isStatic);

    bool isOverride() const;
    bool isConst() const;
    bool isVolatile() const;
    bool isRestrict() const;
    bool isStatic() const;

private:
    Args m_args;
    bool m_isOverride {false};
    bool m_isConst {false};
    bool m_isVolatile {false};
    bool m_isRestrict {false};
    bool m_isStatic {false};
};
}

#endif //SOL2_GENERATOR_FUNCTION_H
