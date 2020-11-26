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
#include "../fs/FileSystem.h"

namespace fnct {
    /**
     * Formats the data file of the filesystem according to given size, based of the default setting of the {@fs::Superblock}.
     *
     * @param parameters requires requested disk size as first parameter, others are ignored
     * @param fileSystem fileSystem who's datafile has to be formatted
     */
    void format(const std::vector<std::string>& parameters, FileSystem* fileSystem);
    void incp(const std::vector<std::string>& parameters, FileSystem* fileSystem);
}
#endif //PRIMITIVE_FS_FUNCTION_H
