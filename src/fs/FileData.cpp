//
// Author: markovd@students.zcu.cz
//

#include "FileData.h"
#include "../common/structures.h"

std::vector<std::string> parseData(const std::string& data, const std::size_t clusterSize = fs::Superblock::CLUSTER_SIZE) {
    std::vector<std::string> dataClusters;

    std::size_t index;
    for (index = 0; index < data.length(); index += clusterSize) {
        dataClusters.push_back(data.substr(index, clusterSize));
    }

    return dataClusters;
}

std::vector<std::string> parseData(const fs::FileData& fileData, const std::size_t clusterSize = fs::Superblock::CLUSTER_SIZE) {
    return parseData(fileData.data(), clusterSize);
}