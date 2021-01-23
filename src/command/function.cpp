//
// Author: markovd@students.zcu.cz
//

#include <vector>
#include <fstream>

#include "../common/structures.h"
#include "../utils/InputParamsValidator.h"
#include "../utils/StringNumberConverter.h"
#include "../fs/FileSystem.h"
#include "returnval.h"
#include "function.h"

void fnct::format(const std::vector <std::string>& parameters, FileSystem* fileSystem) {
    if (fileSystem == nullptr) {
        std::cout << fnct::CANNOT_CREATE_FILE << '\n';
        return;
    }

    if (!InputParamsValidator::validateFormatFunctionPatameters(parameters)) {
        std::cout << fnct::CANNOT_CREATE_FILE << '\n';
        return;
    }

    const std::string& diskSizeStr = parameters.at(0);
    fs::Superblock superblock(StringNumberConverter::convertStringToInt(diskSizeStr).value);

    if(!fileSystem->initialize(superblock)) {
        std::cout << fnct::CANNOT_CREATE_FILE << '\n';
        return;
    }

    std::cout << fnct::OK << '\n';
}

void fnct::incp(const std::vector<std::string> &parameters, FileSystem* fileSystem) {
    if (fileSystem == nullptr || !fileSystem->isInitialized()) {
        std::cout << "File system is not initialized!\n";
        return;
    }

    if (!InputParamsValidator::validateIncpFunctionParameters(parameters)) {
        std::cout << fnct::FNF_SOURCE << '\n';
        return;
    }

    const auto hddPath = parameters.at(0);

    std::ifstream hddFile(hddPath, std::ios::in | std::ios_base::binary);
    if (!hddFile) {
        std::cout << fnct::FNF_SOURCE << '\n';
        return;
    }

    const auto fileSize = std::filesystem::file_size(hddPath);
    char fileBuffer[fileSize];
    hddFile.read((char*)fileBuffer, fileSize);
    std::string fileData;
    for (const auto &c : fileBuffer) {
        fileData += c;
    }
    /// Adding null termination character, so we can simply work with dynamic strings
    /// Null termination character will be removed when saving into the data file
    try {
        fileSystem->createFile(parameters.at(1), fs::FileData(fileData));
        std::cout << fnct::OK << '\n';
    } catch (const std::exception& ex) {
        std::cout << ex.what() << '\n';
    }

}

void fnct::pwd(const std::vector<std::string> &parameters, FileSystem *fileSystem) {
    if (fileSystem == nullptr || !fileSystem->isInitialized()) {
        std::cout << "File system is not initialized!\n";
        return;
    }

    std::cout << fileSystem->getCurrentDir() << '\n';
}

void fnct::cd(const std::vector<std::string> &parameters, FileSystem *fileSystem) {
    if (fileSystem == nullptr || !fileSystem->isInitialized()) {
        std::cout << "File system is not initialized!\n";
        return;
    }

    ///Here we don't validate anything. If no parameter is passed, we cd to root directory, otherwise we try to cd into given directory
    try {
        if (parameters.empty() || parameters.at(0).empty()) {
            fileSystem->changeDirectory("/");
        } else {
            fileSystem->changeDirectory(parameters.at(0));
        }
        std::cout << fnct::OK << '\n';
    } catch (const std::invalid_argument& ex) {
        std::cout << fnct::PNF_PATH << '\n';
    }
}

void fnct::ls(const std::vector<std::string> &parameters, FileSystem *fileSystem) {
    if (fileSystem == nullptr || !fileSystem->isInitialized()) {
        std::cout << "File system is not initialized!\n";
        return;
    }

    if (parameters.empty() || parameters.at(0).empty()) {
        std::cout << fnct::PNF_DEST << '\n';
        return;
    }

    std::vector<fs::DirectoryItem> dirItems;
    try {
         dirItems = fileSystem->getDirectoryItems(parameters.at(0));
    } catch (const std::exception &ex) {
        std::cout << fnct::PNF_DIR << '\n';
        return;
    }

    for (const auto &dirItem : dirItems) {
        char mark;
        if (fileSystem->findInode(dirItem.getInodeId()).isDirectory()) {
            mark = '+';
        } else {
            mark = '-';
        }
        std::cout << mark << dirItem.getItemName().data() << '\n';
    }
}
void fnct::rm(const std::vector<std::string> &parameters, FileSystem *fileSystem) {
    if (fileSystem == nullptr || !fileSystem->isInitialized()) {
        std::cout << "File system is not initialized!\n";
        return;
    }

    if (parameters.empty() || parameters.at(0).empty()) {
        std::cout << fnct::INVALID_ARG << '\n';
        return;
    }

    try {
        fileSystem->removeFile(parameters.at(0));
    } catch (const std::exception &ex) {
        std::cout << fnct::FILE_NOT_FOUND << '\n';
        return;
    }

    std::cout << fnct::OK << '\n';
}

