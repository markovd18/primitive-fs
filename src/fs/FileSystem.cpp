//
// Author: markovd@students.zcu.cz
//

#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include "FileSystem.h"
#include "../utils/FilePathUtils.h"
#include "../utils/InvalidState.h"
#include "../command/returnval.h"

bool FileSystem::initialize(fs::Superblock &sb) {

    std::fstream dataFile(m_dataFileName, std::ios::out | std::ios::binary);
    if (!dataFile) {
        return false;
    }
    /**
     * First we write superblock at the start of the filesystem
     */
    if(!writeSuperblock(dataFile, sb)) {
        std::cout << "Error while writing superblock!\n";
        return false;
    }
    /**
     * Then we write i-node and data bitmap
     */
    if (!initializeInodeBitmap(dataFile)) {
        std::cout << "Error while writing i-node bitmap!\n";
        return false;
    }

    if (!initializeDataBitmap(dataFile)) {
        std::cout << "Error while writing data-block bitmap!\n";
        return false;
    }
    /**
     * After initialization, there will be only root i-node
     */
    fs::Inode rootInode(0, true, 0);
    rootInode.addDirectLink(0); /// We add direct link to the first data block, where the root folder data will be

    rootInode.save(dataFile, m_superblock.getInodeStartAddress());
    /// We fill the empty i-node space
    for (size_t i = (m_superblock.getInodeStartAddress() + sizeof(fs::Inode)); i < m_superblock.getDataStartAddress(); ++i) {
        dataFile.put('\0');
    }

    /**
     * Creating content of the root directory:
     * +root
     * -.
     * -..
     *
     */
    fs::DirectoryItem rootSelf(pfs::path::SELF, 0);
    rootSelf.save(dataFile, m_superblock.getDataStartAddress());
    fs::DirectoryItem rootParent(pfs::path::PARENT, 0);
    rootParent.save(dataFile, dataFile.tellp());

    /// We fill the empty data space
    for (size_t i = (m_superblock.getDataStartAddress() + 2 * sizeof(fs::DirectoryItem)); i < sb.getDiskSize(); ++i) {
        dataFile.put('\0');
    }

    dataFile.flush();
    /// In the end we are successfully initialized
    m_currentDirPath = "/";
    m_currentDirInode = rootInode;
    m_initialized = true;
    return true;
}

bool FileSystem::initializeFromExisting() {
    std::ifstream dataFile(m_dataFileName, std::ios::in | std::ios::binary);
    if (!dataFile) {
        return false;
    }

    m_superblock.load(dataFile, 0);

    if (!initializeInodeBitmap(dataFile)) {
        std::cout << "Error while reading inode bitmap from the data file!\n";
        return false;
    }

    if (!initializeDataBitmap(dataFile)) {
        std::cout << "Error while reading data bitmap from the data file!\n";
        return false;
    }

    /// By default we start in the root directory.
    m_currentDirPath = "/";
    /// Load the inode representing current directory
    m_currentDirInode.load(dataFile, m_superblock.getInodeStartAddress());
    std::cout << "Initialized from existing file!\n";
    /// In the end we are successfully initialized
    m_initialized = true;
    return true;
}

int32_t FileSystem::getInodeId() const {
    return m_inodeBitmap.findFirstFreeIndex();
}

fs::Inode FileSystem::createInode(bool isDirectory, int32_t fileSize) const {
    return fs::Inode(getInodeId(), isDirectory, fileSize);
}

bool FileSystem::writeSuperblock(std::fstream& dataFile, fs::Superblock &sb) {
    if (!dataFile.is_open()) {
        return false;
    }
    m_superblock = sb;
     /// Superblock has to be at the start of the file-system.
    m_superblock.save(dataFile, 0);

    return !dataFile.bad();
}

bool FileSystem::initializeInodeBitmap(std::fstream& dataFile) {
    if (!dataFile.is_open()) {
        return false;
    }

    m_inodeBitmap = std::move(fs::Bitmap(
            m_superblock.getDataBitmapStartAddress() - m_superblock.getInodeBitmapStartAddress()));
    m_inodeBitmap.setIndexFilled(0);
    m_inodeBitmap.save(dataFile, m_superblock.getInodeBitmapStartAddress());
    return !dataFile.bad();
}

bool FileSystem::initializeInodeBitmap(std::ifstream& dataFile) {
    if (!dataFile.is_open()) {
        return false;
    }

    m_inodeBitmap = std::move(fs::Bitmap(
            m_superblock.getDataBitmapStartAddress() - m_superblock.getInodeBitmapStartAddress()));
    m_inodeBitmap.load(dataFile, m_superblock.getInodeBitmapStartAddress());
    return !dataFile.bad();
}

bool FileSystem::initializeDataBitmap(std::fstream& dataFile) {
    if (!dataFile.is_open()) {
        return false;
    }

    m_dataBitmap = std::move(fs::Bitmap(m_superblock.getInodeStartAddress() - m_superblock.getDataBitmapStartAddress()));
    m_dataBitmap.setIndexFilled(0);
    m_dataBitmap.save(dataFile, m_superblock.getDataBitmapStartAddress());
    return !dataFile.bad();
}

bool FileSystem::initializeDataBitmap(std::ifstream& dataFile) {
    if (!dataFile.is_open()) {
        return false;
    }

    m_dataBitmap = std::move(fs::Bitmap(m_superblock.getInodeStartAddress() - m_superblock.getDataBitmapStartAddress()));
    m_dataBitmap.load(dataFile, m_superblock.getDataBitmapStartAddress());
    return !dataFile.bad();
}

