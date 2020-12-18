//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_FILEDATAUTILS_H
#define PRIMITIVE_FS_FILEDATAUTILS_H

#include <string>
#include "../fs/FileData.h"

/**
 * Namespace for working with data od individual files.
 */
namespace pfs::data {

    /**
     * Parses given file data into individual clusters of given size.
     *
     * @param fileData file data to parse
     * @param clusterSize cluster size
     * @return vector of parsed data
     */
    std::vector<std::string> parseData(const fs::FileData& fileData, const std::size_t clusterSize) {
        std::string data = fileData.data();
        std::vector<std::string> dataClusters;

        std::size_t index;
        for (index = 0; index < data.length(); index += (clusterSize + 1)) {
            dataClusters.push_back(data.substr(index, clusterSize));
        }

        return dataClusters;
    }
}

#endif //PRIMITIVE_FS_FILEDATAUTILS_H
