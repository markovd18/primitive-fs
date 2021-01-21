//
// Author: markovd@students.zcu.cz
//

#include <fstream>
#include <iostream>
#include "structures.h"

namespace fs {

    Superblock::Superblock(const size_t newDiskSize) : m_signature(), m_volumeDescription() {
        strncpy(m_signature.data(), AUTHOR_NAME, SIGNATURE_LENGTH);
        strncpy(m_volumeDescription.data(), VOLUME_DESCRIPTION, VOLUME_DESC_LENGTH);
        /**
         * Converting passed size in megabytes to bytes.
         */
        m_diskSize = newDiskSize * 1000000;
        /**
         * Setting the i-node count to 1/1000 of the disk size
         */
        m_inodeCount = m_diskSize / 1000;
        m_inodeBitmapStartAddress = sizeof(Superblock);
        m_dataBitmapStartAddress = m_inodeBitmapStartAddress +
                ((m_inodeCount % 8 == 0) ? (m_inodeCount / 8) : ((m_inodeCount / 8) + 1));

        size_t inodeStorageSize = m_inodeCount * sizeof(Inode);
        /**
         * Size of storage for data-block bitmap and blocks themselves.
         */
        size_t dataMapAndStorageSize = m_diskSize - m_dataBitmapStartAddress - inodeStorageSize;
        /**
         * Maximum cluster count if we didn't store data-block bitmap.
         */
        size_t clusterCountNoMap = dataMapAndStorageSize / CLUSTER_SIZE;

        size_t clustersToRemove = 0;
        while ((clustersToRemove * CLUSTER_SIZE) < clusterCountNoMap) {
            clustersToRemove++;
        }

        m_clusterCount = clusterCountNoMap - clustersToRemove;

        m_inodeStartAddress = m_dataBitmapStartAddress +
                ((m_clusterCount % 8 == 0) ? (m_clusterCount / 8) : ((m_clusterCount / 8) + 1));
        m_dataStartAddress = m_inodeStartAddress + inodeStorageSize;
    }

    const std::array<char, Superblock::SIGNATURE_LENGTH> &Superblock::getSignature() const {
        return m_signature;
    }

    const std::array<char, Superblock::VOLUME_DESC_LENGTH> &Superblock::getVolumeDescription() const {
        return m_volumeDescription;
    }

    int32_t Superblock::getDiskSize() const {
        return m_diskSize;
    }

    int32_t Superblock::getClusterCount() const {
        return m_clusterCount;
    }

    int32_t Superblock::getInodeBitmapStartAddress() const {
        return m_inodeBitmapStartAddress;
    }

    int32_t Superblock::getDataBitmapStartAddress() const {
        return m_dataBitmapStartAddress;
    }

    int32_t Superblock::getInodeStartAddress() const {
        return m_inodeStartAddress;
    }

    int32_t Superblock::getDataStartAddress() const {
        return m_dataStartAddress;
    }

    int32_t Superblock::getInodeCount() const {
        return m_inodeCount;
    }

    void Superblock::load(std::ifstream &dataFile, const size_t address) {
        if (!dataFile.is_open()) {
            throw std::invalid_argument("Předaný datový soubor není otevřen ke čtení");
        }

        dataFile.seekg(address, std::ios_base::beg);
        dataFile.read(m_signature.data(), SIGNATURE_LENGTH);
        dataFile.read(m_volumeDescription.data(), VOLUME_DESC_LENGTH);
        dataFile.read((char*)&m_diskSize, sizeof(m_diskSize));
        dataFile.read((char*)&m_inodeCount, sizeof(m_inodeCount));
        dataFile.read((char*)&m_clusterCount, sizeof(m_clusterCount));
        dataFile.read((char*)&m_inodeBitmapStartAddress, sizeof(m_inodeBitmapStartAddress));
        dataFile.read((char*)&m_dataBitmapStartAddress, sizeof(m_dataBitmapStartAddress));
        dataFile.read((char*)&m_inodeStartAddress, sizeof(m_inodeStartAddress));
        dataFile.read((char*)&m_dataStartAddress, sizeof(m_dataStartAddress));
    }