void FileSystem::createFile(const std::filesystem::path &path, const fs::FileData &fileData) {
    if (!path.has_filename()) {
        throw std::invalid_argument("Předaná cesta nekončí názvem souboru");
    }

    std::string pathStr(path.string());
    std::string pathNoFilename(pathStr.substr(0, pathStr.length() - path.filename().string().length()));
    /// We store current working directory, so we can return back in the end
    const std::string currentDir = m_currentDirPath;
    const fs::Inode currentInode = m_currentDirInode;

    if (m_currentDirPath != pathNoFilename) {
        /// By changing to given path we not only get access to the i-node we need, but also validate it's existence
        try {
            changeDirectory(pathNoFilename);
        } catch (const std::exception &ex) {
            throw std::invalid_argument(fnct::PNF_DEST);
        }
    }

    std::vector<fs::DirectoryItem> dirItems(getDirectoryItems(m_currentDirPath));
    auto it = std::find_if(dirItems.begin(), dirItems.end(), [&path](const fs::DirectoryItem item) { return item.nameEquals(path.filename()); });
    if (it != dirItems.end()) {
        throw pfs::InvalidState("Soubor s předaným názvem již exituje!");
    }

    fs::Inode inode(createInode(false, fileData.size()));

    fs::ClusteredFileData clusteredData(fileData);
    std::vector<int32_t> dataClusterIndexes(getFreeDataBlocks(clusteredData.requiredDataBlocks()));
    inode.setData(fs::DataLinks(dataClusterIndexes));

    saveInode(inode);
    saveFileData(clusteredData, dataClusterIndexes);
    /// We cd'd into the parent folder so we can just save into current folder
    saveDirItemIntoCurrent(fs::DirectoryItem(path.filename(), inode.getInodeId()));
    m_currentDirInode.setFileSize(m_currentDirInode.getFileSize() + inode.getFileSize());
    saveInode(m_currentDirInode);

    if (currentDir != pathNoFilename) {
        /// Returning back to the original directory
        m_currentDirInode = currentInode;
        m_currentDirPath = currentDir;
    }

}

void FileSystem::removeFile(const std::filesystem::path &path) {
    if (!path.has_filename()) {
        throw std::invalid_argument("Předaná cesta nekončí názvem souboru");
    }

    std::string pathStr(path.string());
    std::string pathNoFilename = pathStr.substr(0, pathStr.length() - path.filename().string().length());
    /// We store current working directory, so we can return back in the end
    const std::string currentDir = m_currentDirPath;
    const fs::Inode currentInode = m_currentDirInode;

    if (m_currentDirPath != pathNoFilename) {
        /// By changing to given path we not only get access to the i-node we need, but also validate it's existence
        changeDirectory(pathNoFilename);
    }

    fs::DirectoryItem directoryItem = findDirectoryItem(path.filename());
    fs::Inode fileInode(findInode(directoryItem.getInodeId()));
    if (fileInode.isDirectory()) {
        throw std::invalid_argument("Soubor na předané cestě nelze smazat, protože je to složka");
    }

    removeDirectoryItem(path.filename());
    clearInodeData(fileInode);

    if (currentDir != pathNoFilename) {
        /// Returning back to the former directory
        m_currentDirInode = currentInode;
        m_currentDirPath = currentDir;
    }

}

void FileSystem::changeDirectory(const std::filesystem::path& path) {
    if (path.empty()) {
        /// If no path is provided, we don't change anything and return.
        return;
    }

    std::vector<std::string> tokens = pfs::path::parsePath(path);

    /// First we need to know from which directory we will move
    fs::Inode referenceFolder;
    bool isAbsolute = pfs::path::isAbsolute(path);
    if (isAbsolute) {
        getRootInode(referenceFolder);
    } else {
        referenceFolder = m_currentDirInode;
    }


    std::vector<fs::DirectoryItem> directoryItems;
    /// Now we can iterate through every passed file name
    for (const auto &name : tokens) {
        if (name.empty()) {
            continue;
        }

        directoryItems = getDirectoryItems(referenceFolder);
        auto it = std::find_if(directoryItems.begin(), directoryItems.end(), [&name](const fs::DirectoryItem item) { return item.nameEquals(name); });
        if (it == directoryItems.end()) {
            throw std::invalid_argument("Předaná cesta neexistuje");
        }

        fs::Inode dirItemInode = findInode(it.base()->getInodeId());
        if (!dirItemInode.isDirectory()) {
            throw std::invalid_argument("Soubor v předané cestě není adresář");
        }

        referenceFolder = dirItemInode;
    }

    /// Setting found path as current path
    m_currentDirInode = referenceFolder;
    if (isAbsolute) {
        m_currentDirPath = path;
    } else {
        m_currentDirPath = pfs::path::createAbsolutePath(m_currentDirPath, path);
    }
}

void FileSystem::getRootInode(fs::Inode &rootInode) {
    std::ifstream dataFile(m_dataFileName, std::ios::in | std::ios::binary);
    if (!dataFile) {
        return;
    }

    rootInode.load(dataFile, m_superblock.getInodeStartAddress());
}

std::vector<fs::DirectoryItem> FileSystem::getDirectoryItems(const std::filesystem::path &dirPath) {
    const fs::Inode currentDirInode = m_currentDirInode;
    const std::string currentDirPath = m_currentDirPath;

    if (m_currentDirPath != dirPath) {
        changeDirectory(dirPath);
    }

    std::vector<fs::DirectoryItem> dirItems = getDirectoryItems(m_currentDirInode);

    m_currentDirInode = currentDirInode;
    m_currentDirPath = currentDirPath;

    return dirItems;
}

