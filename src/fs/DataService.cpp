//
// Created by markovda on 24.01.21.
//

#include "DataService.h"

pfs::DataService::DataService(std::string dataFileName, fs::Bitmap dataBitmap, int32_t dataBitmapAddress,
                              int32_t dataStartAddress) : m_dataFileName(std::move(dataFileName)),
                              m_dataBitmap(std::move(dataBitmap)), m_dataBitmapAddress(dataBitmapAddress),
                              m_dataStartAddress(dataStartAddress) {}

std::vector<fs::DirectoryItem> pfs::DataService::getDirectoryItems(const fs::Inode& directory) const {
    if (!directory.isDirectory()) {
        throw std::invalid_argument("Předaný i-uzel musí být složka!");
    }
    return readDirItems(getAllDirectLinks(directory));
}

std::vector<int32_t> pfs::DataService::getAllDirectLinks(const fs::Inode &inode) const {
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
            dataFile.seekg(m_dataStartAddress + (indirectLink * fs::Superblock::CLUSTER_SIZE) + i, std::ios_base::beg);
            dataFile.read((char*)&directLink, sizeof(directLink));
            if (directLink != fs::EMPTY_LINK) {
                directLinks.push_back(directLink);
            }
        }
    }

    return directLinks;
}

int32_t pfs::DataService::getFreeDataBlock() const {
    return m_dataBitmap.findFirstFreeIndex();
}

std::vector<int32_t> pfs::DataService::getFreeDataBlocks(const std::size_t count) const {
    return m_dataBitmap.findFreeIndexes(count);
}

