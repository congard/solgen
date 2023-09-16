#include "FileUtils.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace solgen::FileUtils {
inline static std::string getRelativePath(std::string_view path) {
    if (path.empty())
        return {};

    std::string workingDir;

    // TODO: describe why do we need this env variable
    if (auto cwd = getenv("SOLGEN_CWD"); cwd) {
        workingDir = cwd;
    } else {
        workingDir = fs::current_path().string();
    }

    // path is not absolute
    if (path.find(workingDir) != 0)
        return path.data();

    // +1 - drop /
    return path.substr(workingDir.size() + 1).data();
}

std::string getOutputPath(std::string_view outputDir, std::string_view absPath, std::string_view ext) {
    using namespace std::string_literals;
    auto out = fs::path(outputDir).append(getRelativePath(absPath));
    out.replace_extension("solgen."s + ext.data());
    return out.string();
}

bool shouldBeRegenerated(std::string_view file, const CmdOptions &options) {
    fs::path outputPath = getOutputPath(options.outputDir, file, "cpp");

    auto outSourceTime = fs::exists(outputPath) ?
            fs::last_write_time(outputPath) :
            fs::file_time_type {};

    return options.regenerate || fs::last_write_time(file) != outSourceTime;
}

void printPath(std::string_view path) {
    std::string p {path};

    size_t pos = p.find('\\');

    while (pos != std::string::npos) {
        p.replace(pos, 1, "/");
        pos = p.find('\\', pos + 1);
    }

    std::cout << p << "\n";
}

void printOutputPath(std::string_view outputDir, std::string_view absPath, std::string_view ext) {
    printPath(getOutputPath(outputDir, absPath, ext));
}
}