std::vector<fs::DirectoryItem> FileSystem::getDirectoryItems(const fs::Inode& directory) {
    return readDirItems(getAllDirectLinks(directory));
}

void FileSystem::saveDirItemIntoCurrent(const fs::DirectoryItem &directoryItem) {
    int32_t addressToStoreTo = m_currentDirInode.getLastFilledDirectLinkValue();
    if (addressToStoreTo == fs::EMPTY_LINK) {
        /**
         * Every index is free, we store to any free data block
         */
        saveDirItemToFreeDirectLink(directoryItem);
        return;
    }

    /// First we need to check if there is any free space in the last filled data block
    size_t indexInCluster = getFreeDirItemDataBlockSubindex(addressToStoreTo);
    if (indexInCluster < fs::Superblock::CLUSTER_SIZE) {
        /// We found free space in direct link and save there
        saveDirItemToAddress(directoryItem, m_superblock.getDataStartAddress() + (addressToStoreTo * fs::Superblock::CLUSTER_SIZE) + indexInCluster);
    } else {
        /// Entire direct link is full, we save into next direct or indirect
        if (m_currentDirInode.getFirstFreeDirectLink() < m_currentDirInode.getDirectLinks().size()) {
            /// There is free direct link, so we store directly and add direct link
            saveDirItemToFreeDirectLink(directoryItem);
        } else {
            /// Every direct link is filled, we store to indirect link
            saveDirItemToCurrentIndirect(directoryItem);
        }
    }

}


std::vector<fs::DirectoryItem> FileSystem::readDirItems(const std::vector<int32_t> &indexList) {
    std::ifstream dataFile(m_dataFileName, std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při čtení ze souboru");
    }

    std::vector<fs::DirectoryItem> directoryItems;

    for (const auto& index : indexList) {
        for (int i = 0; i < fs::Superblock::CLUSTER_SIZE; i += sizeof(fs::DirectoryItem)) {
            fs::DirectoryItem dirItem;
            dirItem.load(dataFile, m_superblock.getDataStartAddress() + (index * fs::Superblock::CLUSTER_SIZE) + i);
            if (dirItem.getItemName().at(0) != 0) {
                /// Ending character at the beginning of the file name would mean, that we read "empty" memory
                directoryItems.push_back(dirItem);
            }
        }
    }

    return directoryItems;
}

fs::Inode FileSystem::findInode(const int inodeId) {
    std::ifstream dataFile(m_dataFileName, std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při čtení ze souboru");
    }

    fs::Inode inode;
    inode.load(dataFile, m_superblock.getInodeStartAddress() + (inodeId * sizeof(fs::Inode)));
    if (inode.getInodeId() == inodeId) {
        return inode;
    }

    /// If we got here, the inode was not found
    throw pfs::ObjectNotFound("I-uzel nenalezen");
}

void FileSystem::saveInode(const fs::Inode &inode) {
    if (inode.getInodeId() == fs::FREE_INODE_ID) {
        throw std::invalid_argument("Nelze uložit i-uzel bez unikátního ID");
    }

    std::fstream dataFile(m_dataFileName, std::ios::out | std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru");
    }

    inode.save(dataFile, m_superblock.getInodeStartAddress() + (inode.getInodeId() * sizeof(fs::Inode)));

    /// Updating the bitmap
    m_inodeBitmap.setIndexFilled(inode.getInodeId());
    m_inodeBitmap.save(dataFile, m_superblock.getInodeBitmapStartAddress());
}

int32_t FileSystem::getFreeDataBlock() const {
    return m_dataBitmap.findFirstFreeIndex();
}

std::vector<int32_t> FileSystem::getFreeDataBlocks(const std::size_t count) const {
    return m_dataBitmap.findFreeIndexes(count);
}

void FileSystem::saveFileData(const fs::ClusteredFileData& clusteredData, const std::vector<int32_t>& dataClusterIndexes) {
    std::fstream dataFile(m_dataFileName, std::ios::out | std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru");
    }

    std::size_t cluster = fs::Inode::DIRECT_LINKS_COUNT;
    std::size_t processedLinksInIndirect = fs::Inode::LINKS_IN_INDIRECT;
    int32_t currentIndirectLink;
    for (int i = 0; i < dataClusterIndexes.size(); ++i) {
        if (i < fs::Inode::DIRECT_LINKS_COUNT) {
            m_dataBitmap.setIndexFilled(dataClusterIndexes.at(i));
            dataFile.seekp(m_superblock.getDataStartAddress() + (dataClusterIndexes.at(i) * fs::Superblock::CLUSTER_SIZE), std::ios_base::beg);
            dataFile.write(clusteredData.at(i).data(), fs::Superblock::CLUSTER_SIZE);
            dataFile.flush();
        } else {
            if (processedLinksInIndirect == fs::Inode::LINKS_IN_INDIRECT) {
                /// Just saving the indirect index, will be saving there other direct links, is already saved in inode
                currentIndirectLink = dataClusterIndexes.at(i);
                dataFile.seekg(m_superblock.getDataStartAddress() + (currentIndirectLink * fs::Superblock::CLUSTER_SIZE));
                for (int j = 0; j < fs::Superblock::CLUSTER_SIZE; ++j) {
                    dataFile.put(fs::EMPTY_LINK);
                }
                m_dataBitmap.setIndexFilled(currentIndirectLink);
                processedLinksInIndirect = 0;
            } else {
                /// Saving the direct data
                m_dataBitmap.setIndexFilled(dataClusterIndexes.at(i));
                dataFile.seekp(m_superblock.getDataStartAddress() +
                    dataClusterIndexes.at(i) * fs::Superblock::CLUSTER_SIZE, std::ios_base::beg);
                dataFile.write(clusteredData.at(cluster).data(), clusteredData.at(cluster).length());
                dataFile.flush();
                /// Saving a link to direct data to indirect data cluster
                dataFile.seekp(m_superblock.getDataStartAddress() + (currentIndirectLink * fs::Superblock::CLUSTER_SIZE)
                    + (processedLinksInIndirect * sizeof(int32_t)), std::ios_base::beg);
                dataFile.write((char*)&dataClusterIndexes.at(i), sizeof(int32_t));
                dataFile.flush();
                /// Incrementing needed variables
                cluster++;
                processedLinksInIndirect++;
            }
        }
    }

    /// In the end we need to save the changes we made to the bitmap
    m_dataBitmap.save(dataFile, m_superblock.getDataBitmapStartAddress());
}

