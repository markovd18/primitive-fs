//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_DATAFILE_H
#define PRIMITIVE_FS_DATAFILE_H


#include <string>
#include <utility>

/**
 * Represents the data file that represents the entire file system. Allows operations such as open, clone, write,
 * read etc.
 */
class DataFile {
private: //private attributes
    /**
     * Name of the file.
     */
    const std::string fileName;

public: //public methods
    explicit DataFile(const std::string& fileName)
        : fileName(fileName){
        //
    }

    explicit DataFile(const std::string&& fileName)
        : fileName(std::move(fileName)){
        //
    }
    /**
     * Returns the name of the data file.
     * @return name of the data file
     */
    [[nodiscard]] const std::string& getFileName() const {
        return fileName;
    }

};


#endif //PRIMITIVE_FS_DATAFILE_H
