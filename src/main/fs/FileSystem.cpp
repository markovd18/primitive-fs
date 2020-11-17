//
// Author: markovd@students.zcu.cz
//

#include <fstream>
#include "FileSystem.h"

bool FileSystem::initialize(const fs::Superblock &superblock) const {

    std::ofstream dataFile(dataFileName, std::ios::binary);
    if (!dataFile) {
        return false;
    }

    dataFile.write((char*)&superblock, sizeof(fs::Superblock));
    if (dataFile.bad()) {
        std::cout << "Error while writing superblock!\n";
        return false;
    }

    fs::Bitmap inodeBitmap(superblock.getInodeCount());
    inodeBitmap.getBitmap()[0] = 0b10000000;
    dataFile.write((char*)inodeBitmap.getBitmap(), inodeBitmap.getLength());
    if (dataFile.bad()) {
        std::cout << "Error while writing i-node bitmap!\n";
        return false;
    }

    fs::Bitmap dataBitmap(superblock.getInodeStartAddress() - superblock.getDataBitmapStartAddress());
    dataBitmap.getBitmap()[0] = 0b11100000;
    dataFile.write((char*)dataBitmap.getBitmap(), dataBitmap.getLength());
    if (dataFile.bad()) {
        std::cout << "Error while writing data-block bitmap!\n";
        return false;
    }
    fs::Inode rootInode(0, false, 20);
    //TODO add direct links
    dataFile.write((char*)&rootInode, sizeof(fs::Inode));
    for (int i = 0; i < superblock.getInodeCount() - 1; ++i) {
        dataFile.write("\0", sizeof(fs::Inode));
    }

    fs::DirectoryItem root("/", 0);
    dataFile.write((char*)&root, sizeof(fs::DirectoryItem));
    fs::DirectoryItem rootSelf(".", 0);
    dataFile.write((char*)&rootSelf, sizeof(fs::DirectoryItem));
    fs::DirectoryItem rootParent("..", 0);
    dataFile.write((char*)&rootParent, sizeof(fs::DirectoryItem));
    for (int i = 0; i < superblock.getClusterCount() - 3; ++i) {
        dataFile.write("\0", sizeof(fs::DirectoryItem));
    }
    /**
     * For now just pile of code that has to be refactored.
     */
    //TODO reformat into smaller functions
    //TODO format 600 only 34MB file??
    return true;
}
