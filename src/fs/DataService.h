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
        std::string m_dataFileName;
        fs::Bitmap m_dataBitmap;
        int32_t m_dataBitmapAddress = -1;
        int32_t m_dataStartAddress = -1;

    public: // public methods
        DataService() = default;
        DataService(std::string mDataFileName, fs::Bitmap dataBitmap,
                    int32_t dataBitmapAddress, int32_t dataStartAddress);
        [[nodiscard]] std::vector<fs::DirectoryItem> getDirectoryItems(const fs::Inode& directory) const;
        [[nodiscard]] int32_t getFreeDataBlock() const;
        [[nodiscard]] std::vector<int32_t> getFreeDataBlocks(std::size_t count) const;
        [[nodiscard]] std::vector<int32_t> getAllDirectLinks(const fs::Inode &inode) const;
        [[nodiscard]] std::vector<fs::DirectoryItem> readDirItems(const std::vector<int32_t> &indexList) const;
        void saveDirItemIntoDirectory(const fs::DirectoryItem &directoryItem, fs::Inode& directory);
        void clearInodeData(const fs::Inode &inode);
        fs::DirectoryItem removeDirectoryItem(const std::string &filename, fs::Inode& directory);
        [[nodiscard]] fs::DirectoryItem findDirectoryItem(const std::filesystem::path &fileName, const fs::Inode& directory) const;
        void saveFileData(const fs::ClusteredFileData& clusteredData, const std::vector<int32_t>& dataClusterIndexes);
        [[nodiscard]] std::string getFileContent(const fs::Inode &inode) const;
        [[nodiscard]] size_t getFreeDirItemDataBlockSubindex(int32_t dirItemDataBlockSubindex) const;

    private: // private methods
        bool saveDirItemToFreeDirectLink(const fs::DirectoryItem& directoryItem, fs::Inode& directory);
        void saveDirItemToIndex(const fs::DirectoryItem &directoryItem, int32_t index);
        void saveDirItemToAddress(const fs::DirectoryItem& directoryItem, int32_t address) const;
        void saveDirItemToIndirect(const fs::DirectoryItem& directoryItem, fs::Inode& directory);
        [[nodiscard]] size_t getFreeIndirectLinkDataBlockSubindex(int32_t indirectLinkDatablockIndex) const;
        void saveDirItemToFreeIndirectLink(const fs::DirectoryItem &directoryItem, fs::Inode& directory);
        [[nodiscard]] bool isDirItemIndexFree(int32_t index) const;
        [[nodiscard]] bool isIndirectLinkFree(int32_t index) const;
        [[nodiscard]] fs::DirectoryItem removeDirItemFromCluster(const std::string &filename, int index) const;

    };
}


#endif //PRIMITIVE_FS_DATASERVICE_H
