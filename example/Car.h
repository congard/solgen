#ifndef SOLGEN_EXAMPLE_CAR
#define SOLGEN_EXAMPLE_CAR

#include "Vehicle.h"

namespace example {
class Car: public Vehicle {
public:
    Car();

public:
    int passengers;
};
}

#endif //SOLGEN_EXAMPLE_CAR