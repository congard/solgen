#include "PTBuilder.h"
#include "SolGen.h"
#include "Conf.h"

#include <cstring>
#include <iostream>

using namespace std;
using namespace solgen;

constexpr auto args = R"(
Available arguments:

    --help
        print this message

    --help-options
        show available SolGen options

    --clang-args <args>
        must be specified at the end

    --input <strs: files list>

    --includes <strs: files list>
        include paths

    --output-dir <str: output_dir>

    --output-namespace <str: namespace>
        namespace in which generated code will be placed
        default: solgen

    --namespace-filter <str: filter>
        PTBuilder filter

    --type-filter <str: filter>
        SolGen filter

    --conf <str: path to config file>
        Options conf. Type --help-options for more info

    --regenerate
        if specified, all files will be regenerated

    --regenerate-derived
        derived classes will be regenerated in case if one of the bases was regenerated

    --print-paths
        print output files paths, regardless of whether the file was generated or not
)";

constexpr auto options = R"(
Available options:

    ignore
        ignore symbol (all)

    ignoreName
        ignore function/field by name with provided regex (class)
    
    explicit_cast
        use static_cast instead of just taking address of function (class, function)
    
    factory
        use specified factory instead of regular ctors (class)
        params: TYPE - type name, ANAMES - argument names
        example: std::make_shared<$TYPE>($ANAMES)
    
    include
        include specified file (class)
        example: include memory
    
    prop_name
        set property name. Can be used to link getter & setter of different types (function)
    
    impl
        bind custom implementation (function)
    
    dep
        specified dependency will be registered as Lua type,
        i.e. will be called registerLuaUsertype<dep>(table) (class)
    
    end
        end options block (in some cases it's mandatory,
        e.g. if solgen options in the middle of a doc comment) (all)
)";

int main(int argc, char *argv[]) {
    if (argc == 1) {
        fprintf(stderr, "fatal error: no input files\n");
        exit(-1);
    }

    std::forward_list<File> inputs;

    CmdOptions options;
    Conf conf;
    PTBuilder builder(options, conf);
    SolGen solGen(options);

    for (int i = 1; i < argc; ++i) {
        auto isArg = [argv, i](const char *arg) { return strcmp(argv[i], arg) == 0; };

        auto readVal = [argc, argv, &i]() {
            if (++i < argc) {
                return argv[i];
            } else {
                fprintf(stderr, "Error: not enough arguments\n%s\n", args);
                exit(-1);
            }
        };

        if (isArg("--help")) {
            printf("%s\n", args);
        } else if (isArg("--help-options")) {
            printf("%s\n", ::options);
        } else if (isArg("--clang-args")) {
            for (int j = i + 1; j < argc; ++j) {
                std::pair<std::string, std::string> param;
                param.first = argv[j];
                param.second = (j + 1 < argc && argv[j + 1][0] != '-') ? argv[++j] : "";
                builder.params.emplace_back(param);
            }
        } else if (isArg("--output-dir")) {
            auto val = readVal();
            options.outputDir = val;
        } else if (isArg("--output-namespace")) {
            auto val = readVal();
            solGen.outNamespace = val;
        } else if (isArg("--input")) {
            ++i;
            while (i < argc && argv[i][0] != '-')
                inputs.emplace_front(argv[i++]);
            --i; // will be increased in for loop body
        } else if (isArg("--includes")) {
            ++i;

            while (i < argc && argv[i][0] != '-') {
                std::pair<std::string, std::string> param;
                param.first = "-I";
                param.second = argv[i++];
                builder.params.emplace_back(param);
            }

            --i; // will be increased in for loop body
        } else if (isArg("--namespace-filter")) {
            builder.filter = std::regex(readVal());
        } else if (isArg("--type-filter")) {
            solGen.filter = std::regex(readVal());
        } else if (isArg("--conf")) {
            conf.load(readVal());
        } else if (isArg("--regenerate")) {
            options.regenerate = true;
        } else if (isArg("--regenerate-derived")) {
            options.regenerateDerived = true;
        } else if (isArg("--print-paths")) {
            options.printPaths = true;
        }
    }

    // builder.filter = std::regex("zoo");
    // builder.params = {{"-I", "/usr/lib64/clang/14.0.0/include"}};
    // builder.outputDir = "/mnt/common/Development/linux/Projects/LuaTests/LuaZoo";
    builder.init();

    for (File &file : inputs)
        builder.parse(file);
    
    // solGen.filter = std::regex("zoo::.+");
    // solGen.outputDir = "/mnt/common/Development/linux/Projects/LuaTests/LuaZoo";
    solGen.generate(builder);
}
