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
     * Returns smallest available inode id, if any is available, otherwise throws ObjectNotFound.
     * Returned id is also an offset from the beginning of the inode storage, meaning that inode with id = 3
     * will be stored at 3 * sizeof(fs::Inode) from the inode storage start.
     *
     * @throw pfs::ObjectNotFound If no free i-node id was found
     * @return smallest available inode id
     */
    [[nodiscard]] int32_t getInodeId() const;

    /**
     * Creates an instance of fs::Inode based on given parameters. This factory method should be used for creation of every
     * inode instance, which is intended to be saved into the data file, since it checks side effects of creating inode,
     * such as if there is any space left for new inode instance in out file system.
     *
     * @param isDirectory will inode represent a directory?
     * @param fileSize size of the represented file
     * @return fs::Inode instance
     */
    [[nodiscard]] fs::Inode createInode(bool isDirectory, int32_t fileSize) const;

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
     * Saves given inode into data file.
     *
     * @param inode inode to save
     */
    void saveInode(const fs::Inode& inode);

    /**
     * Returns all directory items of directory on given path in a virtual filesystem.
     *
     * @param dirPath directory path in the virtual file system
     * @return vector of directory items
     * @throw invalid_argument if the path doesn't exist or doesn't point to a directory
     */
    std::vector<fs::DirectoryItem> getDirectoryItems(const std::filesystem::path& dirPath);

    /**
     * @brief Removes directory item with given file name from current directory.
     * @param filename
     * @return removed directory item
     * @throw ObjectNotFound if current directory doesn't contain a directory item with given name
     */
    fs::DirectoryItem removeDirectoryItem(const std::string& filename);

    /**
     * Returns all directory items of directory, represented by given inode. If inode doesn't represent folder, throws @a invalid_argument
     *
     * @param directory inode representing the directory
     * @return vector of directory items
     * @throw invalid_argument if the inode doesn't represent a directory
     */
    std::vector<fs::DirectoryItem> getDirectoryItems(const fs::Inode& directory);

    /**
     * Saves given directory item into current working directory.
     *
     * @param directoryItem directory item to save
     */
    void saveDirItemIntoCurrent(const fs::DirectoryItem& directoryItem);

    /**
     * Returns index of free data block. If none is found, throws ObjectNotFound.
     * @throw ObjectNotFound if no free index is found
     * @return free index of data block
     */
    [[nodiscard]] int32_t getFreeDataBlock() const;

    /**
     * Returns indexes of free data blocks based on the requested count. If none or less than requested count
     * is found, throws ObjectNotFound.
     *
     * @param count requested number of free data blocks
     * @return vector of free data block indexes
     * @throw ObjectNotFound when no free index is found or not enough free indexes are found
     */
    [[nodiscard]] std::vector<int32_t> getFreeDataBlocks(std::size_t count) const;

    /**
     * Updates the inode bitmap. The member instance of bitmap, representing inode bitmap, will be saved into the data file
     * in it's current state.
     */
    void updateInodeBitmap();

    /**
     * Updates the data bitmap. THe member instance of bitmap, representing data bitmap, will be saved into the data file
     * in it's current state.
     */
    void updateDataBitmap();

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

    /**
     * Finds the i-node of root directory and stores it into the inode passed as a parameter.
     *
     * @param rootInode instance to store rootInode data into
     */
    void getRootInode(fs::Inode& rootInode);

    /**
     * Reads all directory items on given indexes
     *
     * @param indexList vector of directory item data indexes
     * @return vector with directory items found on given indexes
     */
    std::vector<fs::DirectoryItem> readDirItems(const std::vector<int32_t>& indexList);

    /**
     * Saves all file data into the file system data file.
     *
     * @param clusteredData file data parsed into clusters
     * @param dataClusterIndexes indexes of clusters in the file system data file
     */
    void saveFileData(const fs::ClusteredFileData& clusteredData, const std::vector<int32_t>& dataClusterIndexes);

    /**
     * Checks given data block containing directory items for a free sub-index, where next directory item could be stored.
     * If none is found, returns the size of a cluster.
     *
     * @param dirItemDataBlockSubindex index of a data block
     * @return free sub-index in given data block or size of a data block
     */
    size_t getFreeDirItemDataBlockSubindex(int32_t dirItemDataBlockIndex);

    /**
     * Checks given indirect link data block containing direct links with directory items for a free sub-index, where next
     * direct link could be stores
     *
     * @param indirectLinkDataBlockIndex index of indirect link
     * @return free sub-index in given data block or size of a data block
     */
    size_t getFreeIndirectLinkDataBlockSubindex(int32_t indirectLinkDataBlockIndex);
    /**
     * @brief Saves directory item into given index
     * @param directoryItem directory item to save
     * @param offset index to save directory item to
     */
    void saveDirItemToIndex(const fs::DirectoryItem& directoryItem, int32_t offset);

    /**
     * Saves given directory item to any free data block and adds direct link to current directory inode.
     * @param directoryItem directory item to store to current directory
     * @return true if successfully added direct link, otherwise false
     */
    bool saveDirItemToFreeDirectLink(const fs::DirectoryItem &directoryItem);

    /**
     * @brief Saves given directory item to indirect link of current directory.
     * @param directoryItem directory item to store
     *
     * Checks for indirect links of current directory. If all indirect links are full, throws @a ObjectNotFound.
     * If the last filled indirect link is not completely full, checks if the last direct link in that indirect link is full.
     * If not, saves given directory item there, otherwise creates new direct links, stores directory item there and new direct link
     * stores into this indirect link. If last filled indirect link is completely full and there is space for new indirect link,
     * creates new indirect link, new direct link to store directory item, direct link saves into indirect link and adds new indirect
     * link to current directory.
     */
    void saveDirItemToCurrentIndirect(const fs::DirectoryItem &directoryItem);

    /**
     * @brief Saves given directory item into the data file to given address
     * @param directoryItem directory item to save
     * @param address address to save directory item to
     *
     * This method only saves the directory item and does NOTHING else. Setting bit in bitmap has to be done separately.
     */
    void saveDirItemToAddress(const fs::DirectoryItem &directoryItem, int32_t address);

    /**
     * @brief Saves given directory item to new free indirect link
     * @param directoryItem directory item to save
     *
     * Creates new direct link, saves directory item into it and saves the direct link into new indirect link. New indirect
     * link is added to current directory i-node.
     */
    void saveDirItemToFreeIndirectLink(const fs::DirectoryItem &directoryItem);

    /**
     * Clears all inode data from this file system, including inode itself and it's direct and indirect links.
     *
     * @param inode inode to remove from file system
     */
    void clearInodeData(const fs::Inode& inode);
    /**
     * Clears data block index in data bitmap. Only sets given index as free, but does not delete or modify memory at that index.
     *
     * @param dataBlockIndex index to be set as free
     */
    void clearDataBlock(int32_t dataBlockIndex);

    /**
     * Returns a vector filled with all the direct links from given inode, meaning all stored direct links and all
     * direct links stored in indirect links.
     *
     * @param inode inode to find it's direct links
     * @return vector with inode's direct links
     */
    std::vector<int32_t> getAllDirectLinks(const fs::Inode& inode);

    /**
     * @brief Checks if given directory item data cluster is free or not.
     *
     * @param index index of directory item data cluster
     * @return true, if given data cluster is free, otherwise false
     */
    bool isDirItemIndexFree(int32_t index);

    /**
     * @brief CHecks if given indirect link is free or not.
     *
     * @param index index of indirect link data cluster
     * @return true, if given data cluster is free, otherwise false
     */
    bool isIndirectLinkFree(int32_t index);

    fs::DirectoryItem removeDirItemFromCluster(const std::string &basicString, int index);
};


#endif //PRIMITIVE_FS_FILESYSTEM_H
