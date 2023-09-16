#ifndef SOL2_GENERATOR_FILEUTILS_H
#define SOL2_GENERATOR_FILEUTILS_H

#include <string>
#include <string_view>

#include "../CmdOptions.h"

namespace solgen::FileUtils {
/**
 * Performs the following transformation:
 * <ul>
 *   <li>file.h -> [outputDir]/file.solgen.[ext]</li>
 *   <li>file -> [outputDir]/file.solgen.[ext]</li>
 *   <li>folder/file.h -> [outputDir]/folder/file.solgen.[ext]</li>
 *   <li>folder/file -> [outputDir]/folder/file.solgen.[ext]</li>
 * </ul>
 * @param outputDir The output directory.
 * @param absPath The file's absolute path.
 * @param ext The output file's extension.
 * @return The transformed string representing the output path
 * in the following format: `[outputDir]/[filename].solgen.[ext]`.
 */
std::string getOutputPath(std::string_view outputDir, std::string_view absPath, std::string_view ext = "cpp");

/**
 * Checks whether the specified file should be regenerated or not.
 * @param file The file to be checked.
 * @param options The command line options.
 * @return `true` if the specified file should be regenerated,
 * `false` otherwise.
 */
bool shouldBeRegenerated(std::string_view file, const CmdOptions &options);

/**
 * Prints the specified path. Note that unix-like separator `/`
 * will be used.
 * @param path The path to print.
 */
void printPath(std::string_view path);

/**
 * The same as FileUtils::printPath(FileUtils::getOutputPath(...))
 * @param outputDir The output directory.
 * @param absPath The file's absolute path.
 * @param ext The output file's extension.
 */
void printOutputPath(std::string_view outputDir, std::string_view absPath, std::string_view ext = "cpp");
}

#endif // SOL2_GENERATOR_FILEUTILS_H
