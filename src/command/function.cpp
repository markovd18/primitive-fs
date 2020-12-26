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
#include "../utils/ObjectNotFound.h"

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
        exit(EXIT_FAILURE);
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

    std::ifstream hddFile(hddPath, std::ios_base::binary);
    if (!hddFile) {
        std::cout << fnct::FNF_SOURCE << '\n';
        return;
    }

    const auto fileSize = std::filesystem::file_size(hddPath);
    char fileBuffer[fileSize];
    hddFile.read((char*)fileBuffer, fileSize);

    try {
        fileSystem->createFile(parameters.at(1), fs::FileData(fileBuffer));
        std::cout << fnct::OK << '\n';
    } catch (const std::invalid_argument& ex) {
        std::cout << fnct::PNF_DEST << '\n';
    } catch (const pfs::ObjectNotFound& ex) {
        std::cout << fnct::FS_FULL << '\n';
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
        std::cout << fnct::PNF_DEST << '\n';
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
    } catch (const std::invalid_argument &ex) {
        std::cout << fnct::PNF_DEST << '\n';
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