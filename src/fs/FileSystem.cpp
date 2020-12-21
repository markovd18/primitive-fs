//
// Author: markovd@students.zcu.cz
//

#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include "FileSystem.h"
#include "../utils/FilePathUtils.h"

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
    dataFile.seekp(m_superblock.getDataStartAddress(), std::ios_base::beg);
    fs::DirectoryItem rootSelf(pfs::path::SELF, 0);
    dataFile.write((char*)&rootSelf, sizeof(fs::DirectoryItem));
    fs::DirectoryItem rootParent(pfs::path::PARENT, 0);
    dataFile.write((char*)&rootParent, sizeof(fs::DirectoryItem));
    /// We fill the empty data space
    for (int i = std::filesystem::file_size(std::filesystem::path(m_dataFileName)); i < sb.getDiskSize(); ++i) {
        dataFile.write("\0", sizeof(char));
    }

    /// In the end we are successfully initialized
    m_currentDirPath = "/";
    m_currentDirInode = rootInode;
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
    return m_inodeBitmap.findFirstFreeIndex();
}

fs::Inode FileSystem::createInode(bool isDirectory, int32_t fileSize) const {
    return fs::Inode(getInodeId(), isDirectory, fileSize);
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
    m_dataBitmap.getBitmap()[0] = 0b10000000;
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

void FileSystem::createFile(const std::filesystem::path &path, const fs::FileData &fileData) {
    /// We store current working directory, so we can return back in the end
    const std::string currentDir = m_currentDirPath;
    const fs::Inode currentInode = m_currentDirInode;

    /// By changing to given path we not only get access to the i-node we need, but also validate it's existence
    changeDirectory(path);

    fs::Inode inode = createInode(false, fileData.size());

    fs::ClusteredFileData clusteredData(fileData);
    std::vector<std::size_t> dataClusterIndexes = getFreeDataBlocks(clusteredData.requiredDataBlocks());
    inode.setData(dataClusterIndexes);

    saveInode(inode);
    //saveFileData(dataClusters, dataClusterIndexes);

    /// Returning back to the original directory
    m_currentDirInode = currentInode;
    m_currentDirPath = currentDir;
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
    std::ifstream dataFile(m_dataFileName);
    if (!dataFile) {
        return;
    }

    fs::Inode tmp = rootInode;
    dataFile.seekg(m_superblock.getInodeStartAddress(), std::ios_base::beg);
    dataFile.read((char*)&rootInode, sizeof(fs::Inode));
    if (dataFile.bad()) {
        rootInode = tmp;
    }
}

std::vector<fs::DirectoryItem> FileSystem::getDirectoryItems(const fs::Inode& directory) {

    std::vector<fs::DirectoryItem> directoryItems;
    readDirItemsDirect(directory, directoryItems);
    readDirItemsIndirect(directory, directoryItems);

    return directoryItems;
}


void FileSystem::readDirItemsDirect(const fs::Inode &directory, std::vector<fs::DirectoryItem> &directoryItems) {
    if (!directory.isDirectory()) {
        throw std::invalid_argument("Předaný inode nereprezentuje adresář");
    }

    std::vector<int32_t> links;
    for (const auto &link : directory.getDirectLinks()) {
        if (link != fs::EMPTY_LINK) {
            links.push_back(link);
        }
    }

    readDirItems(links, directoryItems);
}

void FileSystem::readDirItemsIndirect(const fs::Inode &directory, std::vector<fs::DirectoryItem> &directoryItems) {
    if (!directory.isDirectory()) {
        throw std::invalid_argument("Předaný inode nereprezentuje adresář");
    }

    std::ifstream dataFile(m_dataFileName);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při čtení dat");
    }

    std::vector<int32_t> links;
    for (const auto &indirectLink : directory.getIndirectLinks()) {
        if (indirectLink == fs::EMPTY_LINK) {
            continue;
        }

        for (int i = 0; i < fs::Superblock::CLUSTER_SIZE; i += sizeof(int32_t)) {
            int32_t directLink;
            dataFile.seekg(m_superblock.getDataStartAddress() + indirectLink, std::ios_base::beg);
            dataFile.read((char*)&directLink, sizeof(int32_t));
            if (directLink != fs::EMPTY_LINK) {
                links.push_back(directLink);
            }
        }
    }

    dataFile.close();
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při uzavření datového souboru");
    }

    readDirItems(links, directoryItems);
}

