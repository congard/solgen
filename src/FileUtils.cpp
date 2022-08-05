#include "FileUtils.h"

#include <cstring>
#include <cstdio>
#include <ctime>
#include <utime.h>
#include <sys/stat.h>

#ifdef __linux__
    #include <dirent.h>
    #include <unistd.h>

    #define setCurrentDir chdir
    #define getCurrentDir getcwd
    #define separator '/'
#elif defined(_WIN32)
    #include <Windows.h>

    #define setCurrentDir SetCurrentDirectory
    #define getCurrentDir(buffer, length) GetCurrentDirectory(length, buffer)
    #define separator '\\'
#endif

namespace solgen {
bool exists(std::string_view path) {
#if defined(__linux__)
    FILE *file = fopen(path.data(), "r");

    if (file) 
        fclose(file);

    return file;
#elif defined(_WIN32)
    return GetFileAttributes(m_path.c_str()) != INVALID_FILE_ATTRIBUTES;
#endif
}

time_t getModificationTime(std::string_view path) {
    struct stat fileStat;
    if (stat(path.data(), &fileStat) == -1)
        return 0; // file does not exist
    return fileStat.st_mtime;
}

void setModificationTime(std::string_view path, time_t time) {
    struct stat fileStat;
    stat(path.data(), &fileStat);

    struct utimbuf newTimes;
    newTimes.actime = fileStat.st_atime;
    newTimes.modtime = time;

    utime(path.data(), &newTimes);
}

void writeStr(std::string_view path, std::string_view data) {
    FILE *file = fopen(path.data(), "w");
    fwrite(data.data(), 1, data.size(), file);
    fflush(file);
    fclose(file);
}

void mkdirs(const std::string &path) {
    if (path.empty() || exists(path))
        return;

    if (auto pos = path.find_last_of('/'); pos != std::string::npos)
        mkdirs(path.substr(0, path.find_last_of('/')));
    
    mkdir(path.c_str(), 0700);
}

std::string getRelativePath(std::string_view path) {
    if (path.empty())
        return {};
    
    char buff[FILENAME_MAX];

    if (auto cwd = getenv("SOLGEN_CWD"); cwd) {
        strcpy(buff, cwd);
    } else {
        getCurrentDir(buff, FILENAME_MAX);
    }

    std::string_view workingDir {buff};

    // path is not absolute
    if (path.find(workingDir) != 0)
        return path.data();

    // +1 - drop /
    return path.substr(workingDir.size() + 1).data();
}

std::string getParentDir(std::string_view file) {
    auto pos = file.find_last_of('/');
    
    if (pos == std::string::npos)
        pos = file.find_last_of('\\');
    
    if (pos == std::string::npos)
        return {};
    
    return std::string(file.substr(0, pos));
}

std::string getFileName(std::string_view file) {
    auto pos = file.find_last_of('/');
    
    if (pos == std::string::npos)
        pos = file.find_last_of('\\');
    
    if (pos == std::string::npos)
        return file.data();
    
    return std::string(file.substr(pos + 1));
}

inline bool isAbsolutePath(std::string_view path) {
#ifdef __linux__
    return path.find(separator) == 0; // /home/folder
#elif defined(_WIN32)
    return path.size() > 1 && path[1] == ':'; // C:/folder
#endif
}

std::string join(std::string_view p1, std::string_view p2) {
    if (p1.empty())
        return p2.data();

    // if p2 is absolute just return it
    if (isAbsolutePath(p2))
        return p2.data();

    if (p1.back() != '/' && p1.back() != '\\')
        return std::string(p1) + separator + p2.data();

    return std::string(p1) + p2.data();
}

std::string getOutputPath(std::string_view outputDir, std::string_view absPath, std::string_view ext) {
    std::string out = join(outputDir, getRelativePath(absPath));
    auto dotPos = out.find_last_of('.');
    auto slashPos = out.find_last_of('/');
    constexpr auto npos = std::string::npos;

    // file.h -> file
    // file -> file
    // folder/.hidden/file.h -> folder/file
    // folder/.hidden/file -> folder/.hidden/file

    if (dotPos != npos && (slashPos == npos || (slashPos != npos && dotPos > slashPos)))
        out.erase(dotPos); // erase file extension

    out += ".solgen.";
    out += ext;

    return out;
}

bool shouldBeRegenerated(const File &file, const CmdOptions &options) {
    time_t outSourceTime = getModificationTime(getOutputPath(options.outputDir, file, "cpp"));
    return options.regenerate || getModificationTime(file) != outSourceTime;
}
}