void FileSystem::updateInodeBitmap() {
    std::fstream dataFile(m_dataFileName, std::ios::out | std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru");
    }

    m_inodeBitmap.save(dataFile, m_superblock.getInodeBitmapStartAddress());
}

void FileSystem::updateDataBitmap() {
    std::fstream dataFile(m_dataFileName, std::ios::out | std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru");
    }

    m_dataBitmap.save(dataFile, m_superblock.getDataBitmapStartAddress());
}

size_t FileSystem::getFreeDirItemDataBlockSubindex(int32_t dirItemDataBlockSubindex) {
    std::ifstream dataFileR(m_dataFileName, std::ios::in | std::ios::app);
    if (!dataFileR) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru");
    }

    size_t indexInCluster = 0;
    size_t offset = m_superblock.getDataStartAddress() + (dirItemDataBlockSubindex * fs::Superblock::CLUSTER_SIZE);
    while (indexInCluster < fs::Superblock::CLUSTER_SIZE) {
        fs::DirectoryItem dirItem;
        dirItem.load(dataFileR, offset + indexInCluster);
        if (dirItem.getItemName()[0] == '\0') {
            return indexInCluster;
        }

        indexInCluster += sizeof(fs::DirectoryItem);
    }

    return indexInCluster;
}

size_t FileSystem::getFreeIndirectLinkDataBlockSubindex(int32_t indirectLinkDatablockIndex) {
    std::ifstream dataFileR(m_dataFileName, std::ios::in | std::ios::app);
    if (!dataFileR) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru");
    }

    size_t indexInCluster = 0;
    size_t offset = m_superblock.getDataStartAddress() + (indirectLinkDatablockIndex * fs::Superblock::CLUSTER_SIZE);
    while (indexInCluster < fs::Superblock::CLUSTER_SIZE) {
        int32_t storedLink;
        dataFileR.seekg(offset, std::ios_base::beg);
        dataFileR.read((char*)&storedLink, sizeof(int32_t));
        if (storedLink == fs::EMPTY_LINK) {
            return indexInCluster;
        }

        indexInCluster += sizeof(fs::DirectoryItem);
        offset += sizeof(fs::DirectoryItem);
    }

    return indexInCluster;
}

void FileSystem::saveDirItemToIndex(const fs::DirectoryItem &directoryItem, const int32_t index) {
    int32_t address = m_superblock.getDataStartAddress() + (index * fs::Superblock::CLUSTER_SIZE);
    saveDirItemToAddress(directoryItem, address);

    /// When saving to new cluster, we need to make sure every other bit of memory is set to 0 (empty) for future i/o operations
    std::fstream dataFile(m_dataFileName, std::ios::out | std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru");
    }
    dataFile.seekp(address + sizeof(directoryItem), std::ios_base::beg);
    for (int i = sizeof(directoryItem); i < fs::Superblock::CLUSTER_SIZE; ++i) {
        dataFile.put('\0');
    }
    dataFile.flush();

    m_dataBitmap.setIndexFilled(index);
    m_dataBitmap.save(dataFile, m_superblock.getDataBitmapStartAddress());
}

void FileSystem::saveDirItemToAddress(const fs::DirectoryItem& directoryItem, const int32_t address) {
    std::fstream dataFile(m_dataFileName, std::ios::out | std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru");
    }

    directoryItem.save(dataFile, address);
}

bool FileSystem::saveDirItemToFreeDirectLink(const fs::DirectoryItem& directoryItem) {
    int32_t addressToStoreTo = getFreeDataBlock();

    saveDirItemToIndex(directoryItem, addressToStoreTo);
    return m_currentDirInode.addDirectLink(addressToStoreTo);
}