void pfs::DataService::saveFileData(const fs::ClusteredFileData& clusteredData, const std::vector<int32_t>& dataClusterIndexes) {
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
            dataFile.seekp(m_dataStartAddress + (dataClusterIndexes.at(i) * fs::Superblock::CLUSTER_SIZE), std::ios_base::beg);
            dataFile.write(clusteredData.at(i).data(), fs::Superblock::CLUSTER_SIZE);
            dataFile.flush();
        } else {
            if (processedLinksInIndirect == fs::Inode::LINKS_IN_INDIRECT) {
                /// Just saving the indirect index, will be saving there other direct links, is already saved in inode
                currentIndirectLink = dataClusterIndexes.at(i);
                dataFile.seekg(m_dataStartAddress + (currentIndirectLink * fs::Superblock::CLUSTER_SIZE));
                for (int j = 0; j < fs::Superblock::CLUSTER_SIZE; ++j) {
                    dataFile.put(fs::EMPTY_LINK);
                }
                m_dataBitmap.setIndexFilled(currentIndirectLink);
                processedLinksInIndirect = 0;
            } else {
                /// Saving the direct data
                m_dataBitmap.setIndexFilled(dataClusterIndexes.at(i));
                dataFile.seekp(m_dataStartAddress +
                               dataClusterIndexes.at(i) * fs::Superblock::CLUSTER_SIZE, std::ios_base::beg);
                dataFile.write(clusteredData.at(cluster).data(), clusteredData.at(cluster).length());
                dataFile.flush();
                /// Saving a link to direct data to indirect data cluster
                dataFile.seekp(m_dataStartAddress + (currentIndirectLink * fs::Superblock::CLUSTER_SIZE)
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
    m_dataBitmap.save(dataFile, m_dataBitmapAddress);
}


std::vector<fs::DirectoryItem> pfs::DataService::readDirItems(const std::vector<int32_t> &indexList) const {
    std::ifstream dataFile(m_dataFileName, std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při čtení ze souboru");
    }

    std::vector<fs::DirectoryItem> directoryItems;

    for (const auto& index : indexList) {
        for (int i = 0; i < fs::Superblock::CLUSTER_SIZE; i += sizeof(fs::DirectoryItem)) {
            fs::DirectoryItem dirItem;
            dirItem.load(dataFile, m_dataStartAddress + (index * fs::Superblock::CLUSTER_SIZE) + i);
            if (dirItem.getItemName().at(0) != 0) {
                /// Ending character at the beginning of the file name would mean, that we read "empty" memory
                directoryItems.push_back(dirItem);
            }
        }
    }

    return directoryItems;
}

void pfs::DataService::saveDirItemIntoDirectory(const fs::DirectoryItem &directoryItem, fs::Inode& directory) {
    if (!directory.isDirectory()) {
        throw std::invalid_argument("Předaný i-uzel musí být složka!");
    }

    int32_t addressToStoreTo = directory.getLastFilledDirectLinkValue();
    if (addressToStoreTo == fs::EMPTY_LINK) {
        /**
         * Every index is free, we store to any free data block
         */
        saveDirItemToFreeDirectLink(directoryItem, directory);
        return;
    }

    /// First we need to check if there is any free space in the last filled data block
    size_t indexInCluster = getFreeDirItemDataBlockSubindex(addressToStoreTo);
    if (indexInCluster < fs::Superblock::CLUSTER_SIZE) {
        /// We found free space in direct link and save there
        saveDirItemToAddress(directoryItem, m_dataStartAddress + (addressToStoreTo * fs::Superblock::CLUSTER_SIZE) + indexInCluster);
    } else {
        /// Entire direct link is full, we save into next direct or indirect
        if (directory.getFirstFreeDirectLink() < directory.getDirectLinks().size()) {
            /// There is free direct link, so we store directly and add direct link
            saveDirItemToFreeDirectLink(directoryItem, directory);
        } else {
            /// Every direct link is filled, we store to indirect link
            saveDirItemToIndirect(directoryItem, directory);
        }
    }

}

bool pfs::DataService::saveDirItemToFreeDirectLink(const fs::DirectoryItem& directoryItem, fs::Inode& directory) {
    int32_t addressToStoreTo = getFreeDataBlock();

    saveDirItemToIndex(directoryItem, addressToStoreTo);
    return directory.addDirectLink(addressToStoreTo);
}

void pfs::DataService::saveDirItemToIndex(const fs::DirectoryItem &directoryItem, const int32_t index) {
    int32_t address = m_dataStartAddress + (index * fs::Superblock::CLUSTER_SIZE);
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
    m_dataBitmap.save(dataFile, m_dataBitmapAddress);
}

void pfs::DataService::saveDirItemToAddress(const fs::DirectoryItem& directoryItem, const int32_t address) const {
    std::fstream dataFile(m_dataFileName, std::ios::out | std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru");
    }

    directoryItem.save(dataFile, address);
}

size_t pfs::DataService::getFreeDirItemDataBlockSubindex(const int32_t dirItemDataBlockSubindex) const {
    std::ifstream dataFileR(m_dataFileName, std::ios::in | std::ios::app);
    if (!dataFileR) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru");
    }

    size_t indexInCluster = 0;
    size_t offset = m_dataStartAddress + (dirItemDataBlockSubindex * fs::Superblock::CLUSTER_SIZE);
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

size_t pfs::DataService::getFreeIndirectLinkDataBlockSubindex(const int32_t indirectLinkDatablockIndex) const {
    std::ifstream dataFileR(m_dataFileName, std::ios::in | std::ios::app);
    if (!dataFileR) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru");
    }

    size_t indexInCluster = 0;
    size_t offset = m_dataStartAddress + (indirectLinkDatablockIndex * fs::Superblock::CLUSTER_SIZE);
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

void pfs::DataService::saveDirItemToIndirect(const fs::DirectoryItem& directoryItem, fs::Inode& directory) {
    int32_t lastFilledIndirectLink = directory.getLastFilledIndirectLinkValue();
    if (lastFilledIndirectLink == fs::EMPTY_LINK) {
        /// Every indirect link is free, we store directly and save index to indirect
        saveDirItemToFreeIndirectLink(directoryItem, directory);
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
            dataFile.seekg(m_dataStartAddress + (lastFilledIndirectLink * fs::Superblock::CLUSTER_SIZE) +
                    (indexInCluster - sizeof(int32_t)), std::ios_base::beg);
            dataFile.read((char *) &lastDirectLinkInIndirectLink, sizeof(lastDirectLinkInIndirectLink));
        }

        size_t indexInDirectLink = getFreeDirItemDataBlockSubindex(lastDirectLinkInIndirectLink);
        if (indexInDirectLink < fs::Superblock::CLUSTER_SIZE) {
            /// We found free space in direct link and save there
            saveDirItemToAddress(directoryItem, m_dataStartAddress + (lastFilledIndirectLink * fs::Superblock::CLUSTER_SIZE) + indexInCluster);
        } else {
            int32_t addressToStoreTo = getFreeDataBlock();
            saveDirItemToIndex(directoryItem, addressToStoreTo);

            std::fstream dataFile(m_dataFileName, std::ios::out | std::ios::in | std::ios::binary);
            if (!dataFile) {
                throw std::ios_base::failure("Chyba při otevírání datového souboru!");
            }

            dataFile.seekp(m_dataStartAddress + (lastFilledIndirectLink * fs::Superblock::CLUSTER_SIZE) + indexInCluster, std::ios_base::beg);
            dataFile.write((char*)&addressToStoreTo, sizeof(addressToStoreTo));
            dataFile.flush();
        }
    } else {
        /// Last filled indirect link is full, we save into next indirect link
        if (directory.getFirstFreeIndirectLink() == directory.getIndirectLinks().size()) {
            /// Every direct and indirect link of current directory is filled, cannot save any more items
            throw pfs::ObjectNotFound("Do předaného adresáře nelze uložit další soubory!");
        }

        saveDirItemToFreeIndirectLink(directoryItem, directory);
    }
}

void pfs::DataService::saveDirItemToFreeIndirectLink(const fs::DirectoryItem &directoryItem, fs::Inode& directory) {
    int32_t addressToStoreTo = getFreeDataBlock();
    saveDirItemToIndex(directoryItem, addressToStoreTo);

    int32_t newIndirectLink = getFreeDataBlock();
    directory.addIndirectLink(newIndirectLink);
    std::fstream dataFile(m_dataFileName, std::ios::out | std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru!");
    }

    dataFile.seekp(m_dataStartAddress + (newIndirectLink * fs::Superblock::CLUSTER_SIZE), std::ios_base::beg);
    dataFile.write((char*)&addressToStoreTo, sizeof(addressToStoreTo));
    /// Setting all other memory bits to -1 (empty) for future i/o operations
    for (int i = sizeof(addressToStoreTo); i < fs::Superblock::CLUSTER_SIZE; ++i) {
        dataFile.put(fs::EMPTY_LINK);
    }
    dataFile.flush();
}

void pfs::DataService::clearInodeData(const fs::Inode &inode) {
    std::fstream dataFile(m_dataFileName, std::ios::out | std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru!");
    }

    std::array<int, fs::Superblock::CLUSTER_SIZE> buffer { 0 };
    for (const auto &directLink : getAllDirectLinks(inode)) {
        m_dataBitmap.setIndexFree(directLink);
        dataFile.seekp(m_dataStartAddress + (directLink * fs::Superblock::CLUSTER_SIZE));
        dataFile.write((char*)buffer.data(), buffer.size());
        dataFile.flush();
    }

    for (const auto &indirectLink : inode.getIndirectLinks()) {
        m_dataBitmap.setIndexFree(indirectLink);
        dataFile.seekp(m_dataStartAddress + (indirectLink * fs::Superblock::CLUSTER_SIZE));
        dataFile.write((char*) buffer.data(), buffer.size());
        dataFile.flush();
    }

    m_dataBitmap.save(dataFile, m_dataBitmapAddress);
}

fs::DirectoryItem pfs::DataService::removeDirectoryItem(const std::string &filename, fs::Inode& directory) {
    fs::DirectoryItem dirItem;

    for (const auto &index : directory.getDirectLinks()) {
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
            directory.clearDirectLink(index);
        }

        return dirItem;
    }

    std::ifstream dataFile(m_dataFileName, std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios::failure("Chyba při otevírání datového souboru!");
    }

    int32_t directLink = fs::EMPTY_LINK;
    for (const auto &index : directory.getIndirectLinks()) {
        if (index == fs::EMPTY_LINK) {
            continue;
        }

        for (int i = 0; i < fs::Superblock::CLUSTER_SIZE; i += sizeof(int32_t)) {
            dataFile.seekg(m_dataStartAddress + (index * fs::Superblock::CLUSTER_SIZE) + i, std::ios::beg);
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
                    directory.clearIndirectLink(index);
                }
            }

            return dirItem;
        }
    }

    throw pfs::ObjectNotFound("DirectoryItem s názvem " + filename + " nenalezen");
}

