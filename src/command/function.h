//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_FUNCTION_H
#define PRIMITIVE_FS_FUNCTION_H

/**
 * Namespace of functions that can be executed upon the filesystem.
 */
#include <vector>
#include <string>
#include "../fs/FileSystem.h"

namespace fnct {
    /**
     * Formats the data file of the filesystem according to given size, based of the default setting of the {@code fs::Superblock}.
     *
     * @param parameters requires requested disk size as first parameter, others are ignored
     * @param fileSystem fileSystem who's datafile has to be formatted
     */
    void format(const std::vector<std::string>& parameters, FileSystem* fileSystem);
    /**
     * Copies a file on given path from the real hard-drive into the path in the virtual file system. If either on of the paths
     * doesn't exist or error while copying files occur, prints an error.
     *
     * @param parameters requires two parameters - existing path in the real hard-drive and existing path in the virtual file system
     * @param fileSystem virtual file system to copy the file into
     */
    void incp(const std::vector<std::string>& parameters, FileSystem* fileSystem);
    /**
     * Prints current working directory. Doesn't require any parameters.
     *
     * @param parameters no parameters required, any given parameter will be ignored
     * @param fileSystem file system which current working directory we want to print of
     */
    void pwd(const std::vector<std::string>& parameters, FileSystem* fileSystem);
    /**
     * Changes current working directory to given directory.
     *
     * @param parameters requires one parameter - existing directory in virtual file system
     * @param fileSystem file system which we want to change the working directory of
     */
    void cd(const std::vector<std::string>& parameters, FileSystem* fileSystem);
    /**
     * Prints all files in given directory.
     *
     * @param parameters requires one parameter - existing directory
     * @param fileSystem file system which we want to access
     */
    void ls(const std::vector<std::string>& parameters, FileSystem* fileSystem);

    /**
     * Removes file at the end of given path.
     *
     * @param parameters requires one parameter - path to existing file which is not directory
     * @param fileSystem file system which we want to access
     */
    void rm(const std::vector<std::string>& parameters, FileSystem* fileSystem);

    /**
     * Prints contents of a file into the console.
     *
     * @param parameters requires one parameter - path to existing file which is not a directory
     * @param fileSystem system which we want to access
     */
    void cat(const std::vector<std::string>& parameters, FileSystem* fileSystem);

    /**
     * Copies one file from virtual file system into the real filesystem in this machine.
     *
     * @param parameters  requires two parameters - path to existing file in virtual filesystem and path in the PC to store it's copy
     * @param fileSystem virtual file system which we want to access
     */
    void outcp(const std::vector<std::string>& parameters, FileSystem* fileSystem);

    /**
     * Prints info about the file at the end of given path.
     *
     * @param parameters requires one parameter - existing path to a file or directory in virtual file system
     * @param fileSystem virtual file system that we want to access
     */
    void info(const std::vector<std::string>& parameters, FileSystem* fileSystem);

    /**
     * Creates a directory at given path.
     *
     * @param parameters requires one parameter -
     * @param fileSystem
     */
    void mkdir(const std::vector<std::string>& parameters, FileSystem* fileSystem);

    /**
     * Removes a directory at given path, if it is empty.
     *
     * @param parameters requires one parameter - existing path to a directory in virtual file system
     * @param fileSystem virtual file system that we want to access
     */
    void rmdir(const std::vector<std::string>& parameters, FileSystem* fileSystem);

    /**
     * Copies a file in virtual file system at given path to the second given path.
     *
     * @param parameters requires two parameters - path to an existing file, which is not a directory, and an existing path to a directory
     * @param fileSystem virtual file system that we want to access
     */
    void cp(const std::vector<std::string>& parameters, FileSystem* fileSystem);

    /**
     * Moves a file in virtual filesystem at given path to the second given path.
     *
     * @param parameters requires two parameters - path to an existing file, which is not a directory, and an existing path to a directory
     * @param fileSystem virtual file system that we want ot access
     */
    void mv(const std::vector<std::string>& parameters, FileSystem* fileSystem);

    /**
     * Loads a file from hard-disk drive with individual commands and executes them sequentially.
     *
     * @param parameters requires one parameter - path to an existing file with function commands
     * @param fileSystem virtual file system that we want to access
     */
    void load(const std::vector<std::string> &parameters, FileSystem *fileSystem);

    /**
     * Performs data consistence check throughout the file system.
     * @param parameters requires no parameters, none of given parameters will be used
     * @param fileSystem file system to check
     */
    void check(const std::vector<std::string> &parameters, FileSystem* fileSystem);

    /**
     * Breaks data consistence. Only used to demonstrate data consistence check.
     *
     * @param parameters requires no parameters, none of given parameters will be used
     * @param fileSystem file system to break
     */
    void breakData(const std::vector<std::string> &parameters, FileSystem* fileSystem);
}
#endif //PRIMITIVE_FS_FUNCTION_H
