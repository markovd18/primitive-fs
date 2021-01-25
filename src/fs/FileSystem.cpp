//
// Author: markovd@students.zcu.cz
//

#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include "FileSystem.h"
#include "../utils/FilePathUtils.h"
#include "../utils/InvalidState.h"
#include "../command/returnval.h"

bool FileSystem::initialize(fs::Superblock &sb) {

    std::fstream dataFile(m_dataFileName, std::ios::out | std::ios::binary);
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
    fs::Inode rootInode(0, true, 0);
    rootInode.addDirectLink(0); /// We add direct link to the first data block, where the root folder data will be

    rootInode.save(dataFile, m_superblock.getInodeStartAddress());
    /// We fill the empty i-node space
    for (size_t i = (m_superblock.getInodeStartAddress() + sizeof(fs::Inode)); i < m_superblock.getDataStartAddress(); ++i) {
        dataFile.put('\0');
    }

    /**
     * Creating content of the root directory:
     * +root
     * +.
     * +..
     *
     */
    fs::DirectoryItem rootSelf(pfs::path::SELF, 0);
    rootSelf.save(dataFile, m_superblock.getDataStartAddress());
    fs::DirectoryItem rootParent(pfs::path::PARENT, 0);
    rootParent.save(dataFile, dataFile.tellp());

    /// We fill the empty data space
    for (size_t i = (m_superblock.getDataStartAddress() + 2 * sizeof(fs::DirectoryItem)); i < sb.getDiskSize(); ++i) {
        dataFile.put('\0');
    }

    dataFile.flush();
    /// In the end we are successfully initialized
    m_currentDirPath = "/";
    m_currentDirInode = rootInode;
    m_initialized = true;
    return true;
}

bool FileSystem::initializeFromExisting() {
    std::ifstream dataFile(m_dataFileName, std::ios::in | std::ios::binary);
    if (!dataFile) {
        return false;
    }

    m_superblock.load(dataFile, 0);
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
    m_currentDirInode.load(dataFile, m_superblock.getInodeStartAddress());
    std::cout << "Initialized from existing file!\n";
    /// In the end we are successfully initialized
    m_initialized = true;
    return true;
}

bool FileSystem::writeSuperblock(std::fstream& dataFile, fs::Superblock &sb) {
    if (!dataFile.is_open()) {
        return false;
    }
    m_superblock = sb;
     /// Superblock has to be at the start of the file-system.
    m_superblock.save(dataFile, 0);

    return !dataFile.bad();
}

bool FileSystem::initializeInodeBitmap(std::fstream& dataFile) {
    if (!dataFile.is_open()) {
        return false;
    }

    fs::Bitmap inodeBitmap(m_superblock.getDataBitmapStartAddress() - m_superblock.getInodeBitmapStartAddress());
    inodeBitmap.setIndexFilled(0);
    inodeBitmap.save(dataFile, m_superblock.getInodeBitmapStartAddress());
    m_inodeService = pfs::InodeService(m_dataFileName, inodeBitmap,
                                       m_superblock.getInodeBitmapStartAddress(), m_superblock.getInodeStartAddress());
    return !dataFile.bad();
}

bool FileSystem::initializeInodeBitmap(std::ifstream& dataFile) {
    if (!dataFile.is_open()) {
        return false;
    }

    fs::Bitmap inodeBitmap(m_superblock.getDataBitmapStartAddress() - m_superblock.getInodeBitmapStartAddress());
    inodeBitmap.load(dataFile, m_superblock.getInodeBitmapStartAddress());
    m_inodeService = pfs::InodeService(m_dataFileName, inodeBitmap,
                                       m_superblock.getInodeBitmapStartAddress(), m_superblock.getInodeStartAddress());
    return !dataFile.bad();
}

bool FileSystem::initializeDataBitmap(std::fstream& dataFile) {
    if (!dataFile.is_open()) {
        return false;
    }

    fs::Bitmap dataBitmap(m_superblock.getInodeStartAddress() - m_superblock.getDataBitmapStartAddress());
    dataBitmap.setIndexFilled(0);
    dataBitmap.save(dataFile, m_superblock.getDataBitmapStartAddress());
    m_dataService = pfs::DataService(m_dataFileName, dataBitmap,
                                     m_superblock.getDataBitmapStartAddress(),m_superblock.getDataStartAddress());
    return !dataFile.bad();
}