bool pfs::DataService::isDirItemIndexFree(const int32_t index) const {
    std::ifstream dataFile(m_dataFileName, std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios::failure("Chyba při otevírání datového souboru!");
    }

    fs::DirectoryItem dirItem;
    dirItem.load(dataFile, m_dataStartAddress + (index * fs::Superblock::CLUSTER_SIZE));
    return ((dirItem.getInodeId() == 0) && (dirItem.getItemName()[0] == 0));
}

bool pfs::DataService::isIndirectLinkFree(const int32_t index) const {
    std::ifstream dataFile(m_dataFileName, std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios::failure("Chyba při otevírání datového souboru!");
    }

    int32_t directLink;
    dataFile.seekg(m_dataStartAddress + (index * fs::Superblock::CLUSTER_SIZE));
    dataFile.read((char*)&directLink, sizeof(directLink));
    return directLink == 0;
}

fs::DirectoryItem pfs::DataService::removeDirItemFromCluster(const std::string &filename, const int index) const {
    std::ifstream dataFile(m_dataFileName, std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios::failure("Chyba při otevírání datového souboru!");
    }

    fs::DirectoryItem dirItem;
    for (int i = 0; i < fs::Superblock::CLUSTER_SIZE; i += sizeof(fs::DirectoryItem)) {
        dirItem.load(dataFile, m_dataStartAddress + (index * fs::Superblock::CLUSTER_SIZE) + i);
        if (dirItem.nameEquals(filename)) {
            std::fstream dataFileW(m_dataFileName, std::ios::out | std::ios::in | std::ios::binary);
            if (!dataFileW) {
                throw std::ios::failure("Chyba při otevírání datového souboru");
            }

            dataFileW.seekp(m_dataStartAddress + (index * fs::Superblock::CLUSTER_SIZE) + i, std::ios::beg);
            for (int j = 0; j < sizeof(fs::DirectoryItem); ++j) {
                dataFileW.put('\0');
            }

            return dirItem;
        }
    }

    throw pfs::ObjectNotFound("Directory item s názvem " + filename + " nenalezen");
}