void FileSystem::saveDirItemToCurrentIndirect(const fs::DirectoryItem& directoryItem) {
    int32_t lastFilledIndirectLink = m_currentDirInode.getLastFilledIndirectLinkValue();
    if (lastFilledIndirectLink == fs::EMPTY_LINK) {
        /// Every indirect link is free, we store directly and save index to indirect
        saveDirItemToFreeIndirectLink(directoryItem);
        return;
    }

    /// First we need to check if there is any free space in the last filled indirect link
    size_t indexInCluster = getFreeIndirectLinkDataBlockSubindex(lastFilledIndirectLink);
    if (indexInCluster < fs::Superblock::CLUSTER_SIZE) {
        /// We found free space in indirect link, now we check if the previous direct link is filled
        /// If previous direct link is full, we store to new direct link, otherwise we store into
        /// the previous link
        size_t lastDirectLinkInIndirectLink;
        {
            std::ifstream dataFile(m_dataFileName, std::ios::out | std::ios::binary | std::ios::app);
            if (!dataFile) {
                throw std::ios_base::failure("Chyba při otevírání datového souboru!");
            }
            dataFile.seekg(
                    m_superblock.getDataStartAddress() + (lastFilledIndirectLink * fs::Superblock::CLUSTER_SIZE) +
                    (indexInCluster - sizeof(int32_t)), std::ios_base::beg);
            dataFile.read((char *) &lastDirectLinkInIndirectLink, sizeof(lastDirectLinkInIndirectLink));
        }

        size_t indexInDirectLink = getFreeDirItemDataBlockSubindex(lastDirectLinkInIndirectLink);
        if (indexInDirectLink < fs::Superblock::CLUSTER_SIZE) {
            /// We found free space in direct link and save there
            saveDirItemToAddress(directoryItem, m_superblock.getDataStartAddress() + (lastFilledIndirectLink * fs::Superblock::CLUSTER_SIZE) + indexInCluster);
        } else {
            int32_t addressToStoreTo = getFreeDataBlock();
            saveDirItemToIndex(directoryItem, addressToStoreTo);

            std::fstream dataFile(m_dataFileName, std::ios::out | std::ios::in | std::ios::binary);
            if (!dataFile) {
                throw std::ios_base::failure("Chyba při otevírání datového souboru!");
            }

            dataFile.seekp(m_superblock.getDataStartAddress() + (lastFilledIndirectLink * fs::Superblock::CLUSTER_SIZE) + indexInCluster, std::ios_base::beg);
            dataFile.write((char*)&addressToStoreTo, sizeof(addressToStoreTo));
            dataFile.flush();
        }
    } else {
        /// Last filled indirect link is full, we save into next indirect link
        if (m_currentDirInode.getFirstFreeIndirectLink() == m_currentDirInode.getIndirectLinks().size()) {
            /// Every direct and indirect link of current directory is filled, cannot save any more items
            throw pfs::ObjectNotFound("Cannot save any more directory items to current directory (" + m_currentDirPath + ")");
        }

        saveDirItemToFreeIndirectLink(directoryItem);
    }
}

void FileSystem::saveDirItemToFreeIndirectLink(const fs::DirectoryItem &directoryItem) {
    int32_t addressToStoreTo = getFreeDataBlock();
    saveDirItemToIndex(directoryItem, addressToStoreTo);

    int32_t newIndirectLink = getFreeDataBlock();
    m_currentDirInode.addIndirectLink(newIndirectLink);
    std::fstream dataFile(m_dataFileName, std::ios::out | std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru!");
    }

    dataFile.seekp(m_superblock.getDataStartAddress() + (newIndirectLink * fs::Superblock::CLUSTER_SIZE), std::ios_base::beg);
    dataFile.write((char*)&addressToStoreTo, sizeof(addressToStoreTo));
    /// Setting all other memory bits to -1 (empty) for future i/o operations
    dataFile.write((char*)&fs::EMPTY_LINK, fs::Superblock::CLUSTER_SIZE - sizeof(fs::EMPTY_LINK));
    dataFile.flush();
}

void FileSystem::clearInodeData(const fs::Inode &inode) {
    std::fstream dataFile(m_dataFileName, std::ios::out | std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru!");
    }

    std::array<int, fs::Superblock::CLUSTER_SIZE> buffer { 0 };
    for (const auto &directLink : getAllDirectLinks(inode)) {
        m_dataBitmap.setIndexFree(directLink);
        dataFile.seekp(m_superblock.getDataStartAddress() + (directLink * fs::Superblock::CLUSTER_SIZE));
        dataFile.write((char*)buffer.data(), buffer.size());
        dataFile.flush();
    }

    for (const auto &indirectLink : inode.getIndirectLinks()) {
        m_dataBitmap.setIndexFree(indirectLink);
        dataFile.seekp(m_superblock.getDataStartAddress() + (indirectLink * fs::Superblock::CLUSTER_SIZE));
        dataFile.write((char*) buffer.data(), buffer.size());
        dataFile.flush();
    }

    dataFile.seekp(m_superblock.getInodeStartAddress() + (inode.getInodeId() * sizeof(inode)));
    dataFile.write((char*)buffer.data(), sizeof(inode));
    dataFile.flush();

    m_dataBitmap.save(dataFile, m_superblock.getDataBitmapStartAddress());

    m_inodeBitmap.setIndexFree(inode.getInodeId());
    m_inodeBitmap.save(dataFile, m_superblock.getInodeBitmapStartAddress());

    m_currentDirInode.setFileSize(m_currentDirInode.getFileSize() - inode.getFileSize());
    saveInode(m_currentDirInode);
}

fs::DirectoryItem FileSystem::removeDirectoryItem(const std::string &filename) {


    fs::DirectoryItem dirItem;
    for (const auto &index : m_currentDirInode.getDirectLinks()) {
        if (index == fs::EMPTY_LINK) {
            continue;
        }
        try {
            dirItem = removeDirItemFromCluster(filename, index);
        } catch (const pfs::ObjectNotFound &ex) {
            /// We didn't find directory item in this index, we continue to the next one
            continue;
        }

        if (isDirItemIndexFree(index)) {
            m_dataBitmap.setIndexFree(index);
            m_currentDirInode.clearDirectLink(index);
        }

        return dirItem;
    }
    std::ifstream dataFile(m_dataFileName, std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios::failure("Chyba při otevírání datového souboru!");
    }

    int32_t directLink = fs::EMPTY_LINK;
    for (const auto &index : m_currentDirInode.getIndirectLinks()) {
        if (index == fs::EMPTY_LINK) {
            continue;
        }
        for (int i = 0; i < fs::Superblock::CLUSTER_SIZE; i += sizeof(int32_t)) {
            dataFile.seekg(m_superblock.getDataStartAddress() + (index * fs::Superblock::CLUSTER_SIZE) + i, std::ios::beg);
            dataFile.read((char*)&directLink, sizeof(int32_t));
            if (directLink == fs::EMPTY_LINK) {
                continue;
            }

            try {
                dirItem = removeDirItemFromCluster(filename, directLink);
            } catch (const pfs::ObjectNotFound& ex) {
                /// We didn't find directory item in this index, we continue to the next one
                continue;
            }

            if (isDirItemIndexFree(directLink)) {
                m_dataBitmap.setIndexFree(directLink);

                if (isIndirectLinkFree(index)) {
                    m_dataBitmap.setIndexFree(index);
                    m_currentDirInode.clearIndirectLink(index);
                }
            }

            return dirItem;
        }
    }

    throw pfs::ObjectNotFound("DirectoryItem s názvem " + filename + " nenalezen");
}

