//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_FILEPATHUTILS_H
#define PRIMITIVE_FS_FILEPATHUTILS_H

#include <string>
#include <vector>
/**
 * Namespace for working with paths in the virtual filesystem.
 */
namespace pfs::path {

    /**
     * Name of the directory item, referencing the directory, containing this item.
     */
    static const char* SELF = ".";
    /**
     * Name of the parent directory to the directory containing this item.
     */
    static const char* PARENT = "..";

    /**
     * Checks if given path is absolute. Returns true, if given path is absolute, otherwise false.
     *
     * @param path path to check
     * @return true if the path is absolute
     */
    bool isAbsolute(const std::string& path) {
        return std::filesystem::path(path).is_absolute();
    }

    /**
     * Parses given path by {@code /} delimiter into individual file names.
     *
     * @param path path to parse into individual file names
     * @return vector of parsed file names
     */
    std::vector<std::string> parsePath(std::string path) {
        std::vector<std::string> tokens;
        std::size_t pos;

        while ((pos = path.find('/')) != std::string::npos) {
            tokens.push_back(path.substr(0, pos));
            path.erase(0, pos + 1);
        }
        /// Pushing back the last token

        if (!path.empty()) {
            tokens.push_back(path);
        }

        return tokens;
    }

    /**
     * Creates absolute or relative path, based on given parameter, from given list of file names. This function does not validate
     * given list of file names or created path, so the result may not make sense, if senseless list of file names is provided. The list of file names
     * is went through from the front.
     *
     * @param tokens list of file names to create path from
     * @param absolute should the path be absolute? default is true
     * @return string representation of created path
     */
    std::string createPathFromTokens(std::vector<std::string> tokens, const bool absolute = true) {
        std::string path;
        if (absolute) {
            path.append("/");
        }

        for (std::vector<std::string>::iterator it = tokens.begin(); it != (tokens.end() - 1); it++) {
            path.append(it->data());
            path.append("/");
        }
        /// Skipping the last one, so we can append only the file name without "/"
        path.append(tokens.back());
        return path;
    }

    /**
     * Creates new absolute path from given absolute path and relative path.
     *
     * @param oldAbsPath old absolute path
     * @param newRelPath relative path to create new path from absolute
     * @return new absolute path
     */
    std::string createAbsolutePath(std::string oldAbsPath, std::string newRelPath) {
        if (!isAbsolute(oldAbsPath) || isAbsolute(newRelPath)) {
            throw std::invalid_argument("Předány nesprávné parametry. Vyžadována absolutní a relativní cesta");
        }

        std::vector<std::string> relPathTokens = parsePath(newRelPath);
        std::vector<std::string> absPathTokens = parsePath(oldAbsPath);

        for (const auto &token : relPathTokens) {
            /// If there is reference to "this folder" in the path, we skip it
            if (token == SELF) {
                continue;
            } else if (token == PARENT) {
                absPathTokens.pop_back();
            } else {
                absPathTokens.push_back(token);
            }
        }

        return createPathFromTokens(absPathTokens);
    }

}
#endif //PRIMITIVE_FS_FILEPATHUTILS_H
