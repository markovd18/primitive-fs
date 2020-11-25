//
// Author: markovd@students.zcu.cz
//

#include <fstream>
#include <filesystem>
#include "FileSystem.h"

bool FileSystem::initialize(fs::Superblock &sb) {

    std::ofstream dataFile(dataFileName, std::ios::binary);
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
    if (!writeInodeBitmap(dataFile)) {
        std::cout << "Error while writing i-node bitmap!\n";
        return false;
    }

    if (!writeDataBitmap(dataFile)) {
        std::cout << "Error while writing data-block bitmap!\n";
        return false;
    }
    /**
     * After initialization, there will be only root i-node
     */
    fs::Inode rootInode(0, true, 2 * sizeof(fs::DirectoryItem));
    rootInode.addDirectLink(0); /// We add direct link to the first data block, where the root folder data will be
    dataFile.write((char*)&rootInode, sizeof(fs::Inode));
    for (int i = 0; i < sb.getInodeCount() - 1; ++i) {
        dataFile.write("\0", sizeof(fs::Inode));
    }

    fs::DirectoryItem root("/", 0);
    dataFile.write((char*)&root, sizeof(fs::DirectoryItem));
    fs::DirectoryItem rootSelf(".", 0);
    dataFile.write((char*)&rootSelf, sizeof(fs::DirectoryItem));
    fs::DirectoryItem rootParent("..", 0);
    dataFile.write((char*)&rootParent, sizeof(fs::DirectoryItem));
    for (int i = std::filesystem::file_size(std::filesystem::path(dataFileName)); i < sb.getDiskSize(); ++i) {
        dataFile.write("\0", sizeof(char));
    }

    return true;
}

bool FileSystem::writeSuperblock(std::ofstream& dataFile, fs::Superblock &sb) {
    if (!dataFile.is_open()) {
        return false;
    }
    setSuperblock(&sb);
     /// Superblock has to be at the start of the file-system.
    dataFile.seekp(std::ios_base::beg);
    dataFile.write((char*)&superblock, sizeof(fs::Superblock));

    return !dataFile.bad();
}

bool FileSystem::writeInodeBitmap(std::ofstream& dataFile) {
    fs::Bitmap inodeBitmap(superblock->getInodeCount());
    inodeBitmap.getBitmap()[0] = 0b10000000;
    return writeBitmap(dataFile, inodeBitmap, superblock->getInodeBitmapStartAddress());
}

bool FileSystem::writeDataBitmap(std::ofstream& dataFile) {
    fs::Bitmap dataBitmap(superblock->getInodeStartAddress() - superblock->getDataBitmapStartAddress());
    dataBitmap.getBitmap()[0] = 0b11100000;
    return writeBitmap(dataFile, dataBitmap, superblock->getDataBitmapStartAddress());
}

bool FileSystem::writeBitmap(std::ofstream& dataFile, const fs::Bitmap& bitmap, int offset) {
    /// If the output stream is closed, we cannot write anything, returning fail state
    if (!dataFile.is_open()) {
        return false;
    }
    /// If the bitmap had zero length, we will not write anything, returning success
    if (bitmap.getLength() == 0) {
        return true;
    }

    /// We set the position relative to beginning of the file and write
    dataFile.seekp(offset, std::ios_base::beg);
    dataFile.write((char*)bitmap.getBitmap(), bitmap.getLength());

    return !dataFile.bad();
}
