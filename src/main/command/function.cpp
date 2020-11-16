//
// Author: markovd@students.zcu.cz
//

#include <vector>

#include "../common/structures.h"
#include "../../utils/InputParamsValidator.h"
#include "../../utils/StringNumberConverter.h"
#include "../fs/FileSystem.h"
#include "function.h"

void fnct::format(const std::vector <std::string>& parameters, const FileSystem* fileSystem) {

    if (!InputParamsValidator::validateFormatFunctionPatameters(parameters)) {
        std::cout << "Invalid parameters passed!\n"
                     "At least one parameter has to be passed - size of the disk to format in megabytes\n";
        return;
    }

    const std::string& diskSizeStr = parameters.at(0);
    fs::Superblock superblock(StringNumberConverter::convertStringToInt(diskSizeStr).value);

    if(!fileSystem->initialize(superblock)) {
        //
    }


}

void fnct::incp(const std::vector<std::string> &parameters, const FileSystem* fileSystem) {

}