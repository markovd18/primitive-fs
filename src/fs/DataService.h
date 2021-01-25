//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_DATASERVICE_H
#define PRIMITIVE_FS_DATASERVICE_H

#include <string>
#include <vector>
#include <filesystem>
#include "../common/structures.h"
#include "FileData.h"

namespace pfs {

    /**
     * Class responsible for manipulation with inode data.
     */
    class DataService {
    private: // private attributes
        /// Data file representing the virtual file system
        std::string m_dataFileName;
        /// Data block bitmap
        fs::Bitmap m_dataBitmap;
        /// Address where to store the data bitmap
        int32_t m_dataBitmapAddress = -1;
        /// Start address of the data block storage
        int32_t m_dataStartAddress = -1;

    public: // public methods
        DataService() = default;
        DataService(std::string mDataFileName, fs::Bitmap dataBitmap,
                    int32_t dataBitmapAddress, int32_t dataStartAddress);
        /**
         * Returns all directory items of directory, represented by given inode. If inode doesn't represent folder, throws @a invalid_argument
         *
         * @param directory inode representing the directory
         * @return vector of directory items
         * @throw invalid_argument if the inode doesn't represent a directory
         */
        [[nodiscard]] std::vector<fs::DirectoryItem> getDirectoryItems(const fs::Inode& directory) const;
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
         * Returns all direct links to a file data, including the ones stored in indirect links.
         *
         * @param inode inode to return it's direct data links
         * @return direct data links of given inode
         */
        [[nodiscard]] std::vector<int32_t> getAllDirectLinks(const fs::Inode &inode) const;
        /**
         * Saves given directory item into given directory.
         *
         * @param directoryItem directory item to save
         * @param directory directory to save given directory item into
         * @throw invalid_argument if given inode is not a directory
         */
        void saveDirItemIntoDirectory(const fs::DirectoryItem &directoryItem, fs::Inode& directory);
        /**
         * Clears all inode data from this file system. Note that the inode itself will not be deleted.
         *
         * @param inode inode to remove it's data
         */
        void clearInodeData(const fs::Inode &inode);
        /**
         * @brief Removes directory item with given file name from current directory.
         * @param filename
         * @return removed directory item
         * @throw ObjectNotFound if current directory doesn't contain a directory item with given name
         */
        fs::DirectoryItem removeDirectoryItem(const std::string &filename, fs::Inode& directory);
        /**
         * Finds a directory item with given name in given directory.
         *
         * @param fileName name of the directory item
         * @param directory directory where to look
         * @return found directory item
         * @throw ObjectNotFound if the directory doesn't contain a directory item with given name
         */
        [[nodiscard]] fs::DirectoryItem findDirectoryItem(const std::filesystem::path &fileName, const fs::Inode& directory) const;
        /**
         * Saves all file data into the file system data file.
         *
         * @param clusteredData file data parsed into clusters
         * @param dataClusterIndexes indexes of clusters in the file system data file
         */
        void saveFileData(const fs::ClusteredFileData& clusteredData, const std::vector<int32_t>& dataClusterIndexes);
        /**
         * Returns concatenated data of given file.
         *
         * @param inode file to return it's data
         * @return file's data
         * @throw invalid_argument if file is a directory
         */
        [[nodiscard]] std::string getFileContent(const fs::Inode &inode) const;
        /**
         * Checks given data block containing directory items for a free sub-index, where next directory item could be stored.
         * If none is found, returns the size of a cluster.
         *
         * @param dirItemDataBlockSubindex index of a data block
         * @return free sub-index in given data block or size of a data block
         */
        [[nodiscard]] size_t getFreeDirItemDataBlockSubindex(int32_t dirItemDataBlockSubindex) const;
    private: // private methods
        /// Reads all directory items on given indexes
        [[nodiscard]] std::vector<fs::DirectoryItem> readDirItems(const std::vector<int32_t> &indexList) const;
        /// Saves directory item to any free direct link of given directory
        bool saveDirItemToFreeDirectLink(const fs::DirectoryItem& directoryItem, fs::Inode& directory);
        /// Saves directory item to given data block index
        void saveDirItemToIndex(const fs::DirectoryItem &directoryItem, int32_t index);
        /// Saves directory item to given address
        void saveDirItemToAddress(const fs::DirectoryItem& directoryItem, int32_t address) const;
        /// Saves directory item to the indirect link of given directory
        void saveDirItemToIndirect(const fs::DirectoryItem& directoryItem, fs::Inode& directory);
        /// Returns sub-index from a data block of direct links, where empty storage starts
        [[nodiscard]] size_t getFreeIndirectLinkDataBlockSubindex(int32_t indirectLinkDatablockIndex) const;
        /// Saves directory item to any free indirect link of given directory
        void saveDirItemToFreeIndirectLink(const fs::DirectoryItem &directoryItem, fs::Inode& directory);
        /// Checks if directory item data block is empty
        [[nodiscard]] bool isDirItemIndexFree(int32_t index) const;
        /// Checks if indirect link data block is empty
        [[nodiscard]] bool isIndirectLinkFree(int32_t index) const;
        /// Removes directory item from given data block
        [[nodiscard]] fs::DirectoryItem removeDirItemFromCluster(const std::string &filename, int index) const;

    };
}


#endif //PRIMITIVE_FS_DATASERVICE_H
