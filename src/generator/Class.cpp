#include <cassert>
#include <list>
#include <ranges>
#include <algorithm>

#include "Class.h"
#include "Generator.h"
#include "../utils/format.h"
#include "../utils/matches.h"
#include "../ArgHandler/ArgHandler.h"

namespace solgen {
namespace templates {
constexpr auto usertype = R"(
template<> void registerLuaUsertype<$TYPE>(sol::table &table, void *userdata) {
    if (table["$NAME"].valid())
        return;

    auto ctors = $CTORSOBJ;
    auto usertype = table.new_usertype<$TYPE>(
        "$NAME", $OPERATORS
        sol::meta_function::construct, ctors,
        sol::call_constructor, ctors,
        sol::base_classes, sol::bases<$BASES>());
    $FIELDS $FUNCTIONS $USERTYPETABLE $ENUMS $REGDEPS
}
)";

constexpr auto declaration = "template<> void registerLuaUsertype<$TYPE>(sol::table &table, void *userdata);";
constexpr auto constructors = "sol::constructors<$CTORS>()";
constexpr auto factories = "sol::factories($FACTORIES)";
constexpr auto readOnlyField = R"(usertype["$FNAME"] = sol::readonly_property(sol::var($TYPE::$FNAME));)";
constexpr auto field = R"(usertype["$FNAME"] = &$TYPE::$FNAME;)";
constexpr auto function = R"(usertype["$FNAME"] = $FPTR;)";
constexpr auto functionPtr = "&$TYPE::$FNAME";
constexpr auto overload = "sol::overload($OVERLOADS)";
constexpr auto defaultParamsOverload = "[]($TYPE &self$ARGS) { $RETself.$FNAME($ANAMES); }";
constexpr auto defaultParamsStaticOverload = "[]($ARGS) { $RET$TYPE::$FNAME($ANAMES); }";
constexpr auto factoryLambda = "[]($ARGS) { return $FACTORY; }";
constexpr auto funcCast = "static_cast<$RETTYPE ($TYPE::*)($ARGS)$QUALIFIERS>(&$TYPE::$FNAME)";
constexpr auto staticFuncCast = "static_cast<$RETTYPE (*)($ARGS)>(&$TYPE::$FNAME)";
constexpr auto property = R"(usertype["$PNAME"] = sol::property($PROP);)";
constexpr auto usertypeTable = R"(auto usertypeTable = table["$NAME"].get<sol::table>();)";
constexpr auto regType = "registerLuaUsertype<$TYPE>($TABLE, userdata);";
}

constexpr auto usertypeTableName = "usertypeTable";

void Class::setFile(const std::string &file) {
    m_file = file;
}

const std::string& Class::getFile() const {
    return m_file;
}

void Class::setAbsFile(const std::string &absFile) {
    m_absFile = absFile;
}

const std::string& Class::getAbsFile() const {
    return m_absFile;
}

void Class::addBase(const Class::Base &base) {
    m_bases.emplace_front(base);
}

Class::Constructor& Class::addConstructor(const Class::Constructor &constructor) {
    return m_ctors.emplace_front(constructor);
}

Enum& Class::addEnum(const Enum &e) {
    return m_enums.emplace_front(e);
}

Field& Class::addField(const Field &field) {
    return m_fields.emplace_front(field);
}

Class& Class::addClass(const Class &clazz) {
    return m_classes.emplace_front(clazz);
}

Function& Class::addFunction(const Function &func) {
    if (auto it = m_functions.find(func.getName()); it != m_functions.end()) {
        return it->second.emplace_front(func);
    } else {
        return (m_functions[func.getName()] = {func}).front();
    }
}

void Class::setAbstract(bool isAbstract) {
    m_isAbstract = isAbstract;
}

bool Class::isAbstract() const {
    return m_isAbstract;
}

class ValuesBuilder {
public:
    void setName(const std::string &name) {
        m_values["NAME"] = name;
    }

    void setType(const Type &type) {
        m_values["TYPE"] = type.getCanonicalName();
    }

    void setConstructors(const std::string &code) {
        m_values["CTORSOBJ"] = code;
    }

