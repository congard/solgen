#ifndef SOL2_GENERATOR_FILEUTILS_H
#define SOL2_GENERATOR_FILEUTILS_H

#include <string>
#include <string_view>
#include <bits/types/time_t.h>

#include "CmdOptions.h"
#include "types.h"

namespace solgen {
bool exists(std::string_view path);
time_t getModificationTime(std::string_view path);
void setModificationTime(std::string_view path, time_t time);
void writeStr(std::string_view path, std::string_view data);
void mkdirs(const std::string &path);
std::string getRelativePath(std::string_view path);
std::string getParentDir(std::string_view file);
std::string getFileName(std::string_view file);
std::string join(std::string_view p1, std::string_view p2);
std::string getOutputPath(std::string_view outputDir, std::string_view absPath, std::string_view ext);
bool shouldBeRegenerated(const File &file, const CmdOptions &options);
}

#endif // SOL2_GENERATOR_FILEUTILS_H
