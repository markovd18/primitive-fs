//
// Author: markovd@students.zcu.cz
//

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
        m_dataBitmapStartAddress = m_inodeBitmapStartAddress + m_inodeCount;

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

        m_inodeStartAddress = m_dataBitmapStartAddress + m_clusterCount;
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


    Inode::Inode(int32_t nodeId, bool isDirectory, int32_t fileSize) : m_nodeId(nodeId), m_isDirectory(isDirectory),
                                                                       m_fileSize(fileSize), m_references(1), m_directLinks(), m_indirectLinks() {
        m_directLinks.fill(EMPTY_LINK);
        m_indirectLinks.fill(EMPTY_LINK);
    }

    int32_t Inode::getNodeId() const {
        return m_nodeId;
    }

    void Inode::setNodeId(int32_t nodeId) {
        m_nodeId = nodeId;
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

    bool Inode::addDirectLink(int32_t index) {
        for (int & directLink : m_directLinks) {
            if (directLink == fs::EMPTY_LINK) {
                directLink = index;
                return true;
            }
        }
        return false;
    }

    bool Inode::addIndirectLink(int32_t address) {
        //TODO
        return false;
    }
}