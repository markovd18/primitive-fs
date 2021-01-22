//
// Author: markovd@students.zcu.cz
//

#include "FileData.h"

namespace fs {

    std::vector<std::string> parseData(const std::string& data, const std::size_t clusterSize) {
        std::vector<std::string> dataClusters;

        for (size_t index = 0; index < data.length(); index += clusterSize) {
            dataClusters.push_back(data.substr(index, clusterSize));
        }

        return dataClusters;
    }

    std::vector<std::string> parseData(const fs::FileData& fileData, const std::size_t clusterSize) {
        return parseData(fileData.data(), clusterSize);
    }

    FileData::FileData(std::string dataBuffer) : m_dataBuffer(std::move(dataBuffer)) {}

    std::string FileData::data() const {
        return m_dataBuffer;
    }

    unsigned long FileData::size() const noexcept {
        return m_dataBuffer.length() - 1;
    }

    ClusteredFileData::ClusteredFileData(const std::string &data) : m_dataClusters(parseData(data)) {}

    ClusteredFileData::ClusteredFileData(const fs::FileData &fileData) : m_dataClusters(parseData(fileData)) {}

    ClusteredFileData::ClusteredFileData(const std::vector<std::string> &dataClusters) {
        std::string data;
        for (const auto &cluster : dataClusters) {
            data += cluster;
        }

        m_dataClusters = parseData(data, fs::Superblock::CLUSTER_SIZE);
    }

    const std::string& ClusteredFileData::at(const size_t index) const {
        return m_dataClusters[index];
    }

    size_t ClusteredFileData::requiredDataBlocks() const noexcept {
        size_t clusterCount = size();
        if (clusterCount <= fs::Inode::DIRECT_LINKS_COUNT) {
            return clusterCount;
        }

        size_t requiredDataBlocks = clusterCount + ((clusterCount - fs::Inode::DIRECT_LINKS_COUNT) / fs::Inode::LINKS_IN_INDIRECT);
        if (((clusterCount - fs::Inode::DIRECT_LINKS_COUNT) % fs::Inode::LINKS_IN_INDIRECT) != 0) {
            requiredDataBlocks++; /// Adding one data block for indirect link to store the links in remainder
        }

        return requiredDataBlocks;

    }

    size_t ClusteredFileData::size() const noexcept {
        return m_dataClusters.size();
    }
}