    void Superblock::save(std::fstream &dataFile, const size_t address) const {
        if (!dataFile.is_open()) {
            throw std::invalid_argument("Předaný datový soubor není otevřen k zápisu");
        }

        dataFile.seekp(address, std::ios_base::beg);
        dataFile.write(m_signature.data(), SIGNATURE_LENGTH);
        dataFile.write(m_volumeDescription.data(), VOLUME_DESC_LENGTH);
        dataFile.write((char*)&m_diskSize, sizeof(m_diskSize));
        dataFile.write((char*)&m_inodeCount, sizeof(m_inodeCount));
        dataFile.write((char*)&m_clusterCount, sizeof(m_clusterCount));
        dataFile.write((char*)&m_inodeBitmapStartAddress, sizeof(m_inodeBitmapStartAddress));
        dataFile.write((char*)&m_dataBitmapStartAddress, sizeof(m_dataBitmapStartAddress));
        dataFile.write((char*)&m_inodeStartAddress, sizeof(m_inodeStartAddress));
        dataFile.write((char*)&m_dataStartAddress, sizeof(m_dataStartAddress));
        dataFile.flush();
    }


    DirectoryItem::DirectoryItem(const std::string &itemName, const int32_t inodeId) : m_inodeId(inodeId), m_itemName(){
        if (itemName.length() > 12) {
            strncpy(this->m_itemName.data(), itemName.c_str(), DIR_ITEM_NAME_LENGTH);
        } else {
            strncpy(this->m_itemName.data(), itemName.c_str(), itemName.length());
        }
    }

    int32_t DirectoryItem::getInodeId() const {
        return m_inodeId;
    }

    const std::array<char, DirectoryItem::DIR_ITEM_NAME_LENGTH> &DirectoryItem::getItemName() const {
        return m_itemName;
    }

    bool DirectoryItem::nameEquals(const std::string &name) const {
        if (name.length() != getItemNameLength()) {
            return false;
        }

        return name == m_itemName.data();
    }

    int DirectoryItem::getItemNameLength() const {
        for (int i = 0; i < DIR_ITEM_NAME_LENGTH; ++i) {
            if (m_itemName.at(i) == '\0') {
                return i;
            }
        }
        return DIR_ITEM_NAME_LENGTH;
    }

    void DirectoryItem::save(std::fstream &dataFile, size_t address) const {
        if (!dataFile.is_open()) {
            throw std::invalid_argument("Předaný datový soubor není otevřený k zápisu");
        }

        std::cout << "Saving DirectoryItem: InodeId=" << m_inodeId << ", ItemName=" << m_itemName.data()
            << "to address " << address << std::endl;
        dataFile.seekp(address, std::ios_base::beg);
        dataFile.write((char*)&m_inodeId, sizeof(m_inodeId));
        dataFile.write(m_itemName.data(), m_itemName.size());
        dataFile.flush();
    }

    void DirectoryItem::load(std::ifstream &dataFile, size_t address) {
        if (!dataFile.is_open()) {
            throw std::invalid_argument("Předaný datový soubor není otevřený ke čtení");
        }

        dataFile.seekg(address, std::ios_base::beg);
        dataFile.read((char*)&m_inodeId, sizeof(m_inodeId));
        dataFile.read((char*)m_itemName.data(), m_itemName.size());
        std::cout << "Loaded DirectoryItem: InodeId=" << m_inodeId << ", ItemName=" << m_itemName.data()
                  << "from address " << address << std::endl;
    }

    Inode::Inode() {
        init();
    }
    Inode::Inode(int32_t nodeId, bool isDirectory, int32_t fileSize) : m_inodeId(nodeId), m_isDirectory(isDirectory),
                                                                       m_fileSize(fileSize){
        init();
    }

    int32_t Inode::getInodeId() const {
        return m_inodeId;
    }

    void Inode::setInodeId(int32_t nodeId) {
        m_inodeId = nodeId;
    }

    bool Inode::isDirectory() const {
        return m_isDirectory;
    }

    void Inode::setIsDirectory(bool isDirectory) {
        m_isDirectory = isDirectory;
    }

    int8_t Inode::getReferences() const {
        return m_references;
    }

    void Inode::setReferences(int8_t references) {
        m_references = references;
    }

    int32_t Inode::getFileSize() const {
        return m_fileSize;
    }

    void Inode::setFileSize(int32_t fileSize) {
        m_fileSize = fileSize;
    }
    const std::array<int32_t, Inode::DIRECT_LINKS_COUNT> &Inode::getDirectLinks() const {
        return m_directLinks;
    }

    const std::array<int32_t, Inode::INDIRECT_LINKS_COUNT> &Inode::getIndirectLinks() const {
        return m_indirectLinks;
    }

    int32_t Inode::getLastFilledDirectLinkValue() const {
        return getLastFilledIndexValue(m_directLinks, fs::EMPTY_LINK);
    }

    int32_t Inode::getFirstFreeDirectLink() const {
        return getFirstFreeIndex(m_directLinks, fs::EMPTY_LINK);
    }

    int32_t Inode::getLastFilledIndirectLinkValue() const {
        return getLastFilledIndexValue(m_indirectLinks, fs::EMPTY_LINK);
    }

