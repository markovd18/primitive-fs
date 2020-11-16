//
// Author: markovd@students.zcu.cz
//

#include "structures.h"

namespace fs {

    Superblock::Superblock(const size_t newDiskSize) {
        strncpy(signature.data(), AUTHOR_NAME, SIGNATURE_LENGTH);
        strncpy(volumeDescription.data(), VOLUME_DESCRIPTION, VOLUME_DESC_LENGTH);
        /**
         * Converting passed size in megabytes to bytes.
         */
        diskSize = newDiskSize * 1000000;
        /**
         * Setting the i-node count to 1/1000 of the disk size
         */
        inodeCount = diskSize / 1000;
        inodeBitmapStartAddress = sizeof(Superblock);
        dataBitmapStartAddress = inodeBitmapStartAddress + inodeCount;

        size_t inodeStorageSize = inodeCount * sizeof(struct inode);
        /**
         * Size of storage for data-block bitmap and blocks themselves.
         */
        size_t dataMapAndStorageSize = diskSize - dataBitmapStartAddress - inodeStorageSize;
        /**
         * Maximum cluster count if we didn't store data-block bitmap.
         */
        size_t clusterCountNoMap = dataMapAndStorageSize / CLUSTER_SIZE;

        size_t clustersToRemove = 0;
        while ((clustersToRemove * CLUSTER_SIZE) < clusterCountNoMap) {
            clustersToRemove++;
        }

        clusterCount = clusterCountNoMap - clustersToRemove;

        inodeStartAddress = dataBitmapStartAddress + clusterCount;
        dataStartAddress = inodeStartAddress + inodeStorageSize;
    }

    const std::array<char, Superblock::SIGNATURE_LENGTH> &Superblock::getSignature() const {
        return signature;
    }

    const std::array<char, Superblock::VOLUME_DESC_LENGTH> &Superblock::getVolumeDescription() const {
        return volumeDescription;
    }

    int32_t Superblock::getDiskSize() const {
        return diskSize;
    }

    int32_t Superblock::getClusterCount() const {
        return clusterCount;
    }

    int32_t Superblock::getInodeBitmapStartAddress() const {
        return inodeBitmapStartAddress;
    }

    int32_t Superblock::getDataBitmapStartAddress() const {
        return dataBitmapStartAddress;
    }

    int32_t Superblock::getInodeStartAddress() const {
        return inodeStartAddress;
    }

    int32_t Superblock::getDataStartAddress() const {
        return dataStartAddress;
    }

    int32_t Superblock::getInodeCount() const {
        return inodeCount;
    }


    DirectoryItem::DirectoryItem(const std::string &itemName, const int32_t inodeId) {
        if (itemName.length() > 12) {
            strncpy(this->itemName.data(), itemName.c_str(), DIR_ITEM_NAME_LENGTH);
        } else {
            strncpy(this->itemName.data(), itemName.c_str(), itemName.length());
        }

        this->inodeId = inodeId;
    }

    int32_t DirectoryItem::getInodeId() const {
        return inodeId;
    }

    const std::array<char, DirectoryItem::DIR_ITEM_NAME_LENGTH> &DirectoryItem::getItemName() const {
        return itemName;
    }
}