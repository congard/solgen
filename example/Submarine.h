#ifndef SOLGEN_EXAMPLE_SUBMARINE
#define SOLGEN_EXAMPLE_SUBMARINE

#include "Vehicle.h"

namespace example {
class Submarine: public Vehicle {
public:
    static constexpr float MaxDepth = 10994;

public:
    Submarine();

    void setCurrentDepth(float depth);
    float getCurrentDepth() const;

private:
    float m_depth;
};
}

#endif //SOLGEN_EXAMPLE_SUBMARINE
