//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_FILEDATA_H
#define PRIMITIVE_FS_FILEDATA_H

#include <string>
#include <utility>
#include <vector>
#include "../common/structures.h"

namespace fs {

    /**
     * Class representing the data of a file.
     */
    class FileData {
    private://private attributes
        std::string m_dataBuffer;

    public://public methods
        explicit FileData(std::string dataBuffer);

        [[nodiscard]] std::string data() const;

        /**
         * Returns number of bytes used by the data.
         * @return size of the data in bytes
         */
        [[nodiscard]] int size() const noexcept;
    };

    /**
     * Parses given data into individual clusters of given size.
     *
     * @param data data to be parsed
     * @param clusterSize cluster size
     * @return vector of parsed data
     */
    std::vector<std::string> parseData(const std::string& data, std::size_t clusterSize = fs::Superblock::CLUSTER_SIZE);

    /**
     * Parses given file data into individual clusters of given size.
     *
     * @param fileData file data to parse
     * @param clusterSize cluster size
     * @return vector of parsed data
     */
    std::vector<std::string> parseData(const fs::FileData& fileData, std::size_t clusterSize = fs::Superblock::CLUSTER_SIZE);

    /**
     * Class representing the data of a file already parsed into individual clusters. Maximal size of one cluster is
     * @a fs::Superblock::CLUSTER_SIZE.
     */
    class ClusteredFileData {
    private://private attributes
        std::vector<std::string> m_dataClusters;

    public://public methods
        /**
         * Creates an instance from given vector of strings. In case the strings in given vector aren't long enough or
         * are longer, all the data is concatenated into one data buffer and then parsed by proper length.
         *
         * @param dataClusters vector of data
         */
        explicit ClusteredFileData(const std::vector<std::string>& dataClusters);

        /**
         * Creates an instance from given string of data. Data is parsed by proper length.
         *
         * @param data string of data to be parsed into clusters
         */
        explicit ClusteredFileData(const std::string& data);

        /**
         * Creates an instance from given @a FileData. File data is parsed by proper length.
         *
         * @param fileData file data to be parsed into clusters
         */
        explicit ClusteredFileData(const fs::FileData& fileData);

        /**
         * Returns a read-only reference to @a index-th cluster
         *
         * @param index index of a cluster
         * @return read-only write reference to a cluster
         */
        [[nodiscard]] const std::string& at(size_t index) const;

        /**
         * Returns a number of required data blocks required to store the data contained by this instance. It takes to account
         * that an inode has only @a fs::Inode::DIRECT_LINKS_COUNT of direct links and needs additional data blocks to
         * store indirect links if there is more data to be stored.
         *
         * @return number of required data blocks to store contained data
         */
        [[nodiscard]] size_t requiredDataBlocks() const noexcept;

        /**
         * Returns number of clusters stored.
         *
         * @return number of stored clusters.
         */
        [[nodiscard]] size_t size() const noexcept;
    };
}
#endif //PRIMITIVE_FS_FILEDATA_H
