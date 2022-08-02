#ifndef SOLGEN_EXAMPLE_SPORT_CAR
#define SOLGEN_EXAMPLE_SPORT_CAR

#include "Car.h"

namespace example {
class SportCar: public Car {
public:
    class HyperEngine {
    public:
        bool isNuclear() const;
    };

public:
    SportCar();

    void enableHyperEngine(int timeout = 1000, int attempts = 5);
    void disableHyperEngine(bool tryToNotExplode = true);

    /// #solgen #ignore
    HyperEngine& getHyperEngine();
};
}

#endif //SOLGEN_EXAMPLE_SPORT_CAR