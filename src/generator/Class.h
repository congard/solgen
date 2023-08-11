#ifndef SOL2_GENERATOR_CLASS_H
#define SOL2_GENERATOR_CLASS_H

#include <unordered_map>
#include <forward_list>
#include <list>

#include "Arg.h"
#include "Enum.h"
#include "Field.h"
#include "Function.h"
#include "Access.h"
#include "Generated.h"
#include "Construct.h"
#include "../types.h"

namespace solgen {
class Generator;

class Class: public Construct {
public:
    struct Constructor {
        Args args;
        Options options;
        Access access;
    };

    struct Base {
        Class *clazz;
        Access access;
    };

public:
    void setFile(const std::string &file);
    const std::string& getFile() const;

    void setAbsFile(const std::string &absFile);
    const std::string& getAbsFile() const;

    void addBase(const Base &base);
    auto& getBases() const { return m_bases; }

    Constructor& addConstructor(const Constructor &constructor);
    Enum& addEnum(const Enum &e);
    Field& addField(const Field &field);
    Class& addClass(const Class &clazz);
    Function& addFunction(const Function &func);

    void setAbstract(bool isAbstract);
    bool isAbstract() const;

public:
    /**
     * Generates binding for the class.
     * @return An instance of the `Generated`.
     */
    Generated generate();

    void setGenerator(Generator *generator);

public:
    bool isIgnore() const;

    template<typename T>
    bool isIgnore(const T &obj) const {
        if (obj.getOptions().isIgnore())
            return true;

        // check if object is ignored by name at class level
        // only applicable for fields and functions
        if constexpr (std::is_same_v<T, Field> || std::is_same_v<T, Function>) {
            auto &options = getOptions();

            if (auto it = options.find(Options::IgnoreName); it != options.end()) {
                auto &regexes = std::any_cast<const Options::IgnoreName_t&>(it->second);

                for (const std::regex &regex : regexes) {
                    auto &name = obj.getName();
                    if (std::sregex_iterator(name.begin(), name.end(), regex) != std::sregex_iterator()) {
                        return true;
                    }
                }
            }
        }

        return false;
    }

private:
    std::string m_file;
    std::string m_absFile;
    std::forward_list<Base> m_bases; // first-level bases
    std::forward_list<Constructor> m_ctors;
    std::forward_list<Enum> m_enums;
    std::forward_list<Field> m_fields;
    std::forward_list<Class> m_classes;
    std::unordered_map<std::string, std::forward_list<Function>> m_functions;
    bool m_isAbstract;

private:
    std::string genCtorsCode() const;
    std::string genOperatorsCode() const;
    std::string genBasesCode();
    std::string genFieldsCode() const;
    std::string genFunctionsCode();
    std::string genEnumsCode();
    std::string genDependenciesReg();

    Generator& currentGenerator();
    const Generator& getCurrentGenerator() const;

    void collectPublicBases(const Type &type);

private:
    // single function can produce multiple bindings:
    // for example if it has default arguments
    struct FunBindings {
        Function *fun {};
        std::list<std::string> bindings; // code
    };

    FunBindings genFunBindings(Function &fun, long functionsCount);

private:
    using Overloads = std::forward_list<FunBindings>;

    struct Property {
        std::string getterCode; // Type getXXX() const;
        std::string setterCode; // void setXXX(Type t);
        Function *getter {nullptr};
        Function *setter {nullptr};
    };

    std::string genProperties(const std::unordered_map<Name, Overloads> &overloads);

private:
    Generator *m_generator {};
};
}

#endif //SOL2_GENERATOR_CLASS_H