bool FileSystem::initializeDataBitmap(std::ifstream& dataFile) {
    if (!dataFile.is_open()) {
        return false;
    }

    fs::Bitmap dataBitmap(m_superblock.getInodeStartAddress() - m_superblock.getDataBitmapStartAddress());
    dataBitmap.load(dataFile, m_superblock.getDataBitmapStartAddress());
    m_dataService = pfs::DataService(m_dataFileName, dataBitmap,
                                     m_superblock.getDataBitmapStartAddress(),m_superblock.getDataStartAddress());
    return !dataFile.bad();
}

void FileSystem::createFile(const std::filesystem::path &path, const fs::FileData &fileData) {
    if (!path.has_filename()) {
        throw std::invalid_argument("Předaná cesta nekončí názvem souboru");
    }

    if (path.filename().string().size() > 11) {
        throw std::invalid_argument("Název souboru smí být maximálně 11 znaků dlouhý!");
    }

    std::string pathStr(path.string());
    std::string pathNoFilename(pathStr.substr(0, pathStr.length() - path.filename().string().length()));
    /// We store current working directory, so we can return back in the end
    const std::string currentDir = m_currentDirPath;

    if (m_currentDirPath != pathNoFilename) {
        /// By changing to given path we not only get access to the i-node we need, but also validate it's existence
        try {
            changeDirectory(pathNoFilename);
        } catch (const std::exception &ex) {
            throw std::invalid_argument(fnct::PNF_DEST);
        }
    }

    std::vector<fs::DirectoryItem> dirItems(m_dataService.getDirectoryItems(m_currentDirInode));
    auto it = std::find_if(dirItems.begin(), dirItems.end(), [&path](const fs::DirectoryItem item) { return item.nameEquals(path.filename()); });
    if (it != dirItems.end()) {
        throw pfs::InvalidState("Soubor s předaným názvem již exituje!");
    }

    fs::Inode inode(m_inodeService.createInode(false, fileData.size()));

    fs::ClusteredFileData clusteredData(fileData);
    std::vector<int32_t> dataClusterIndexes(m_dataService.getFreeDataBlocks(clusteredData.requiredDataBlocks()));
    inode.setData(fs::DataLinks(dataClusterIndexes));

    m_inodeService.saveInode(inode);
    m_dataService.saveFileData(clusteredData, dataClusterIndexes);
    /// We cd'd into the parent folder so we can just save into current folder
    m_dataService.saveDirItemIntoDirectory(fs::DirectoryItem(path.filename(), inode.getInodeId()), m_currentDirInode);
    m_currentDirInode.setFileSize(m_currentDirInode.getFileSize() + inode.getFileSize());
    m_inodeService.saveInode(m_currentDirInode);

    while (m_currentDirPath != "/") {
        changeDirectory("../");
        m_currentDirInode.setFileSize(m_currentDirInode.getFileSize() + inode.getFileSize());
        m_inodeService.saveInode(m_currentDirInode);
    }

    changeDirectory(currentDir);

}

