//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_FILESYSTEM_H
#define PRIMITIVE_FS_FILESYSTEM_H

#include <string>
#include <memory>
#include <iostream>
#include "DataFile.h"

/**
 * Represents the virtual file system loaded by the application. File system is represented by one file where
 * all the data is stored. File system class defines set of operations that can be invoked over the representing
 * file such as writing, reading or deleting data.
 */
class FileSystem {
public: //public attributes
private: //private attributes
    /**
     * The data file representing the file system.
     */
    std::unique_ptr<DataFile> dataFile;
public: //public methods
    explicit FileSystem(const std::string& fileName)
        : dataFile(std::make_unique<DataFile>(fileName)){
        //
    }
private: //private methods
};


#endif //PRIMITIVE_FS_FILESYSTEM_H
