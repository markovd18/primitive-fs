//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_FILEDATA_H
#define PRIMITIVE_FS_FILEDATA_H

#include <string>
#include <utility>

namespace fs {

    /**
     * Class representing the data of a file.
     */
    class FileData {
    private://private attributes
        std::string m_dataBuffer;

    public://public methods
        explicit FileData(std::string dataBuffer) : m_dataBuffer(std::move(dataBuffer)) {}

        [[nodiscard]] std::string data() const { return m_dataBuffer; }

        /**
         * Returns number of bytes used by the data.
         * @return size of the data in bytes
         */
        [[nodiscard]] int size() const noexcept {
            return m_dataBuffer.length();
        }
    };
}
#endif //PRIMITIVE_FS_FILEDATA_H
