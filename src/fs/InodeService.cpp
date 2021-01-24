//
// Created by markovda on 24.01.21.
//

#include "InodeService.h"
#include <utility>

pfs::InodeService::InodeService(std::string mDataFileName, fs::Bitmap inodeBitmap,
                                int32_t inodeBitmapAddress, int32_t inodeStartAddress)
                                : m_dataFileName(std::move(mDataFileName)), m_inodeBitmap(std::move(inodeBitmap)),
                                m_inodeBitmapAddress(inodeBitmapAddress), m_inodeStartAddress(inodeStartAddress) {
    std::fstream dataFile(m_dataFileName, std::ios::out | std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios::failure("Chyba při otevítání datového souboru!");
    }

    m_inodeBitmap.save(dataFile, m_inodeBitmapAddress);
}

int32_t pfs::InodeService::getInodeId() const {
    return m_inodeBitmap.findFirstFreeIndex();
}

fs::Inode pfs::InodeService::createInode(const bool isDirectory, const int32_t fileSize) const {
    return fs::Inode(getInodeId(), isDirectory, fileSize);
}

void pfs::InodeService::saveInode(const fs::Inode &inode) {
    if (inode.getInodeId() == fs::FREE_INODE_ID) {
        throw std::invalid_argument("Nelze uložit i-uzel bez unikátního ID");
    }

    std::fstream dataFile(m_dataFileName, std::ios::out | std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru");
    }

    inode.save(dataFile, m_inodeStartAddress + (inode.getInodeId() * sizeof(inode)));

    /// Updating the bitmap
    m_inodeBitmap.setIndexFilled(inode.getInodeId());
    m_inodeBitmap.save(dataFile, m_inodeBitmapAddress);
}

fs::Inode pfs::InodeService::findInode(const int inodeId) const {
    std::ifstream dataFile(m_dataFileName, std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při čtení ze souboru");
    }

    fs::Inode inode;
    inode.load(dataFile, m_inodeStartAddress + (inodeId * sizeof(fs::Inode)));
    if (inode.getInodeId() == inodeId) {
        return inode;
    }

    /// If we got here, the inode was not found
    throw pfs::ObjectNotFound("I-uzel nenalezen");
}

void pfs::InodeService::removeInode(const fs::Inode &inode) {
    std::fstream dataFile(m_dataFileName, std::ios::out | std::ios::in | std::ios::binary);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru!");
    }

    std::array<int, fs::Superblock::CLUSTER_SIZE> buffer { 0 };

    dataFile.seekp(m_inodeStartAddress + (inode.getInodeId() * sizeof(inode)));
    dataFile.write((char*)buffer.data(), sizeof(inode));
    dataFile.flush();

    m_inodeBitmap.setIndexFree(inode.getInodeId());
    m_inodeBitmap.save(dataFile, m_inodeBitmapAddress);
}

void pfs::InodeService::getRootInode(fs::Inode &rootInode) const {
    std::ifstream dataFile(m_dataFileName, std::ios::in | std::ios::binary);
    if (!dataFile) {
        return;
    }

    rootInode.load(dataFile, m_inodeStartAddress);
}

