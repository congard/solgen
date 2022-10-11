#ifndef SOL2_GENERATOR_ARGHANDLER_H
#define SOL2_GENERATOR_ARGHANDLER_H

#include <string>

class ArgHandler {
public:
    class Match {
    public:
        bool isValid() const;

    public:
        std::string type;
        std::string handler;
    };

public:
    static Match handle(const std::string &type);
};

#endif //SOL2_GENERATOR_ARGHANDLER_H
