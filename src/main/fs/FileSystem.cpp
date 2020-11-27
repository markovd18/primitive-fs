//
// Author: markovd@students.zcu.cz
//

#include <fstream>
#include <filesystem>
#include "FileSystem.h"

bool FileSystem::initialize(fs::Superblock &sb) {

    std::ofstream dataFile(m_dataFileName, std::ios::binary);
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
    fs::Inode rootInode(0, true, 2 * sizeof(fs::DirectoryItem));
    rootInode.addDirectLink(0); /// We add direct link to the first data block, where the root folder data will be
    dataFile.write((char*)&rootInode, sizeof(fs::Inode));
    /// We fill the empty i-node space
    for (int i = 0; i < sb.getInodeCount() - 1; ++i) {
        dataFile.write("\0", sizeof(fs::Inode));
    }

    /**
     * Creating content of the root directory:
     * +root
     * -.
     * -..
     *
     */

    fs::DirectoryItem root("/", 0);
    dataFile.write((char*)&root, sizeof(fs::DirectoryItem));
    fs::DirectoryItem rootSelf(".", 0);
    dataFile.write((char*)&rootSelf, sizeof(fs::DirectoryItem));
    fs::DirectoryItem rootParent("..", 0);
    dataFile.write((char*)&rootParent, sizeof(fs::DirectoryItem));
    /// We fill the empty data space
    for (int i = std::filesystem::file_size(std::filesystem::path(m_dataFileName)); i < sb.getDiskSize(); ++i) {
        dataFile.write("\0", sizeof(char));
    }

    /// In the end we are successfully initialized
    m_initialized = true;
    return true;
}

bool FileSystem::initializeFromExisting() {
    std::ifstream dataFile(m_dataFileName, std::ios::binary);
    if (!dataFile) {
        return false;
    }

    dataFile.seekg(std::ios_base::beg);
    dataFile.read((char*)&m_superblock, sizeof(fs::Superblock));

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
    dataFile.seekg(m_superblock.getInodeStartAddress(), std::ios_base::beg);
    dataFile.read((char*)&m_currentDirInode, sizeof(fs::Inode));
    std::cout << "Initialized from existing file!\n";
    /// In the end we are successfully initialized
    m_initialized = true;
    return true;
}

int32_t FileSystem::getInodeId() const {
    /// We go through the entire bitmap
    for (std::size_t i = 0; i < m_inodeBitmap.getLength(); ++i) {
        for (std::size_t j = 7; j >= 0; --j) {
            if (!((m_inodeBitmap.getBitmap()[i] >> j) & 0b1)) {
                /// If the bit is 0, we return it's index as the inode id
                return i + (7 - j);
            }
        }
    }
    /// If no bit is free, we return FREE_INODE_ID
    return fs::FREE_INODE_ID;
}

bool FileSystem::writeSuperblock(std::ofstream& dataFile, fs::Superblock &sb) {
    if (!dataFile.is_open()) {
        return false;
    }
    m_superblock = sb;
     /// Superblock has to be at the start of the file-system.
    dataFile.seekp(std::ios_base::beg);
    dataFile.write((char*)&m_superblock, sizeof(fs::Superblock));

    return !dataFile.bad();
}

bool FileSystem::initializeInodeBitmap(std::ofstream& dataFile) {
    if (!dataFile.is_open()) {
        return false;
    }
    m_inodeBitmap = std::move(fs::Bitmap(m_superblock.getInodeCount()));
    /// We are initializing new file system, so there will be only one inode - for the root directory
    m_inodeBitmap.getBitmap()[0] = 0b10000000;
    return writeBitmap(dataFile, m_inodeBitmap, m_superblock.getInodeBitmapStartAddress());
}

bool FileSystem::initializeInodeBitmap(std::ifstream& dataFile) {
    if (!dataFile.is_open()) {
        return false;
    }

    m_inodeBitmap = std::move(fs::Bitmap(m_superblock.getInodeCount()));
    return readBitmap(dataFile, m_inodeBitmap, m_superblock.getInodeBitmapStartAddress());
}

bool FileSystem::initializeDataBitmap(std::ofstream& dataFile) {
    if (!dataFile.is_open()) {
        return false;
    }

    m_dataBitmap = std::move(fs::Bitmap(m_superblock.getInodeStartAddress() - m_superblock.getDataBitmapStartAddress()));
    //TODO markovd initialization of data blocks for root only
    m_dataBitmap.getBitmap()[0] = 0b11100000;
    return writeBitmap(dataFile, m_dataBitmap, m_superblock.getDataBitmapStartAddress());
}

bool FileSystem::initializeDataBitmap(std::ifstream& dataFile) {
    if (!dataFile.is_open()) {
        return false;
    }

    m_dataBitmap = std::move(fs::Bitmap(m_superblock.getInodeStartAddress() - m_superblock.getDataBitmapStartAddress()));
    return readBitmap(dataFile, m_dataBitmap, m_superblock.getDataBitmapStartAddress());
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

bool FileSystem::readBitmap(std::ifstream &dataFile, fs::Bitmap &bitmap, int offset) {
    /// If the input stream is closed, we cannot read anything, returning fail state
    if (!dataFile.is_open()) {
        return false;
    }

    dataFile.seekg(offset, std::ios_base::beg);
    dataFile.read((char*)bitmap.getBitmap(), bitmap.getLength());

    return !dataFile.bad();
}

void FileSystem::createFile(const std::string &path, const std::string &data) {

    std::string currentDir = m_currentDirPath;

    //TODO create cd function
    //changeDirectory(path);


}