    void setOperators(const std::string &code) {
        m_values["OPERATORS"] = code;
    }

    void setBases(const std::string &code) {
        m_values["BASES"] = code;
    }

    void setFields(const std::string &code) {
        m_values["FIELDS"] = code;
    }

    void setFunctions(const std::string &code) {
        m_values["FUNCTIONS"] = code;
    }

    void setEnums(const std::string &code) {
        m_values["ENUMS"] = code;
    }

    void setDependencies(const std::string &code) {
        m_values["REGDEPS"] = code;
    }

    void addUsertypeTable() {
        if (!m_isUsertypeTableAdded) {
            m_isUsertypeTableAdded = true;
            m_values["USERTYPETABLE"] = "\n\n    " +
                    format(templates::usertypeTable, {{"NAME", m_values["NAME"]}}) + "\n";
        }
    }

    const auto& values() const {
        return m_values;
    }

    operator const auto&() const {
        return values();
    }

private:
    std::map<std::string_view, std::string> m_values;
    bool m_isUsertypeTableAdded {false};
};

Generated Class::generate() {
    auto &ptBuilder = getCurrentGenerator().getParser();

    ValuesBuilder valuesBuilder;
    Generated generated;

    // read includes
    auto &options = getOptions();

    if (auto it = options.find(Options::Include); it != options.end()) {
        const auto &includes = std::any_cast<const Options::Include_t&>(it->second);
        generated.sourceIncludes.insert(includes.begin(), includes.end());
    }

    generated.sourceIncludes.insert(m_absFile);

    valuesBuilder.setName(getName());
    valuesBuilder.setType(getType());
    valuesBuilder.setConstructors(genCtorsCode()); // constructors
    valuesBuilder.setOperators(genOperatorsCode()); // operators

    // bases (only public, because we should bind only visible functions)
    collectPublicBases(getType());
    valuesBuilder.setBases(genBasesCode());

    // fields
    valuesBuilder.setFields(genFieldsCode());

    // functions
    valuesBuilder.setFunctions(genFunctionsCode());

    // enums
    {
        auto enumsCode = genEnumsCode();
        valuesBuilder.setEnums(enumsCode);

        if (!enumsCode.empty()) {
            valuesBuilder.addUsertypeTable();
        }
    }

    // reg children & first-level bases
    {
        std::string dependencies;
        auto depCode = genDependenciesReg();

        if (!depCode.empty() && !m_classes.empty())
            valuesBuilder.addUsertypeTable(); // add only in case if children exist

        dependencies += depCode;

        valuesBuilder.setDependencies(dependencies);
    }

    generated.sourceDeclarations += "\n" +
            format(templates::declaration, {{"TYPE", getType().getCanonicalName()}});
    generated.source += format(templates::usertype, valuesBuilder);

    return generated;
}

void Class::setGenerator(Generator *generator) {
    m_generator = generator;
}

std::string Class::genCtorsCode() const {
    if (m_isAbstract)
        return "sol::no_constructor";

    std::string code;
    std::string factory = getOptions().getFactory();
    auto &type = getType();

    for (const Constructor &ctor : m_ctors) {
        if (ctor.access != Access::Public || ctor.options.isIgnore())
            continue;

        if (factory.empty()) { // constructors
            code += type.getCanonicalName() + '(' + argsToString(ctor.args) + "), ";
        } else { // factories
            code += "\n        " + format(templates::factoryLambda, {
                {"ARGS",    argsToString(ctor.args, ArgIgnoreOption::None)},
                {"FACTORY", format(factory, {
                    {"TYPE",   type.getCanonicalName()},
                    {"ANAMES", argsToString(ctor.args, ArgIgnoreOption::Type)}
                })}
            }) + ", ";
        }
    }

    if (code.empty()) {
        if (m_ctors.empty()) { // no ctors specified - exists default
            if (factory.empty()) { // ctor
                code = type.getCanonicalName() + "()";
            } else { // factory
                code += format(templates::factoryLambda, {
                    {"ARGS",    ""},
                    {"FACTORY", format(factory, {{"TYPE", type.getCanonicalName()}, {"ANAMES", ""}})}
                });
            }
        } else { // ctors exist, but non-public or ignored - disable construction
            return "sol::no_constructor";
        }
    } else {
        code.erase(code.size() - 2, 2);
    }

    return factory.empty() ?
        format(templates::constructors, {{"CTORS", code}}) :
        format(templates::factories, {{"FACTORIES", code}});
}

