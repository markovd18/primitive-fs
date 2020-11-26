//
// Author: markovd@students.zcu.cz
//

#include <vector>

#include "../common/structures.h"
#include "../../utils/InputParamsValidator.h"
#include "../../utils/StringNumberConverter.h"
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
        exit(EXIT_FAILURE);
    }

    std::cout << fnct::OK << '\n';
}

void fnct::incp(const std::vector<std::string> &parameters, FileSystem* fileSystem) {
    if (fileSystem == nullptr || !fileSystem->isInitialized()) {
        std::cout << "FIle system is not initialized!\n";
        return;
    }

    if (!InputParamsValidator::validateIncpFunctionParameters(parameters)) {
        std::cout << fnct::FNF_SOURCE << '\n';
        return;
    }

    //TODO markovd read file from hard-disk into memory and write it into the data file - create inodes, data etc.

    std::cout << fnct::OK << '\n';

}