std::vector<int32_t> FileSystem::getAllDirectLinks(const fs::Inode &inode) {
    std::vector<int32_t> directLinks;

    for (const auto &directLink : inode.getDirectLinks()) {
        if (directLink != fs::EMPTY_LINK) {
            directLinks.push_back(directLink);
        }
    }

    std::ifstream dataFile(m_dataFileName, std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při čtení dat");
    }

    for (const auto &indirectLink : inode.getIndirectLinks()) {
        if (indirectLink == fs::EMPTY_LINK) {
            continue;
        }

        for (int i = 0; i < fs::Superblock::CLUSTER_SIZE; i += sizeof(int32_t)) {
            int32_t directLink;
            dataFile.seekg(m_superblock.getDataStartAddress() + (indirectLink * fs::Superblock::CLUSTER_SIZE) + i, std::ios_base::beg);
            dataFile.read((char*)&directLink, sizeof(directLink));
            if (directLink != fs::EMPTY_LINK) {
                directLinks.push_back(directLink);
            }
        }
    }

    return directLinks;
}

bool FileSystem::isDirItemIndexFree(const int32_t index) {
    std::ifstream dataFile(m_dataFileName, std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios::failure("Chyba při otevírání datového souboru!");
    }

    fs::DirectoryItem dirItem;
    dirItem.load(dataFile, m_superblock.getDataStartAddress() + (index * fs::Superblock::CLUSTER_SIZE));
    return ((dirItem.getInodeId() == 0) && (dirItem.getItemName()[0] == 0));
}

bool FileSystem::isIndirectLinkFree(const int32_t index) {
    std::ifstream dataFile(m_dataFileName, std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios::failure("Chyba při otevírání datového souboru!");
    }

    int32_t directLink;
    dataFile.seekg(m_superblock.getDataStartAddress() + (index * fs::Superblock::CLUSTER_SIZE));
    dataFile.read((char*)&directLink, sizeof(directLink));
    return directLink == 0;
}

fs::DirectoryItem FileSystem::removeDirItemFromCluster(const std::string &filename, const int index) {
    std::ifstream dataFile(m_dataFileName, std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios::failure("Chyba při otevírání datového souboru!");
    }

    fs::DirectoryItem dirItem;
    for (int i = 0; i < fs::Superblock::CLUSTER_SIZE; i += sizeof(fs::DirectoryItem)) {
        dirItem.load(dataFile, m_superblock.getDataStartAddress() + (index * fs::Superblock::CLUSTER_SIZE) + i);
        if (dirItem.nameEquals(filename)) {
            std::fstream dataFileW(m_dataFileName, std::ios::out | std::ios::in | std::ios::binary);
            if (!dataFileW) {
                throw std::ios::failure("Chyba při otevírání datového souboru");
            }

            dataFileW.seekp(m_superblock.getDataStartAddress() + (index * fs::Superblock::CLUSTER_SIZE) + i,
                            std::ios::beg);
            for (int j = 0; j < sizeof(fs::DirectoryItem); ++j) {
                dataFileW.put('\0');
            }

            return dirItem;
        }
    }

    throw pfs::ObjectNotFound("Directory item s názvem " + filename + " nenalezen");
}

void FileSystem::printFileContent(const std::filesystem::path &pathToFile) {
    if (!pathToFile.has_filename()) {
        throw std::invalid_argument("Předaná cesta nemá název souboru!");
    }

    std::string pathStr(pathToFile.string());
    std::string pathNoFilename(pathStr.substr(0, pathStr.length() - pathToFile.filename().string().length()));
    /// We store current working directory, so we can return back in the end
    const std::string currentDir = m_currentDirPath;
    const fs::Inode currentInode = m_currentDirInode;

    if (m_currentDirPath != pathNoFilename) {
        /// By changing to given path we not only get access to the i-node we need, but also validate it's existence
        changeDirectory(pathNoFilename);
    }

    fs::DirectoryItem dirItem = findDirectoryItem(pathToFile.filename());
    fs::Inode inode = findInode(dirItem.getInodeId());
    if (inode.isDirectory()) {
        throw std::invalid_argument("Obsah adresáře nelze vypsat! Použijte funkci \"ls\"!");
    }

    std::string fileContent = getFileContent(inode);
    std::cout << fileContent << '\n';

    if (currentDir != pathNoFilename) {
        m_currentDirPath = currentDir;
        m_currentDirInode = currentInode;
    }
}