void FileSystem::removeFile(const std::filesystem::path &path) {
    if (!path.has_filename()) {
        throw std::invalid_argument("Předaná cesta nekončí názvem souboru");
    }

    std::string pathStr(path.string());
    std::string pathNoFilename = pathStr.substr(0, pathStr.length() - path.filename().string().length());
    /// We store current working directory, so we can return back in the end
    const std::string currentDir = m_currentDirPath;

    if (m_currentDirPath != pathNoFilename) {
        /// By changing to given path we not only get access to the i-node we need, but also validate it's existence
        changeDirectory(pathNoFilename);
    }

    fs::DirectoryItem directoryItem = m_dataService.findDirectoryItem(path.filename(), m_currentDirInode);
    fs::Inode fileInode(m_inodeService.findInode(directoryItem.getInodeId()));
    if (fileInode.isDirectory()) {
        throw std::invalid_argument("Soubor na předané cestě nelze smazat, protože je to složka");
    }

    m_dataService.removeDirectoryItem(path.filename(), m_currentDirInode);
    m_dataService.clearInodeData(fileInode);
    m_inodeService.removeInode(fileInode);

    m_currentDirInode.setFileSize(m_currentDirInode.getFileSize() - fileInode.getFileSize());
    m_inodeService.saveInode(m_currentDirInode);

    while (m_currentDirPath != "/") {
        changeDirectory("../");
        m_currentDirInode.setFileSize(m_currentDirInode.getFileSize() - fileInode.getFileSize());
        m_inodeService.saveInode(m_currentDirInode);
    }

    changeDirectory(currentDir);
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
        m_inodeService.getRootInode(referenceFolder);
    } else {
        referenceFolder = m_currentDirInode;
    }


    std::vector<fs::DirectoryItem> directoryItems;
    /// Now we can iterate through every passed file name
    for (const auto &name : tokens) {
        if (name.empty()) {
            continue;
        }

        directoryItems = m_dataService.getDirectoryItems(referenceFolder);
        auto it = std::find_if(directoryItems.begin(), directoryItems.end(), [&name](const fs::DirectoryItem item) { return item.nameEquals(name); });
        if (it == directoryItems.end()) {
            throw std::invalid_argument("Předaná cesta neexistuje");
        }

        fs::Inode dirItemInode = m_inodeService.findInode(it.base()->getInodeId());
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

std::vector<fs::DirectoryItem> FileSystem::getDirectoryItems(const std::filesystem::path &dirPath) {
    const fs::Inode currentDirInode = m_currentDirInode;
    const std::string currentDirPath = m_currentDirPath;

    if (m_currentDirPath != dirPath) {
        changeDirectory(dirPath);
    }

    std::vector<fs::DirectoryItem> dirItems = m_dataService.getDirectoryItems(m_currentDirInode);

    m_currentDirInode = currentDirInode;
    m_currentDirPath = currentDirPath;

    return dirItems;
}

fs::Inode FileSystem::findInode(const int inodeId) {
    return m_inodeService.findInode(inodeId);
}


void FileSystem::printFileContent(const std::filesystem::path &pathToFile) {
    if (!pathToFile.has_filename()) {
        throw std::invalid_argument("Předaná cesta nemá název souboru!");
    }

    std::string pathStr(pathToFile.string());
    std::string pathNoFilename(pathStr.substr(0, pathStr.length() - pathToFile.filename().string().length()));
    /// We store current working directory, so we can return back in the end
    const std::string currentDir = m_currentDirPath;
    const fs::Inode currentInode = m_currentDirInode;

    if (m_currentDirPath != pathNoFilename) {
        /// By changing to given path we not only get access to the i-node we need, but also validate it's existence
        changeDirectory(pathNoFilename);
    }

    fs::DirectoryItem dirItem = m_dataService.findDirectoryItem(pathToFile.filename(), m_currentDirInode);
    fs::Inode inode = m_inodeService.findInode(dirItem.getInodeId());
    if (inode.isDirectory()) {
        throw std::invalid_argument("Obsah adresáře nelze vypsat! Použijte funkci \"ls\"!");
    }

    std::string fileContent = m_dataService.getFileContent(inode);
    std::cout << fileContent << '\n';

    if (currentDir != pathNoFilename) {
        m_currentDirPath = currentDir;
        m_currentDirInode = currentInode;
    }
}

std::string FileSystem::getFileContent(const std::filesystem::path &pathToFile) {
    if (!pathToFile.has_filename()) {
        throw std::invalid_argument("Předaná cesta nemá název souboru!");
    }

    std::string pathStr(pathToFile.string());
    std::string pathNoFilename(pathStr.substr(0, pathStr.length() - pathToFile.filename().string().length()));
    /// We store current working directory, so we can return back in the end
    const std::string currentDir = m_currentDirPath;
    const fs::Inode currentInode = m_currentDirInode;

    if (m_currentDirPath != pathNoFilename) {
        /// By changing to given path we not only get access to the i-node we need, but also validate it's existence
        changeDirectory(pathNoFilename);
    }
    fs::DirectoryItem dirItem = m_dataService.findDirectoryItem(pathToFile.filename(), m_currentDirInode);
    fs::Inode inode = m_inodeService.findInode(dirItem.getInodeId());
    if (inode.isDirectory()) {
        throw pfs::InvalidState("Obsah adresáře nelze vrátit! Použijte funkci \"ls\"!");
    }

    std::string fileContent = m_dataService.getFileContent(inode);

    if (currentDir != pathNoFilename) {
        m_currentDirPath = currentDir;
        m_currentDirInode = currentInode;
    }

    return fileContent;
}

void FileSystem::printFileInfo(const std::filesystem::path &pathToFile) {
    std::filesystem::path parentPath(pathToFile.parent_path());

    const std::string currentDir = m_currentDirPath;
    const fs::Inode currentInode = m_currentDirInode;

    if (m_currentDirPath != parentPath && !parentPath.empty()) {
        /// By changing to given path we not only get access to the i-node we need, but also validate it's existence
        changeDirectory(parentPath);
    }

    std::string filename;
    if (pathToFile.has_filename()) {
        filename = pathToFile.filename();
    } else {
        filename = std::filesystem::path(m_currentDirPath).filename();
    }

    fs::DirectoryItem dirItem = m_dataService.findDirectoryItem(filename, m_currentDirInode);
    fs::Inode inode = m_inodeService.findInode(dirItem.getInodeId());

    std::cout << "Name: " << dirItem.getItemName().data() << " - Size: " << inode.getFileSize() << " - Inode ID: " << inode.getInodeId() << " - ";
    std::cout << "Direct links: ";
    for (const auto &link : inode.getDirectLinks()) {
        if (link == fs::EMPTY_LINK) {
            break;
        }

        std::cout << link << " ";
    }
    std::cout << "Indirect links: ";
    for (const auto &link: inode.getIndirectLinks()) {
        if (link == fs::EMPTY_LINK) {
            break;
        }

        std::cout << link << " ";
    }
    std::cout << std::endl;

    if (currentDir != parentPath && !parentPath.empty()) {
        m_currentDirPath = currentDir;
        m_currentDirInode = currentInode;
    }
}

void FileSystem::createDirectory(const std::filesystem::path &path) {
    if (path.empty()) {
        throw std::invalid_argument("Path must not be empty!");
    }

    std::filesystem::path directory;
    std::filesystem::path parent;
    if (path.has_filename()) {
        directory = path.filename();
        parent = path.parent_path();
    } else {
        directory = path.parent_path().filename();
        parent = path.parent_path().parent_path();
    }


    const std::string currentDir = m_currentDirPath;
    const fs::Inode currentInode = m_currentDirInode;

    if (m_currentDirPath != parent && !parent.empty()) {
        /// By changing to given path we not only get access to the i-node we need, but also validate it's existence
        try {
            changeDirectory(parent);
        } catch (const std::exception& ex) {
            throw std::invalid_argument(fnct::PNF_DEST);
        }
    }

    std::vector<fs::DirectoryItem> dirItems(m_dataService.getDirectoryItems(m_currentDirInode));
    auto it = std::find_if(dirItems.begin(), dirItems.end(), [&directory](const fs::DirectoryItem item) { return item.nameEquals(directory.filename()); });
    if (it != dirItems.end()) {
        throw pfs::InvalidState(fnct::EXISTS);
    }

    fs::Inode inode(m_inodeService.createInode(true, 0));
    fs::DirectoryItem directoryItem(directory.string(), inode.getInodeId());
    m_dataService.saveDirItemIntoDirectory(directoryItem, m_currentDirInode);
    m_inodeService.saveInode(inode);
    m_inodeService.saveInode(m_currentDirInode);

    fs::DirectoryItem dirItemSelf(".", inode.getInodeId());
    fs::DirectoryItem dirItemParent("..", m_currentDirInode.getInodeId());

    changeDirectory(directory);
    m_dataService.saveDirItemIntoDirectory(dirItemSelf, m_currentDirInode);
    m_dataService.saveDirItemIntoDirectory(dirItemParent, m_currentDirInode);
    m_inodeService.saveInode(m_currentDirInode);

    /// Returning back to the original directory
    changeDirectory(currentDir);
}

void FileSystem::removeDirectory(const std::filesystem::path &path) {
    if (path.empty()) {
        throw std::invalid_argument("Path must not be empty!");
    }

    std::filesystem::path directory;
    std::filesystem::path parent;
    if (path.has_filename()) {
        directory = path.filename();
        parent = path.parent_path();
    } else {
        directory = path.parent_path().filename();
        parent = path.parent_path().parent_path();
    }

    const std::string currentDir = m_currentDirPath;
    const fs::Inode currentInode = m_currentDirInode;

    if (m_currentDirPath != parent && !parent.empty()) {
        /// By changing to given path we not only get access to the i-node we need, but also validate it's existence
        try {
            changeDirectory(parent);
        } catch (const std::exception &ex) {
            throw std::invalid_argument(fnct::FNF_DIR);
        }
    }

    fs::DirectoryItem dirItem = m_dataService.findDirectoryItem(directory, m_currentDirInode);
    fs::Inode inode = m_inodeService.findInode(dirItem.getInodeId());
    if (!inode.isDirectory()) {
        throw std::invalid_argument(fnct::FNF_DIR);
    }

    for (int i = 1; i < inode.getDirectLinks().size(); ++i) {
        if (inode.getDirectLinks()[i] != fs::EMPTY_LINK) {
            throw pfs::InvalidState(fnct::NOT_EMPTY);
        }
    }

    for (const auto& indirectLink : inode.getIndirectLinks()) {
        if (indirectLink != fs::EMPTY_LINK) {
            throw pfs::InvalidState(fnct::NOT_EMPTY);
        }
    }

    if (m_dataService.getFreeDirItemDataBlockSubindex(inode.getDirectLinks()[0]) != 2 * sizeof(dirItem)) {
        throw pfs::InvalidState(fnct::NOT_EMPTY);
    }

    m_dataService.removeDirectoryItem(directory.string(), m_currentDirInode);
    m_dataService.clearInodeData(inode);
    m_inodeService.removeInode(inode);

    if (m_currentDirPath != parent && !parent.empty()) {
        /// Returning back to the original directory
        m_currentDirInode = currentInode;
        m_currentDirPath = currentDir;
    }
}

void FileSystem::copyFile(const std::filesystem::path &pathFrom, const std::filesystem::path &pathTo) {
    if (pathFrom.empty() || pathTo.empty()) {
        throw std::invalid_argument("Paths must not be empty!");
    }

    std::string fileContent;
    try {
        fileContent = getFileContent(pathFrom);
    } catch (const std::exception &ex) {
        throw std::invalid_argument(fnct::FNF_SOURCE);
    }

    try {
        createFile(pathTo, fs::FileData(fileContent));
    } catch (const std::exception &ex) {
        throw std::invalid_argument(fnct::PNF_DEST);
    }
}

void FileSystem::moveFile(const std::filesystem::path &pathFrom, const std::filesystem::path &pathTo) {
    if (pathFrom.empty() || pathTo.empty()) {
        throw std::invalid_argument("Paths must not be empty!");
    }

    copyFile(pathFrom, pathTo);
    removeFile(pathFrom);
}

void FileSystem::checkData() {
    /// File size check
    std::vector<fs::Inode> inodes = m_inodeService.getAllInodes();
    std::string content;
    fs::Inode root;
    m_inodeService.getRootInode(root);
    std::vector<fs::DirectoryItem> directoryItems = m_dataService.getDirectoryItems(root);

    for (const auto &inode : inodes) {
        if (inode.getInodeId() == 0) {
            continue; /// Not checking root
        }
        if (!inode.isDirectory()) {
            content = m_dataService.getFileContent(inode);
            if (content.length() != inode.getFileSize()) {
                std::cout << "Velikost souboru v I-uzlu " << inode.getInodeId() << " (" << inode.getFileSize()
                          << ") neodpovídá velikosti uložených dat (" << content.length() << ")!\n";
            }
        } else {
            std::vector<fs::DirectoryItem> dirItems = m_dataService.getDirectoryItems(inode);
            directoryItems.insert(directoryItems.end(), std::make_move_iterator(dirItems.begin()), std::make_move_iterator(dirItems.end()));
        }
    }

    for (const auto &inode : inodes) {
        if (inode.getInodeId() == 0) {
            continue; /// Not checking root
        }

        bool isInDirectory = false;
        for (auto &dirItem : directoryItems) {
            if (dirItem.getInodeId() == inode.getInodeId() && (!dirItem.nameEquals(".") && !dirItem.nameEquals(".."))) {
                isInDirectory = true;
                break;
            }
        }

        if (!isInDirectory) {
            std::cout << "I-uzel s ID: " << inode.getInodeId() << " se nanachází v žádném adresáři!\n";
        }
    }

    std::cout << "CHECK COMPLETE" << std::endl;
}

void FileSystem::breakData() {
    fs::Inode root = m_inodeService.findInode(0);

    std::vector<fs::DirectoryItem> rootItems = m_dataService.getDirectoryItems(root);
    for (auto &dirItem : rootItems) {
        if (!dirItem.nameEquals(".") && !dirItem.nameEquals("..")) {
            m_dataService.removeDirectoryItem(dirItem.getItemName().data(), root);
            fs::Inode inode = m_inodeService.findInode(dirItem.getInodeId());
            inode.setData(fs::DataLinks(std::vector<int32_t>()));
            m_inodeService.saveInode(inode);
        }
    }
}