std::string Class::genOperatorsCode() const {
    std::string code;
    auto typeName = getType().getCanonicalName();

    auto addOperator = [&](const std::string &s) {
        code += "\n        " + s + ",";
    };

    // TODO: check overloads & ignores

    if (auto it = m_functions.find("operator=="); it != m_functions.end())
        addOperator("sol::meta_function::equal_to, &" + typeName + "::operator==");
    if (auto it = m_functions.find("operator+"); it != m_functions.end())
        addOperator("sol::meta_function::addition, &" + typeName + "::operator+");
    if (auto it = m_functions.find("operator-"); it != m_functions.end())
        addOperator("sol::meta_function::subtraction, &" + typeName + "::operator-");
    if (auto it = m_functions.find("operator*"); it != m_functions.end())
        addOperator("sol::meta_function::multiplication, &" + typeName + "::operator*");
    if (auto it = m_functions.find("operator/"); it != m_functions.end())
        addOperator("sol::meta_function::division, &" + typeName + "::operator/");
    if (auto it = m_functions.find("operator<"); it != m_functions.end())
        addOperator("sol::meta_function::less_than, &" + typeName + "::operator<");
    if (auto it = m_functions.find("operator[]"); it != m_functions.end()) {
        const Function *index {nullptr};
        const Function *new_index {nullptr};

        for (auto &fun : it->second) {
            if (fun.getOptions().isIgnore())
                continue;

            if (fun.isConst()) {
                index = &fun;
            } else {
                new_index = &fun;
            }
        }

        // TODO: possible duplicate
        auto func_cast = [this](const Function *func) {
            std::string qualifiers;

            if (func->isConst())
                qualifiers += " const";

            if (func->isVolatile())
                qualifiers += " volatile";

            return format(templates::funcCast, {
                {"RETTYPE",    func->getType().getResultType().getCanonicalName()},
                {"TYPE",       getType().getCanonicalName()},
                {"ARGS",       argsToString(func->getArgs())},
                {"QUALIFIERS", qualifiers},
                {"FNAME",      func->getName()}
            });
        };

        if (index) {
            addOperator("sol::meta_function::index, " + func_cast(index));
        }

        if (new_index) {
            addOperator("sol::meta_function::new_index, " + func_cast(new_index));
        }
    }

    return code;
}

std::string Class::genBasesCode() {
    std::string code;
    auto &completeBases = currentGenerator().getTypeBases();

    for (const Type &base : completeBases[getType()])
        code += base.getCanonicalName() + ", ";

    if (!code.empty())
        code.erase(code.size() - 2, 2);

    return code;
}

std::string Class::genFieldsCode() const {
    std::string code;

    for (const Field &fld : m_fields) {
        if (isIgnore(fld))
            continue;

        std::map<std::string_view, std::string> values = {
            {"FNAME", fld.getName()},
            {"TYPE", getType().getCanonicalName()}
        };

        if (fld.isConst()) {
            code += "\n    " + format(templates::readOnlyField, values);
        } else {
            code += "\n    " + format(templates::field, values);
        }
    }

    return code;
}

