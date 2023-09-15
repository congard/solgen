#include "FileUtils.h"

#include <filesystem>

namespace fs = std::filesystem;

namespace solgen {
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
    auto outSourceTime = fs::last_write_time(getOutputPath(options.outputDir, file, "cpp"));
    return options.regenerate || fs::last_write_time(file) != outSourceTime;
}
}
