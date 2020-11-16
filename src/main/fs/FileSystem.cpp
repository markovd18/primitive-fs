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
    dataFile.write((char*)dataBitmap.getBitmap(), inodeBitmap.getLength());
    if (dataFile.bad()) {
        std::cout << "Error while writing data-block bitmap!\n";
        return false;
    }


    return true;
}