std::string Class::genFunctionsCode() {
    std::string code;
    std::unordered_map<Name, Overloads> overloads;

    auto isOperator = [](const std::string &name) {
        auto operators = {
            "operator=", "operator==", "operator!=",
            "operator<", "operator<=", "operator>", "operator>=",
            "operator+", "operator-", "operator/", "operator*",
            "operator[]", "operator->", "operator<<", "operator>>"
        };

        return std::any_of(operators.begin(), operators.end(), [&name](auto el) { return name == el; });
    };

    for (auto &funNameData : m_functions) {
        const std::string &funName = funNameData.first;
        std::forward_list<Function> &functions = funNameData.second;

        if (isOperator(funName))
            continue;

        long functionsCount = std::distance(functions.begin(), functions.end());

        overloads[funName] = Overloads {};

        for (Function &fun : functions) {
            // Note: if the Function has override attrib, we can skip code generation for
            // this function, in case if overriden function in base class is public too;
            // in theory it can reduce lua usertype table size
            if (!isIgnore(fun)) {
                overloads[funName].emplace_front(genFunBindings(fun, functionsCount));
            }
        }

        std::list<std::string> overloadsList;

        for (FunBindings &funVariations : overloads[funName]) {
            for (std::string &ovCode : funVariations.bindings) {
                overloadsList.emplace_front(ovCode);
            }
        }

        auto overloadsToString = [&]() -> std::string {
            if (overloadsList.size() == 1)
                return *overloadsList.begin();

            std::string result;

            for (auto &o : overloadsList)
                result += "\n        " + o + ',';

            return result.erase(result.size() - 1);
        };

        if (overloadsList.size() > 1) {
            code += "\n    " + format(templates::function, {
                {"FNAME", funName},
                {"FPTR", format(templates::overload, {{"OVERLOADS", overloadsToString()}})}
            });
        } else if (overloadsList.size() == 1) {
            code += "\n    " + format(templates::function, {
                {"FNAME", funName},
                {"FPTR", overloadsToString()}
            });
        }
    }

    code += genProperties(overloads);

    return code;
}

std::string Class::genEnumsCode() {
    std::string code;

    for (auto &e : m_enums) {
        if (e.getOptions().isIgnore())
            continue;
        code += "\n    " + e.generateBody(usertypeTableName);
    }

    return code;
}

std::string Class::genDependenciesReg() {
    std::string dependencies;

    auto genCode = [&](const std::string &typeName, const std::string &table) {
        dependencies += "\n    " + format(templates::regType, {
            {"TYPE",    typeName},
            {"TABLE",   table}
        });
    };

    auto gen = [&](Class &dep, const std::string &table) {
        if (dep.isIgnore())
            return;
        genCode(dep.getType().getCanonicalName(), table);
    };

    for (Class &child : m_classes)
        gen(child, usertypeTableName);

    for (auto [base, access] : m_bases) {
        if (access == Access::Public) {
            gen(*base, "table");
        }
    }

    auto &options = getOptions();

    if (auto it = options.find(Options::Dep); it != options.end()) {
        for (const auto &dep : std::any_cast<const Options::Dep_t&>(it->second)) {
            genCode(dep, "table");
        }
    }

    return dependencies.empty() ? "" : '\n' + dependencies;
}

Generator& Class::currentGenerator() {
    assert(m_generator != nullptr);
    return *m_generator;
}

const Generator& Class::getCurrentGenerator() const {
    assert(m_generator != nullptr);
    return *m_generator;
}

void Class::collectPublicBases(const Type &type) {
    auto &bases = currentGenerator().getTypeBases();
    auto &classes = getCurrentGenerator().getParser().getAllClasses();

    if (bases.find(type) != bases.end())
        return; // already found

    for (auto [base, access] : classes.at(type)->getBases()) {
        if (access != Access::Public)
            continue;

        collectPublicBases(base->getType());
        bases[type].insert(base->getType());

        auto &baseBases = bases[base->getType()];
        bases[type].insert(baseBases.begin(), baseBases.end());
    }
}

