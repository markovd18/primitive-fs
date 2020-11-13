//
// Author: markovd@students.zcu.cz
//

#include <vector>

#include "../common/structures.h"
#include "../../utils/InputParamsValidator.h"
#include "../../utils/StringNumberConverter.h"
#include "function.h"

void fnct::format(const std::vector <std::string>& parameters, const DataFile& dataFile) {

    if (!InputParamsValidator::validateFormatFunctionPatameters(parameters)) {
        std::cout << "Invalid parameters passed!\n"
                     "At least one parameter has to be passed - size of the disk to format in megabytes\n";
        return;
    }

    const std::string& diskSizeStr = parameters.at(0);
    Superblock superblock{};
    superblock.initialize(StringNumberConverter::convertStringToInt(diskSizeStr).value);
}

void fnct::incp(const std::vector<std::string> &parameters, const DataFile &dataFile) {

}