    int32_t Inode::getFirstFreeIndirectLink() const {
        return getFirstFreeIndex(m_indirectLinks, fs::EMPTY_LINK);
    }

    template<typename T, typename V>
    V getLastFilledIndexValue(const T& container, const V emptyValue) {
        int32_t lastFilled = emptyValue;
        for (const auto &index : container) {
            if (index == emptyValue) {
                return lastFilled;
            } else {
                lastFilled = index;
            }
        }

        return lastFilled;
    }

    template<typename T, typename V>
    V getFirstFreeIndex(const T& container, const V emptyValue) {
        for (int i = 0; i < container.size(); ++i) {
            if (container[i] == emptyValue) {
                return i;
            }
        }

        return container.size();
    }

    void Inode::save(std::fstream &dataFile, size_t address) const {
        if (!dataFile) {
            throw std::invalid_argument("Předaný datový soubor není otevřen k zápisu");
        }
        std::cout << "Saving Inode: InodeId=" << m_inodeId << ", IsDirectory=" << m_isDirectory << ", FileSize=" << m_fileSize
            << " to address " << address << std::endl;
        dataFile.seekp(address, std::ios_base::beg);
        dataFile.write((char*)&m_inodeId, sizeof(m_inodeId));
        dataFile.write((char*)&m_isDirectory, sizeof(m_isDirectory));
        dataFile.write((char*)&m_references, sizeof(m_references));
        dataFile.write((char*)&m_fileSize, sizeof(m_fileSize));
        dataFile.write((char*)m_directLinks.data(), (m_directLinks.size() * sizeof(int32_t)));
        dataFile.write((char*)m_indirectLinks.data(), (m_indirectLinks.size() * sizeof(int32_t)));
        dataFile.flush();
    }

    void Inode::load(std::ifstream &dataFile, size_t address) {
        if (!dataFile) {
            throw std::invalid_argument("Předaný datový soubor není otevřen ke čtení");
        }

        dataFile.seekg(address, std::ios_base::beg);
        dataFile.read((char*)&m_inodeId, sizeof(m_inodeId));
        dataFile.read((char*)&m_isDirectory, sizeof(m_isDirectory));
        dataFile.read((char*)&m_references, sizeof(m_references));
        dataFile.read((char*)&m_fileSize, sizeof(m_fileSize));
        dataFile.read((char*)m_directLinks.data(), (m_directLinks.size() * sizeof(int32_t)));
        dataFile.read((char*)m_indirectLinks.data(), (m_indirectLinks.size() * sizeof(int32_t)));
        std::cout << "Loaded Inode: InodeId=" << m_inodeId << ", IsDirectory=" << m_isDirectory << ", FileSize=" << m_fileSize
                  << " from address " << address << std::endl;
    }

    bool Inode::addDirectLink(int32_t address) {
        for (auto & directLink : m_directLinks) {
            if (directLink == fs::EMPTY_LINK) {
                directLink = address;
                return true;
            }
        }
        return false;
    }

    bool Inode::addIndirectLink(int32_t address) {
        for (auto &indirectLink : m_indirectLinks) {
            if (indirectLink == fs::EMPTY_LINK) {
                indirectLink = address;
                return true;
            }
        }
        return false;
    }

    void Inode::clearData() {
        init();
    }

    void Inode::setData(const DataLinks &dataLinks) {
        m_directLinks = dataLinks.getDirectLinks();
        m_indirectLinks = dataLinks.getIndirectLinks();
    }

    void Inode::clearDirectLink(const int32_t index) {
        for (int & directLink : m_directLinks) {
            if (directLink == index) {
                directLink = fs::EMPTY_LINK;
            }
        }
    }

    void Inode::clearIndirectLink(int32_t index) {
        for (int &indirectLink : m_indirectLinks) {
            if (indirectLink == index) {
                indirectLink = fs::EMPTY_LINK;
            }
        }
    }

    DataLinks::DataLinks(const std::vector<int32_t> &dataClusterIndexes) {
        init();
        for (int i = 0; i < m_directLinks.size(); ++i) {
            if (i >= dataClusterIndexes.size()) {
                return;
            }

            m_directLinks[i] = dataClusterIndexes[i];
        }

        for (int i = 0; i < m_indirectLinks.size(); ++i) {
            size_t clusterIndex = (i * fs::Inode::LINKS_IN_INDIRECT) + i + m_directLinks.size();
            if (clusterIndex >= dataClusterIndexes.size()) {
                return;
            }

            m_indirectLinks[i] = dataClusterIndexes[clusterIndex];
        }
    }
}