Class::FunBindings Class::genFunBindings(solgen::Function &fun, long functionsCount) {
    // if function has default arguments, we should create bindings
    // for each variation, e.g:
    //  void foo(int a = 0, float b = 1, double c = 2)
    // generates to:
    //  static_cast<rettype (T::*)(int, float, double)>(&T::foo),
    //  [](T &self) { self.foo(); }
    //  [](T &self, int a) { self.foo(a); }
    //  [](T &self, int a, float b) { self.foo(a, b); }

    // if function has arguments of type
    //  std::vector<T>& / const std::vector<T>&
    //  std::map<T>& / const std::map<T>&
    //  std::unordered_map<T>& / const std::unordered_map<T>&
    // they should be replaced with
    //  std::vector<T> / std::map<T> / std::unordered_map<T>
    // e.g:
    //  void foo(const std::vector<int> &v)
    // becomes:
    //  [](T &self, std::vector<int> v) { self.foo(v); }

    // if function is 'clean' - just bind it

    FunBindings funBindings;
    funBindings.fun = &fun;

    // if there exists custom implementation - use it and
    // skip code generation
    if (auto it = fun.getOptions().find(Options::Impl); it != fun.getOptions().end()) {
        for (auto &impl : std::any_cast<const Options::Impl_t&>(it->second))
            funBindings.bindings.emplace_front(impl);
        return funBindings;
    }

    struct ProcessedArg {
        Name name;
        std::string type;
        std::string handler;
        bool hasDefault {};
    };

    std::list<ProcessedArg> newArgs;
    std::string argsStr;
    std::string argsNames;
    bool modifiedArgs = false;

    for (const Arg &arg : fun.getArgs()) {
        if (auto match = ArgHandler::handle(arg.type.getCanonicalName()); match.isValid()) {
            ProcessedArg argGood;
            argGood.name = arg.name;
            argGood.type = match.type;
            argGood.handler = match.handler;
            argGood.hasDefault = arg.hasDefault;
            newArgs.emplace_back(argGood);
            modifiedArgs = true;
        } else {
            ProcessedArg argGood;
            argGood.name = arg.name;
            argGood.type = arg.type.getCanonicalName();
            argGood.hasDefault = arg.hasDefault;
            newArgs.emplace_back(argGood);
        }
    }

    // just adds overload to funVariations
    auto addOverload = [&](const std::string &overload) {
        funBindings.bindings.emplace_front(overload);
    };

    auto addLambdaOverload = [&]() {
        addOverload(format(templates::defaultParamsOverload, {
            {"TYPE",    getType().getCanonicalName()},
            {"ARGS",    argsStr.empty() ? "" : ", " + argsStr},
            {"FNAME",   fun.getName()},
            {"ANAMES",  argsNames.substr(0, argsNames.size() - 2)},
            {"RET", fun.getType().getResultType().getName() == "void" ? "" : "return "}
        }));
    };

    auto addLambdaStaticOverload = [&]() {
        addOverload(format(templates::defaultParamsStaticOverload, {
            {"TYPE",   getType().getCanonicalName()},
            {"ARGS",   argsStr},
            {"FNAME",  fun.getName()},
            {"ANAMES", argsNames.substr(0, argsNames.size() - 2)},
            {"RET", fun.getType().getResultType().getName() == "void" ? "" : "return "}
        }));
    };

    // add overloading if function has default arguments
    for (ProcessedArg &arg : newArgs) {
        if (arg.hasDefault) {
            if (!fun.isStatic()) {
                addLambdaOverload();
            } else {
                addLambdaStaticOverload();
            }
        }

        argsStr += (argsStr.empty() ? "" : ", ") + arg.type + ' ' + arg.name;
        argsNames += (arg.handler.empty() ? arg.name : format(arg.handler, {{"arg", arg.name}})) + ", ";
    }

    if (functionsCount == 1 && !getOptions().isExplicitCast() && !fun.getOptions().isExplicitCast()) {
        if (!modifiedArgs) { // pointer to function without casting
            addOverload(format(templates::functionPtr, {
                {"TYPE", getType().getCanonicalName()},
                {"FNAME", fun.getName()}
            }));
        } else { // lambda
            addLambdaOverload();
        }
    } else {
        if (!modifiedArgs) { // pointer to function with casting
            std::string qualifiers;

            if (fun.isConst())
                qualifiers += " const";

            if (fun.isVolatile())
                qualifiers += " volatile";

            std::map<std::string_view, std::string> values = {
                {"RETTYPE",    fun.getType().getResultType().getCanonicalName()},
                {"TYPE",       getType().getCanonicalName()},
                {"ARGS",       argsToString(fun.getArgs())},
                {"FNAME",      fun.getName()},
                {"QUALIFIERS", qualifiers}
            };

            if (!fun.isStatic()) {
                addOverload(format(templates::funcCast, values));
            } else {
                addOverload(format(templates::staticFuncCast, values));
            }
        } else { // lambda
            if (!fun.isStatic()) {
                addLambdaOverload();
            } else {
                addLambdaStaticOverload();
            }
        }
    }

    return funBindings;
}