void fnct::cat(const std::vector<std::string> &parameters, FileSystem *fileSystem) {
    if (fileSystem == nullptr || !fileSystem->isInitialized()) {
        std::cout << "File system is not initialized!\n";
        return;
    }

    if (parameters.empty() || parameters.at(0).empty()) {
        std::cout << fnct::INVALID_ARG << '\n';
        return;
    }

    try {
        fileSystem->printFileContent(parameters.at(0));
    } catch (const std::exception &exception) {
        std::cout << fnct::FNF_SOURCE << '\n';
    }
}

void fnct::outcp(const std::vector<std::string> &parameters, FileSystem *fileSystem) {
    if (fileSystem == nullptr || !fileSystem->isInitialized()) {
        std::cout << "File system is not initialized!\n";
        return;
    }

    if (parameters.empty() || parameters.size() < 2 || parameters.at(0).empty() || parameters.at(1).empty()) {
        std::cout << fnct::INVALID_ARG << '\n';
        return;
    }

    std::filesystem::path hddPath(parameters.at(1));
    if (!hddPath.has_filename()) {
        std::cout << fnct::INVALID_ARG << '\n';
        return;
    }

    if (std::filesystem::exists(hddPath) || std::filesystem::is_directory(hddPath)) {
        std::cout << fnct::INVALID_ARG << '\n';
        return;
    }

    if (hddPath.has_parent_path() && !std::filesystem::exists(hddPath.parent_path())) {
        std::cout << fnct::PNF_DEST<< '\n';
        return;
    }

    std::string fileContent;
    try {
        fileContent = fileSystem->getFileContent(parameters.at(0));
    } catch (const std::invalid_argument &ex) {
        std::cout << fnct::FNF_SOURCE << '\n';
    }

    std::ofstream hddFile(parameters.at(1), std::ios::out | std::ios::binary);
    if (!hddFile) {
        throw std::ios::failure("Error while opening the hard disk file!");
    }

    hddFile.write(fileContent.data(), fileContent.size());
    hddFile.flush();

    std::cout << fnct::OK << '\n';
}

void fnct::info(const std::vector<std::string> &parameters, FileSystem *fileSystem) {
    if (fileSystem == nullptr || !fileSystem->isInitialized()) {
        std::cout << "File system is not initialized!\n";
        return;
    }

    if (parameters.empty() || parameters.at(0).empty()) {
        std::cout << fnct::INVALID_ARG << '\n';
        return;
    }

    try {
        fileSystem->printFileInfo(parameters.at(0));
    } catch (const pfs::ObjectNotFound& ex) {
        std::cout << fnct::FNF_SOURCE << '\n';
    }
}

void fnct::mkdir(const std::vector<std::string> &parameters, FileSystem *fileSystem) {
    if (fileSystem == nullptr || !fileSystem->isInitialized()) {
        std::cout << "File system is not initialized!\n";
        return;
    }

    if (parameters.empty() || parameters.at(0).empty()) {
        std::cout << fnct::INVALID_ARG << '\n';
        return;
    }

    try {
        fileSystem->createDirectory(parameters.at(0));
    } catch (const std::exception &ex) {
        std::cout << ex.what() << '\n';
        return;
    }

    std::cout << fnct::OK << '\n';
}

void fnct::rmdir(const std::vector<std::string> &parameters, FileSystem *fileSystem) {
    if (fileSystem == nullptr || !fileSystem->isInitialized()) {
        std::cout << "File system is not initialized!\n";
        return;
    }

    if (parameters.empty() || parameters.at(0).empty()) {
        std::cout << fnct::INVALID_ARG << '\n';
        return;
    }

    try {
        fileSystem->removeDirectory(parameters.at(0));
    } catch (const std::exception &ex) {
        std::cout << ex.what() << '\n';
        return;
    }

    std::cout << fnct::OK << '\n';
}

void fnct::cp(const std::vector<std::string> &parameters, FileSystem *fileSystem) {
    if (fileSystem == nullptr || !fileSystem->isInitialized()) {
        std::cout << "File system is not initialized!\n";
        return;
    }

    if (parameters.empty() || parameters.at(0).empty() || parameters.at(1).empty()) {
        std::cout << fnct::INVALID_ARG << '\n';
        return;
    }

    try {
        fileSystem->copyFile(parameters.at(0), parameters.at(1));
    } catch (const std::exception &ex) {
        std::cout << ex.what() << '\n';
    }

    std::cout << fnct::OK << '\n';

}

void fnct::mv(const std::vector<std::string> &parameters, FileSystem *fileSystem) {
    if (fileSystem == nullptr || !fileSystem->isInitialized()) {
        std::cout << "File system is not initialized!\n";
        return;
    }

    if (parameters.empty() || parameters.at(0).empty() || parameters.at(1).empty()) {
        std::cout << fnct::INVALID_ARG << '\n';
        return;
    }

    try {
        fileSystem->moveFile(parameters.at(0), parameters.at(1));
    } catch (const std::exception &ex) {
        std::cout << ex.what() << '\n';
        return;
    }

    std::cout << fnct::OK << '\n';
}