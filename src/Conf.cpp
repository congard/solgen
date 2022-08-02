#include "Conf.h"

namespace solgen {
// Config file structure:
//  # type-canonical-name
//  option_0
//  option_1
//  option_n
//
//  # type-canonical-name
//  option_0
//  option_1
//  option_n

void Conf::load(const File &filePath) {
    FILE *file;
    file = fopen(filePath.c_str(), "r");

    if (!file) {
        fprintf(stderr, "solgen fatal: config file %s not found\n", filePath.c_str());
        exit(-1);
    }

    int c;

    auto skipSpaces = [&]() {
        while (((c = getc(file)) == ' ' || c == '\t') && c != EOF);

        if (c != EOF) {
            ungetc(c, file);
        }
    };

    auto readLine = [&]() {
        std::string line;
        while ((c = getc(file)) != '\n' && c != '#' && c != EOF)
            line += static_cast<std::string::value_type>(c);
        return line;
    };

    auto readWord = [&]() {
        std::string word;
        while ((c = getc(file)) != ' ' && c != '\n' && c != '#' && c != EOF)
            word += static_cast<std::string::value_type>(c);
        return word;
    };
    
    while ((c = getc(file)) != EOF) {
        if (c == '[') { // read type
            skipSpaces();
            auto type = readLine();

            if (type.back() != ']') {
                fprintf(stderr, "solgen fatal: In %s:\n", filePath.c_str());
                fprintf(stderr, "Missing closing bracket:\n\t[%s\n", type.c_str());
                exit(-1);
            }

            type.erase(type.size() - 1);

            GenOptions options;

            // read keys & values
            while (true) {
                skipSpaces();
                auto key = readWord();

                if (key.empty() || key == GenOptions::End)
                    break;

                if (GenOptions::isSwitcher(key)) {
                    options[key] = true;
                } else {
                    skipSpaces();
                    auto value = readLine();
                    options.setVal(key, value);
                }
            }

            m_options[type] = options;
        }
    }
    
    fclose(file);
}

const GenOptions* Conf::getOptions(const std::string &canonicalName) const {
    if (auto it = m_options.find(canonicalName); it != m_options.end())
        return &it->second;
    return nullptr;
}
}
