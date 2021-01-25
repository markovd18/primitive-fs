//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_FILESYSTEM_H
#define PRIMITIVE_FS_FILESYSTEM_H

#include <string>
#include <memory>
#include <iostream>
#include <filesystem>
#include <vector>

#include "../common/structures.h"
#include "FileData.h"
#include "InodeService.h"
#include "DataService.h"

/**
 * Represents the virtual file system loaded by the application. File system is represented by one file where
 * all the data is stored. File system class defines set of operations that can be invoked over the representing
 * file such as writing, reading or deleting data.
 */
class FileSystem {
private: //private attributes
    /// The data file representing the file system.
    std::string m_dataFileName;
    /// Is file system initialized?
    bool m_initialized = false;
    /// Superblock with fundamental information about the file system.
    fs::Superblock m_superblock{};
    /// Inode of current working directory. Used for easier orientation.
    fs::Inode m_currentDirInode;
    /// Current working directory
    std::string m_currentDirPath;
    /// Service for manipulation with inodes
    pfs::InodeService m_inodeService;
    /// Service for manipulation with inode data
    pfs::DataService m_dataService;
public: //public methods
    /**
     * Default constructor for initialization.
     *
     * @param fileName data-file name
     */
    explicit FileSystem(const std::string& fileName)
        : m_dataFileName(fileName) {
        if (std::filesystem::exists(fileName)) {
            try {
                initializeFromExisting();
            } catch (std::exception& exception) {
                std::cout << "Error while file system initialization!\n";
                std::cout << exception.what();
                exit(EXIT_FAILURE);
            }
        }
    }


    /**
     * Initializes the file system in data file. Writes super-block, bitmaps, and root directory inside.
     *
     * @param superblock superblock to initialize the file system by
     * @return true if initialization was successfull, otherwise false
     *
     */
    bool initialize(fs::Superblock& superblock);

    /**
     * Initializes the file system from existing data file. Reads super-block and bitmaps into memory.
     *
     * @return true if initialization was successfull, otherwise false
     */
    bool initializeFromExisting();

    /**
     * Returns true, if the file system has been correctly initialized, otherwise false.
     *
     * @return true, if the file system has been correctly initialized, otherwise false
     */
    [[nodiscard]] bool isInitialized() const {
        return m_initialized;
    }

    /**
     * Creates file in virtual file system on given path with given data.
     *
     * @param path path in virtual file system
     * @param data data of the file
     */
    void createFile(const std::filesystem::path& path, const fs::FileData& data);

    /**
     * Removes file at the end of the given path in virtual file system.
     *
     * @param path path in virtual file system
     */
    void removeFile(const std::filesystem::path& path);

    /**
     * Returns the current working directory.
     *
     * @return current working directory
     */
    [[nodiscard]] const std::string& getCurrentDir() const {
        return m_currentDirPath;
    }
    /**
     * Changes current working directory to the directory give in path. If path doesn't exist or there is a file at the end of the path,
     * throws an exception.
     *
     * @param path path to change current working directory into
     * @throw std::invalid_argument If there is any error
     */
    void changeDirectory(const std::filesystem::path& path);
    /**
     * Returns the inode with given id. If inode with given doesn't exist, throws an exception.
     *
     * @param inodeId id of wanted inode
     * @return inode with given id
     */
    fs::Inode findInode(int inodeId);
    /**
     * Returns all directory items of directory on given path in a virtual filesystem.
     *
     * @param dirPath directory path in the virtual file system
     * @return vector of directory items
     * @throw invalid_argument if the path doesn't exist or doesn't point to a directory
     */
    std::vector<fs::DirectoryItem> getDirectoryItems(const std::filesystem::path& dirPath);
    /**
     * Prints content of a file into the console.
     *
     * @param pathToFile path to a file to print into the console
     * @throw invalid_argument if file is not found or is a directory
     */
    void printFileContent(const std::filesystem::path& pathToFile);
    /**
     * Returns the content of a file.
     *
     * @param pathToFile path to a file to retrieve it's content
     * @return content of given file
     * @throw invalid_parameter if file is not found or is a directory
     */
    std::string getFileContent(const std::filesystem::path& pathToFile);
    /**
     * Prints information about a file at the end of given path into the console.
     *
     * @param pathToFile path to a file to get information of
     * @throw invalid_argument if file is not found
     */
    void printFileInfo(const std::filesystem::path& pathToFile);
    /**
     * Creates new directory at given path.
     *
     * @param path path to new directory
     */
    void createDirectory(const std::filesystem::path& path);
    /**
     * Removes directory at given path, if it is empty.
     *
     * @param path path to existing directory
     */
    void removeDirectory(const std::filesystem::path& path);
    /**
     * Copies an existing file from given path to the second given path. The destination path has to be including the new filename.
     *
     * @param pathFrom source path of a file
     * @param pathTo new destination path of a file
     */
    void copyFile(const std::filesystem::path& pathFrom, const std::filesystem::path& pathTo);
    /**
     * Moves an existing file from given path to the second given path. The destination path has to be including the new filename.
     *
     * @param pathFrom source path of a file
     * @param pathTo new destination path of a file
     */
    void moveFile(const std::filesystem::path& pathFrom, const std::filesystem::path& pathTo);
private: //private methods
    /**
     * Writes superblock at the start of the file-system. Requires open input stream to data file passed. If
     * the input stream is closed, returns a failure.
     *
     * @param dataFile open output file stream
     * @param superblock superblock to write
     * @return true when successfully written superblock, otherwise false
     */
    bool writeSuperblock(std::fstream& dataFile, fs::Superblock& superblock);
    /**
     * Initializes and writes bitmap of inodes into the file-system. Bitmap corresponds to a filesystem with
     * root folder only. Requires open output stream to data file passed. If the output stream is closed,
     * returns a failure.
     *
     * @param dataFile open output file stream
     * @return true when successfully written bitmap, otherwise false
     */
    bool initializeInodeBitmap(std::fstream& dataFile);
    /**
     * Initializes inode bitmap from existing data file and loads it into memory. Requires open input stream from existing
     * data file passed. If the input stream is closed, returns a failure. If the reading process fails, throws an exception.
     *
     * @param dataFile open input file stream
     * @return true when successfully read inode bitmap, otherwise false
     */
    bool initializeInodeBitmap(std::ifstream& dataFile);
    /**
     * Initializes and writes bitmap of data into the file-system. Bitmap corresponds to a filesystem with
     * root folder only. Requires open output stream to data file passed. If the output stream is closed,
     * returns a failure.
     *
     * @param dataFile open output file stream
     * @return true when successfully written bitmap, otherwise false
     */
    bool initializeDataBitmap(std::fstream& dataFile);
    /**
     * Initializes data bitmap from existing data file and loads it into memory. Requires open input stream from existing
     * data file passed. If the input stream is closed, returns a failure. If the reading process fails, throws an exception.
     *
     * @param dataFile open input file stream
     * @return true when successfully read data bitmap, otherwise false
     */
    bool initializeDataBitmap(std::ifstream& dataFile);
};


#endif //PRIMITIVE_FS_FILESYSTEM_H