fs::DirectoryItem FileSystem::findDirectoryItem(const std::filesystem::path &fileName) {
    std::ifstream dataFile(m_dataFileName, std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios::failure("Chyba při otevírání datového souboru!");
    }

    fs::DirectoryItem dirItem;
    for (const auto &directLink : m_currentDirInode.getDirectLinks()) {
        if (directLink == fs::EMPTY_LINK) {
            throw pfs::ObjectNotFound("Directory item s předaným názvem nenalezen!");
        }

        for (int i = 0; i < fs::Superblock::CLUSTER_SIZE; i += sizeof(fs::DirectoryItem)) {
            dirItem.load(dataFile,
                         m_superblock.getDataStartAddress() + (directLink * fs::Superblock::CLUSTER_SIZE) + i);
            if (dirItem.nameEquals(fileName)) {
                return dirItem;
            }
        }
    }

    int32_t directLink = fs::EMPTY_LINK;
    for (const auto &indirectLink : m_currentDirInode.getIndirectLinks()) {
        if (indirectLink == fs::EMPTY_LINK) {
            throw pfs::ObjectNotFound("Directory item s předaným názvem nenalezen!");
        }

        for (int i = 0; i < fs::Superblock::CLUSTER_SIZE; i += sizeof(int32_t)) {
            dataFile.seekg(m_superblock.getDataStartAddress() + (indirectLink * fs::Superblock::CLUSTER_SIZE) + i, std::ios::beg);
            dataFile.read((char*)&directLink, sizeof(directLink));
            if (directLink == fs::EMPTY_LINK) {
                continue;
            }
            for (int j = 0; j < fs::Superblock::CLUSTER_SIZE; j += sizeof(fs::DirectoryItem)) {
                dirItem.load(dataFile,
                             m_superblock.getDataStartAddress() + (directLink * fs::Superblock::CLUSTER_SIZE) + j);
                if (dirItem.nameEquals(fileName)) {
                    return dirItem;
                }
            }
        }
    }

    throw pfs::ObjectNotFound("Directory item s předaným názvem nenalezen!");
}

std::string FileSystem::getFileContent(const fs::Inode &inode) {
    if (inode.isDirectory()) {
        throw std::invalid_argument("Obsah složky nelze vypsat! Použijte funkci \"ls\"!");
    }

    std::ifstream dataFile(m_dataFileName, std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios::failure("Chyba při otevírání datového souboru!");
    }

    std::string fileContent;
    std::array<char, fs::Superblock::CLUSTER_SIZE> buffer { 0 };
    for (const auto &directLink : inode.getDirectLinks()) {
        if (directLink == fs::EMPTY_LINK) {
            return fileContent;
        }
        dataFile.seekg(m_superblock.getDataStartAddress() + (directLink * fs::Superblock::CLUSTER_SIZE), std::ios::beg);
        dataFile.read(buffer.data(), buffer.size());
        for (const auto& c : buffer) {
            if (fileContent.size() == inode.getFileSize()) {
                return fileContent;
            }
            fileContent += c;
        }
    }

    int32_t directLink = fs::EMPTY_LINK;
    for (const auto &indirectLink : inode.getIndirectLinks()) {
        if (indirectLink == fs::EMPTY_LINK) {
            return fileContent;
        }
        for (int i = 0; i < fs::Superblock::CLUSTER_SIZE; i += sizeof(int32_t)) {
            dataFile.seekg(m_superblock.getDataStartAddress() + (indirectLink * fs::Superblock::CLUSTER_SIZE) + i,
                           std::ios::beg);
            dataFile.read((char *) &directLink, sizeof(directLink));
            if (directLink == fs::EMPTY_LINK) {
                return fileContent;
            }
            dataFile.seekg(m_superblock.getDataStartAddress() + (directLink * fs::Superblock::CLUSTER_SIZE), std::ios::beg);
            dataFile.read(buffer.data(), buffer.size());
            for (const auto& c : buffer) {
                if (fileContent.size() == inode.getFileSize()) {
                    return fileContent;
                }
                fileContent += c;
            }
        }

    }

    return fileContent;
}

std::string FileSystem::getFileContent(const std::filesystem::path &pathToFile) {
    if (!pathToFile.has_filename()) {
        throw std::invalid_argument("Předaná cesta nemá název souboru!");
    }

    std::string pathStr(pathToFile.string());
    std::string pathNoFilename(pathStr.substr(0, pathStr.length() - pathToFile.filename().string().length()));
    /// We store current working directory, so we can return back in the end
    const std::string currentDir = m_currentDirPath;
    const fs::Inode currentInode = m_currentDirInode;

    if (m_currentDirPath != pathNoFilename) {
        /// By changing to given path we not only get access to the i-node we need, but also validate it's existence
        changeDirectory(pathNoFilename);
    }
    fs::DirectoryItem dirItem = findDirectoryItem(pathToFile.filename());
    fs::Inode inode = findInode(dirItem.getInodeId());
    if (inode.isDirectory()) {
        throw pfs::InvalidState("Obsah adresáře nelze vrátit! Použijte funkci \"ls\"!");
    }

    std::string fileContent = getFileContent(inode);

    if (currentDir != pathNoFilename) {
        m_currentDirPath = currentDir;
        m_currentDirInode = currentInode;
    }

    return fileContent;
}