fs::DirectoryItem pfs::DataService::findDirectoryItem(const std::filesystem::path &fileName, const fs::Inode& directory) const {
    std::ifstream dataFile(m_dataFileName, std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios::failure("Chyba při otevírání datového souboru!");
    }

    fs::DirectoryItem dirItem;
    for (const auto &directLink : directory.getDirectLinks()) {
        if (directLink == fs::EMPTY_LINK) {
            throw pfs::ObjectNotFound("Directory item s předaným názvem nenalezen!");
        }

        for (int i = 0; i < fs::Superblock::CLUSTER_SIZE; i += sizeof(fs::DirectoryItem)) {
            dirItem.load(dataFile,m_dataStartAddress + (directLink * fs::Superblock::CLUSTER_SIZE) + i);
            if (dirItem.nameEquals(fileName)) {
                return dirItem;
            }
        }
    }

    int32_t directLink = fs::EMPTY_LINK;
    for (const auto &indirectLink : directory.getIndirectLinks()) {
        if (indirectLink == fs::EMPTY_LINK) {
            throw pfs::ObjectNotFound("Directory item s předaným názvem nenalezen!");
        }

        for (int i = 0; i < fs::Superblock::CLUSTER_SIZE; i += sizeof(int32_t)) {
            dataFile.seekg(m_dataStartAddress + (indirectLink * fs::Superblock::CLUSTER_SIZE) + i, std::ios::beg);
            dataFile.read((char*)&directLink, sizeof(directLink));
            if (directLink == fs::EMPTY_LINK) {
                continue;
            }
            for (int j = 0; j < fs::Superblock::CLUSTER_SIZE; j += sizeof(fs::DirectoryItem)) {
                dirItem.load(dataFile,m_dataStartAddress + (directLink * fs::Superblock::CLUSTER_SIZE) + j);
                if (dirItem.nameEquals(fileName)) {
                    return dirItem;
                }
            }
        }
    }

    throw pfs::ObjectNotFound("Directory item s předaným názvem nenalezen!");
}

std::string pfs::DataService::getFileContent(const fs::Inode &inode) const {
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
        dataFile.seekg(m_dataStartAddress + (directLink * fs::Superblock::CLUSTER_SIZE), std::ios::beg);
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
            dataFile.seekg(m_dataStartAddress + (indirectLink * fs::Superblock::CLUSTER_SIZE) + i, std::ios::beg);
            dataFile.read((char *) &directLink, sizeof(directLink));
            if (directLink == fs::EMPTY_LINK) {
                return fileContent;
            }
            dataFile.seekg(m_dataStartAddress + (directLink * fs::Superblock::CLUSTER_SIZE), std::ios::beg);
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

