//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_FUNCTION_H
#define PRIMITIVE_FS_FUNCTION_H

/**
 * Namespace of functions that can be executed upon the filesystem.
 */
#include <vector>
#include <string>
#include "../fs/DataFile.h"

namespace fnct {
    void format(const std::vector<std::string>& parameters, const DataFile& dataFile);
    void incp(const std::vector<std::string>& parameters, const DataFile& dataFile);
}
#endif //PRIMITIVE_FS_FUNCTION_H
