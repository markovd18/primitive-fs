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

/**
 * Represents the virtual file system loaded by the application. File system is represented by one file where
 * all the data is stored. File system class defines set of operations that can be invoked over the representing
 * file such as writing, reading or deleting data.
 */
class FileSystem {
private: //private attributes
    /**
     * The data file representing the file system.
     */
    std::string m_dataFileName;
    /**
     * Is file system initialized?
     */
    bool m_initialized;
    /**
     * Superblock with fundamental information about the file system.
     */
    fs::Superblock m_superblock{};
    /**
     * Bitmap of used and unused memory addresses for i-nodes.
     */
    fs::Bitmap m_inodeBitmap;
    /**
     * Bitmap of used and unused memory addresses for data blocks.
     */
    fs::Bitmap m_dataBitmap;
    /**
     * Inode of current working directory. Used for easier orientation.
     */
    fs::Inode m_currentDirInode;
    /**
     * Current working directory.
     */
    std::string m_currentDirPath;
public: //public methods
    /**
     * Default constructor for initialization.
     *
     * @param fileName data-file name
     */
    explicit FileSystem(const std::string& fileName)
        : m_dataFileName(fileName), m_initialized(false){
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
     */
    bool initialize(fs::Superblock& superblock);

    /**
     * Initializes the file system from existing data file. Reads super-block and bitmaps into memory.
     *
     * @return
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
    void createFile(const std::string& path, const std::string& data);

    /**
     * Returns the current working directory.
     *
     * @return current working directory
     */
    [[nodiscard]] const std::string& getCurrentDir() const {
        return m_currentDirPath;
    }
    /**
     * Returns smallest available inode id, if any is available, otherwise returns {@code FREE_INODE_ID}.
     *
     * @return smallest available inode id, or {@code FREE_INODE_ID}
     */
    [[nodiscard]] int32_t getInodeId() const;

    /**
     * Changes current working directory to the directory give in path. If path doesn't exist or there is a file at the end of the path,
     * throws an exception.
     *
     * @param path path to change current working directory into
     */
    void changeDirectory(const std::string& path);

    /**
     * Returns the inode with given id. If inode with given doesn't exist, throws an exception.
     *
     * @param inodeId id of wanted inode
     * @return inode with given id
     */
    fs::Inode findInode(int inodeId);

    /**
     * Returns all directory items of directory, represented by given inode. If inode doesn't represent folder, returns empty vector.
     *
     * @param directory inode representing the directory
     * @return vector of directory items, if inode is directory, otherwise nothing
     */
    std::vector<fs::DirectoryItem> getDirectoryItems(const fs::Inode& directory);

private: //private methods
    /**
     * Writes superblock at the start of the file-system. Requires open input stream to data file passed. If
     * the input stream is closed, returns a failure.
     *
     * @param dataFile open output file stream
     * @param superblock superblock to write
     * @return true when successfully written superblock, otherwise false
     */
    bool writeSuperblock(std::ofstream& dataFile, fs::Superblock& superblock);
    /**
     * Initializes and writes bitmap of inodes into the file-system. Bitmap corresponds to a filesystem with
     * root folder only. Requires open output stream to data file passed. If the output stream is closed,
     * returns a failure.
     *
     * @param dataFile open output file stream
     * @return true when successfully written bitmap, otherwise false
     */
    bool initializeInodeBitmap(std::ofstream& dataFile);
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
    bool initializeDataBitmap(std::ofstream& dataFile);
    /**
     * Initializes data bitmap from existing data file and loads it into memory. Requires open input stream from existing
     * data file passed. If the input stream is closed, returns a failure. If the reading process fails, throws an exception.
     *
     * @param dataFile open input file stream
     * @return true when successfully read data bitmap, otherwise false
     */
    bool initializeDataBitmap(std::ifstream& dataFile);
    /**
     * Writes bitmap into the data file with the offset relative to the start of the file system.
     * Requires open output stream to data file passed. If the output stream is closed, returns a failure.
     *
     * @param dataFile open output file stream
     * @param bitmap bitmap to write
     * @param offset offset relative to the start of the file system
     * @return true when successfully written bitmap, otherwise false
     */
    bool writeBitmap(std::ofstream& dataFile, const fs::Bitmap& bitmap, int offset);
    /**
     * Reads bitmap from the data file from the position relative to the start of the file system with given offset.
     * Requires open input stream from data file passed. If the input stream is closed, returns a failure. Passed bitmap has to be
     * already initialized into default state - array pointer has to be initialized and length has to be set. Length attribute will be used
     * to determine how many bytes to read from the data file.
     *
     * @param dataFile open input file stream
     * @param bitmap bitmap to read into
     * @param offset offset relative to the start of the file system
     * @return true when successfully read bitmap, otherwise false
     */
    bool readBitmap(std::ifstream& dataFile, fs::Bitmap& bitmap, int offset);

    /**
     * Finds the i-node of root directory and stores it into the inode passed as a parameter.
     *
     * @param rootInode instance to store rootInode data into
     */
    void getRootInode(fs::Inode& rootInode);

    /**
     * Saves given inode into data file.
     *
     * @param inode inode to save
     */
    void saveInode(const fs::Inode& inode);

    /**
     * Reads all directory items of passed directory i-node from it's direct links and stores them into passed vector of directory items.
     * If passed argument is not a directory, throws.
     *
     * @param directory directory which we want to read
     * @param directoryItems vector to store directory items into
     */
    void readDirItemsDirect(const fs::Inode& directory, std::vector<fs::DirectoryItem>& directoryItems);

    /**
     * Reads all directory items of passed directory i-node from it's indirect links and stores them into passed vector of directory items.
     * If passed argument is not a directory, throws.
     *
     * @param directory directory which we want to read
     * @param directoryItems vector to store directory items into
     */
    void readDirItemsIndirect(const fs::Inode& directory, std::vector<fs::DirectoryItem>& directoryItems);

    /**
     * Reads all directory items on passed data indexes and stores them into passed vector of directory items.
     *
     * @param indexList vector of directory item data indexes
     * @param directoryItems vector to store directory items into
     */
    void readDirItems(const std::vector<int32_t>& indexList, std::vector<fs::DirectoryItem>& directoryItems);
};


#endif //PRIMITIVE_FS_FILESYSTEM_H