std::string Class::genProperties(const std::unordered_map<Name, Overloads> &overloads) {
    std::string code;
    std::unordered_map<Name, Property> properties;

    // properties: phase 1: getters
    for (auto & [name, funOverloadsList] : overloads) {
        if (name.size() <= 3 || std::string_view(name.c_str(), 3) != "get")
            continue;

        // now, find non-void non-static function with 0 arguments
        std::ranges::for_each(funOverloadsList | std::views::filter([this](auto &funVariations) {
            Function *fun = funVariations.fun;
            auto retTypeName = fun->getType().getResultType().getName();
            return !isIgnore(*fun) && !fun->isStatic() && !fun->hasArgs() && retTypeName != "void";
        }), [&](const FunBindings &funVariations) {
            std::string propName = name.substr(3);
            propName[0] = std::tolower(propName[0]);

            Property prop;
            prop.getterCode = *funVariations.bindings.begin();
            prop.getter = funVariations.fun;

            properties[propName] = prop;
        });
    }

    // phase 2: find setters
    for (auto & [name, funOverloadsList] : overloads) {
        if (name.size() <= 3 && std::string_view(name.c_str(), 3) != "set")
            continue;

        // now, find non-static void function with 1 argument of type properties[name].type
        std::ranges::for_each(funOverloadsList | std::views::filter([this](auto &funVariations) {
            Function *fun = funVariations.fun;
            auto retTypeName = fun->getType().getResultType().getName();
            return !isIgnore(*fun) && !fun->isStatic() && fun->getArgsCount() == 1 && retTypeName == "void" &&
                   funVariations.bindings.size() == 1;
        }), [&](const FunBindings &funVariations) {
            Function *fun = funVariations.fun;

            std::string propName = name.substr(3);
            propName[0] = static_cast<char>(std::tolower(propName[0]));

            if (auto it = properties.find(propName); it != properties.end()) {
                Property &prop = it->second;

                const Type &setterType = fun->getArgs().front().type;
                Type getterType = prop.getter->getType().getResultType();

                Name setterPropName = fun->getOptions().getPropName();
                Name getterPropName = prop.getter->getOptions().getPropName();

                if (setterType == getterType ||
                    (setterPropName == getterPropName && !setterPropName.empty()))
                {
                    prop.setterCode = *funVariations.bindings.begin();
                    prop.setter = fun;
                }
            }
        });
    }

    // phase 3: find out prop name & gen code
    for (auto &p : properties) {
        Property &prop = p.second;

        Name getterPropName = prop.getter->getOptions().getPropName();
        Name setterPropName = prop.setter ? prop.setter->getOptions().getPropName() : "";
        Name propName;

        if (!getterPropName.empty() && !setterPropName.empty()) {
            if (getterPropName == setterPropName) {
                propName = getterPropName;
            } else {
                fprintf(stderr, "Warning: property names for setter and getter are not the same:\n");
                fprintf(stderr, "\t%s: %s\n", prop.getterCode.c_str(), getterPropName.c_str());
                fprintf(stderr, "\t%s: %s\n", prop.setterCode.c_str(), setterPropName.c_str());
            }
        } else if (!getterPropName.empty() && setterPropName.empty()) {
            propName = getterPropName;
        } else if (getterPropName.empty() && !setterPropName.empty()) {
            propName = setterPropName;
        }

        if (propName.empty())
            propName = p.first;

        code += "\n    " + format(templates::property, {
            {"PNAME", propName},
            {"PROP", prop.getterCode + (prop.setterCode.empty() ? "" : ", " + prop.setterCode)}
        });
    }

    return code;
}

bool Class::isIgnore() const {
    return getOptions().isIgnore() || !matches(getCurrentGenerator().getFilter(), getType());
}
}