void FileSystem::readDirItems(const std::vector<int32_t> &indexList, std::vector<fs::DirectoryItem> &directoryItems) {
    std::ifstream dataFile(m_dataFileName);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při čtení ze souboru");
    }

    for (const auto& index : indexList) {
        for (int i = 0; i < fs::Superblock::CLUSTER_SIZE; i += sizeof(fs::DirectoryItem)) {
            dataFile.seekg(m_superblock.getDataStartAddress() + i,
                           std::ios_base::beg);

            fs::DirectoryItem dirItem;
            dataFile.read((char *) &dirItem, sizeof(fs::DirectoryItem));
            if (dirItem.getItemName().at(0) != 0) {
                /// Ending character at the beginning of the file name would mean, that we read "empty" memory
                directoryItems.push_back(dirItem);
            }
        }
    }
}

fs::Inode FileSystem::findInode(const int inodeId) {
    std::ifstream dataFile(m_dataFileName);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při čtení ze souboru");
    }

    fs::Inode inode;
    for (int i = 0; i < m_inodeBitmap.getLength(); ++i) {
        for (int j = 7; j >= 0; --j) {
            /// If the bit is 1, there is inode on this index
            if ((m_inodeBitmap.getBitmap()[i] >> j) & 0b1) {
                dataFile.seekg(m_superblock.getInodeStartAddress() + (i * (7 - j) * sizeof(fs::Inode)),
                               std::ios_base::beg);

                dataFile.read((char*) &inode, sizeof(fs::Inode));
                if (inode.getInodeId() == inodeId) {
                    return inode;
                }
            }
        }
    }

    /// If we got here, the inode was not found
    throw pfs::ObjectNotFound("I-uzel nenalezen");
}

void FileSystem::saveInode(const fs::Inode &inode) {
    if (inode.getInodeId() == fs::FREE_INODE_ID) {
        throw std::invalid_argument("Nelze uložit i-uzel bez unikátního ID");
    }

    std::ofstream dataFile(m_dataFileName);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru");
    }

    /// Unique ID of each i-node also represents it's index in the i-node storage, so we don't need to iterate through
    /// the bitmap again
    dataFile.seekp(inode.getInodeId() * sizeof(fs::Inode), std::ios_base::beg);
    dataFile.write((char*)&inode, sizeof(fs::Inode));

    /// If we got here, there is no more free space for inodes
    throw std::ios_base::failure("Nelze zapsat více i-uzlů");
}

std::vector<std::size_t> FileSystem::getFreeDataBlocks(const std::size_t count) {
    return m_dataBitmap.findFreeIndexes(count);
}

void FileSystem::saveFileData(const std::vector<std::string>& dataClusters, const std::vector<size_t>& dataClusterIndexes) {
    std::ofstream dataFile(m_dataFileName);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru");
    }

    std::size_t cluster = 0;
    for (int i = 0; i < fs::Inode::DIRECT_LINKS_COUNT; ++i) {
        if (i >= dataClusterIndexes.size()) {
            return;
        }

        m_dataBitmap.setIndexFilled(dataClusterIndexes.at(i));
        dataFile.seekp(m_superblock.getDataStartAddress() + dataClusterIndexes.at(i), std::ios_base::beg);
        dataFile.write(dataClusters.at(cluster).data(), dataClusters.at(cluster).length());
        cluster++;
    }

    for (int i = 0; i < fs::Inode::INDIRECT_LINKS_COUNT; ++i) {
        if ((i + fs::Inode::DIRECT_LINKS_COUNT) >= dataClusterIndexes.size()) {
            return;
        }

        m_dataBitmap.setIndexFilled(dataClusterIndexes.at(i + fs::Inode::DIRECT_LINKS_COUNT));
        //TODO markovd save indirect data
    }

    /// In the end we need to save the changes we made to the bitmap
    writeBitmap(dataFile, m_dataBitmap, m_superblock.getDataBitmapStartAddress());
}

void FileSystem::updateInodeBitmap() {
    std::ofstream dataFile(m_dataFileName);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru");
    }

    writeBitmap(dataFile, m_inodeBitmap, m_superblock.getInodeBitmapStartAddress());
}

void FileSystem::updateDataBitmap() {
    std::ofstream dataFile(m_dataFileName);
    if (!dataFile) {
        throw std::ios_base::failure("Chyba při otevírání datového souboru");
    }

    writeBitmap(dataFile, m_dataBitmap, m_superblock.getDataBitmapStartAddress());
}
