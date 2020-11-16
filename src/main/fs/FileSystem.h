//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_FILESYSTEM_H
#define PRIMITIVE_FS_FILESYSTEM_H

#include <string>
#include <memory>
#include <iostream>

#include "../common/structures.h"

/**
 * Represents the virtual file system loaded by the application. File system is represented by one file where
 * all the data is stored. File system class defines set of operations that can be invoked over the representing
 * file such as writing, reading or deleting data.
 */
class FileSystem {
private: //private attributes
    /**
     * The data file representing the file system.
     */
    std::string dataFileName;
public: //public methods
    /**
     * Default constructor for initialization.
     *
     * @param fileName data-file name
     */
    explicit FileSystem(const std::string& fileName)
        : dataFileName(fileName){
        //
    }

    /**
     * Overloaded assignment operator, which copies the data-file name into this instance.
     *
     * @param otherFs other FileSystem instance
     * @return reference to this FileSystem instance
     */
    FileSystem& operator=(FileSystem&& otherFs){
        dataFileName = otherFs.dataFileName;
        return *this;
    }

    /**
     * Initializes the file system in data file. Writes super-block, bitmaps, and root directory inside.
     *
     * @param superblock superblock to initialize the file system by
     */
    bool initialize(const fs::Superblock& superblock) const;
    /**
     * Returns the name of the data-file which is used as the file system storage.
     *
     * @return data-file name
     */
    const std::string &getDataFileName() const {
        return dataFileName;
    }
};


#endif //PRIMITIVE_FS_FILESYSTEM_H
