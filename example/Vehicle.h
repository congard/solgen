#ifndef SOLGEN_EXAMPLE_VEHICLE
#define SOLGEN_EXAMPLE_VEHICLE

#include <string_view>
#include <string>

namespace example {
class Vehicle {
public:
    enum Type {
        Air,
        Ground,
        Water,
        Underwater
    };

public:
    Vehicle();
    explicit Vehicle(Type type);

    void setFuel(float fuel);
    float getFuel() const;

    void setName(std::string_view name);
    const std::string& getName() const;

    void setType(Type type);
    Type getType() const;
    
private:
    float m_fuel;
    std::string m_name;
    Type type;
};
}

#endif //SOLGEN_EXAMPLE_VEHICLE
