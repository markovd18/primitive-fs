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
    /**
     * Superblock with fundamental information about the file system.
     */
    const fs::Superblock *superblock;
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
    bool initialize(fs::Superblock& superblock);
    /**
     * Returns the name of the data-file which is used as the file system storage.
     *
     * @return data-file name
     */
    const std::string &getDataFileName() const {
        return dataFileName;
    }
    /**
     * Sets new superblock reference.
     *
     * @param sb new superblock reference
     */
    void setSuperblock(fs::Superblock* sb) {
        this->superblock = sb;
    }

private: //private methods
    /**
     * Writes superblock at the start of the file-system. Requires open input stream to data file passed. If
     * the input stream is closed, returns a failure.
     *
     * @param dataFile open output file stream
     * @param superblock superblock to write
     * @return true when successfully written superblock, otherwise false
     */
    bool writeSuperblock(std::ofstream& dataFile, fs::Superblock& superblock);
    /**
     * Initializes and writes bitmap of inodes into the file-system. Bitmap corresponds to a filesystem with
     * root folder only. Requires open output stream to data file passed. If the input stream is closed,
     * returns a failure.
     *
     * @param dataFile open output file stream
     * @return true when successfully written bitmap, otherwise false
     */
    bool writeInodeBitmap(std::ofstream& dataFile);
    /**
     * Initializes and writes bitmap of data into the file-system. Bitmap corresponds to a filesystem with
     * root folder only. Requires open output stream to data file passed. If the input stream is closed,
     * returns a failure.
     *
     * @param dataFile open output file stream
     * @return true when successfully written bitmap, otherwise false
     */
    bool writeDataBitmap(std::ofstream& dataFile);
    /**
     * Writes bitmap into the file-system with the offset relative to the start of the file system.
     * Requires open output stream to data file passed. If the input stream is closed, returns a failure.
     *
     * @param dataFile open output file stream
     * @param bitmap bitmap to write
     * @param offset offset relative to start of the file-system
     * @return true when successfully written bitmap, otherwise false
     */
    bool writeBitmap(std::ofstream& dataFile, const fs::Bitmap& bitmap, int offset);
};


#endif //PRIMITIVE_FS_FILESYSTEM_H