void FileSystem::printFileInfo(const std::filesystem::path &pathToFile) {
    std::filesystem::path parentPath(pathToFile.parent_path());

    const std::string currentDir = m_currentDirPath;
    const fs::Inode currentInode = m_currentDirInode;

    if (m_currentDirPath != parentPath && !parentPath.empty()) {
        /// By changing to given path we not only get access to the i-node we need, but also validate it's existence
        changeDirectory(parentPath);
    }

    std::string filename;
    if (pathToFile.has_filename()) {
        filename = pathToFile.filename();
    } else {
        filename = std::filesystem::path(m_currentDirPath).filename();
    }

    fs::DirectoryItem dirItem = findDirectoryItem(filename);
    fs::Inode inode = findInode(dirItem.getInodeId());

    std::cout << "Name: " << dirItem.getItemName().data() << " - Size: " << inode.getFileSize() << " - Inode ID: " << inode.getInodeId() << " - ";
    std::cout << "Direct links: ";
    for (const auto &link : inode.getDirectLinks()) {
        if (link == fs::EMPTY_LINK) {
            break;
        }

        std::cout << link << " ";
    }
    std::cout << "Indirect links: ";
    for (const auto &link: inode.getIndirectLinks()) {
        if (link == fs::EMPTY_LINK) {
            break;
        }

        std::cout << link << " ";
    }
    std::cout << std::endl;

    if (currentDir != parentPath && !parentPath.empty()) {
        m_currentDirPath = currentDir;
        m_currentDirInode = currentInode;
    }
}

void FileSystem::createDirectory(const std::filesystem::path &path) {
    if (path.empty()) {
        throw std::invalid_argument("Path must not be empty!");
    }

    std::filesystem::path directory;
    std::filesystem::path parent;
    if (path.has_filename()) {
        directory = path.filename();
        parent = path.parent_path();
    } else {
        directory = path.parent_path().filename();
        parent = path.parent_path().parent_path();
    }


    const std::string currentDir = m_currentDirPath;
    const fs::Inode currentInode = m_currentDirInode;

    if (m_currentDirPath != parent && !parent.empty()) {
        /// By changing to given path we not only get access to the i-node we need, but also validate it's existence
        try {
            changeDirectory(parent);
        } catch (const std::exception& ex) {
            throw std::invalid_argument(fnct::PNF_DEST);
        }
    }

    std::vector<fs::DirectoryItem> dirItems(getDirectoryItems(m_currentDirPath));
    auto it = std::find_if(dirItems.begin(), dirItems.end(), [&directory](const fs::DirectoryItem item) { return item.nameEquals(directory.filename()); });
    if (it != dirItems.end()) {
        throw pfs::InvalidState(fnct::EXISTS);
    }

    fs::Inode inode(createInode(true, 0));
    fs::DirectoryItem directoryItem(directory.string(), inode.getInodeId());
    saveDirItemIntoCurrent(directoryItem);
    saveInode(inode);
    saveInode(m_currentDirInode);

    fs::DirectoryItem dirItemSelf(".", inode.getInodeId());
    fs::DirectoryItem dirItemParent("..", m_currentDirInode.getInodeId());

    changeDirectory(directory);
    saveDirItemIntoCurrent(dirItemSelf);
    saveDirItemIntoCurrent(dirItemParent);
    saveInode(m_currentDirInode);

    /// Returning back to the original directory
    m_currentDirInode = currentInode;
    m_currentDirPath = currentDir;
}

void FileSystem::removeDirectory(const std::filesystem::path &path) {
    if (path.empty()) {
        throw std::invalid_argument("Path must not be empty!");
    }

    std::filesystem::path directory;
    std::filesystem::path parent;
    if (path.has_filename()) {
        directory = path.filename();
        parent = path.parent_path();
    } else {
        directory = path.parent_path().filename();
        parent = path.parent_path().parent_path();
    }

    const std::string currentDir = m_currentDirPath;
    const fs::Inode currentInode = m_currentDirInode;

    if (m_currentDirPath != parent && !parent.empty()) {
        /// By changing to given path we not only get access to the i-node we need, but also validate it's existence
        try {
            changeDirectory(parent);
        } catch (const std::exception &ex) {
            throw std::invalid_argument(fnct::FNF_DIR);
        }
    }

    fs::DirectoryItem dirItem = findDirectoryItem(directory);
    fs::Inode inode = findInode(dirItem.getInodeId());
    if (!inode.isDirectory()) {
        throw std::invalid_argument(fnct::FNF_DIR);
    }
    /// We fill direct links from the front, so the second one has to be EMPTY_LINK and the first one has to have stored only
    /// . and .. directory items
    if (inode.getDirectLinks()[1] != fs::EMPTY_LINK ||
            getFreeDirItemDataBlockSubindex(inode.getDirectLinks()[0]) != 2 * sizeof(dirItem)) {
        throw pfs::InvalidState(fnct::NOT_EMPTY);
    }

    removeDirectoryItem(directory.string());
    clearInodeData(inode);

    if (m_currentDirPath != parent && !parent.empty()) {
        /// Returning back to the original directory
        m_currentDirInode = currentInode;
        m_currentDirPath = currentDir;
    }
}

void FileSystem::copyFile(const std::filesystem::path &pathFrom, const std::filesystem::path &pathTo) {
    if (pathFrom.empty() || pathTo.empty()) {
        throw std::invalid_argument("Paths must not be empty!");
    }

    std::string fileContent;
    try {
        fileContent = getFileContent(pathFrom);
    } catch (const std::exception &ex) {
        throw std::invalid_argument(fnct::FNF_SOURCE);
    }

    try {
        createFile(pathTo, fs::FileData(fileContent));
    } catch (const std::exception &ex) {
        throw std::invalid_argument(fnct::PNF_DEST);
    }
}

void FileSystem::moveFile(const std::filesystem::path &pathFrom, const std::filesystem::path &pathTo) {
    if (pathFrom.empty() || pathTo.empty()) {
        throw std::invalid_argument("Paths must not be empty!");
    }

    copyFile(pathFrom, pathTo);
    removeFile(pathFrom);
}
