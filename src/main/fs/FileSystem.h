//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_FILESYSTEM_H
#define PRIMITIVE_FS_FILESYSTEM_H

#include <string>
#include <memory>
#include <iostream>
#include <filesystem>

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
    std::string m_dataFileName;
    /**
     * Is file system initialized?
     */
    bool m_initialized;
    /**
     * Superblock with fundamental information about the file system.
     */
    fs::Superblock m_superblock;
    /**
     * Bitmap of used and unused memory addresses for i-nodes.
     */
    fs::Bitmap m_inodeBitmap;
    /**
     * Bitmap of used and unused memory addresses for data blocks.
     */
    fs::Bitmap m_dataBitmap;
public: //public methods
    /**
     * Default constructor for initialization.
     *
     * @param fileName data-file name
     */
    explicit FileSystem(const std::string& fileName)
        : m_dataFileName(fileName), m_initialized(false){
        if (std::filesystem::exists(fileName)) {
            try {
                initializeFromExisting();
            } catch (std::exception& exception) {
                std::cout << "Error while file system initialization!\n";
                std::cout << exception.what();
                exit(EXIT_FAILURE);
            }
        }
    }

    /**
     * Overloaded assignment operator, which copies the data-file name into this instance.
     *
     * @param otherFs other FileSystem instance
     * @return reference to this FileSystem instance
     */
    FileSystem& operator=(FileSystem&& otherFs){
        m_dataFileName = otherFs.m_dataFileName;
        return *this;
    }

    /**
     * Initializes the file system in data file. Writes super-block, bitmaps, and root directory inside.
     *
     * @param superblock superblock to initialize the file system by
     */
    bool initialize(fs::Superblock& superblock);

    /**
     * Initializes the file system from existing data file. Reads super-block and bitmaps into memory.
     *
     * @return
     */
    bool initializeFromExisting();

    /**
     * Returns true, if the file system has been correctly initialized, otherwise false.
     *
     * @return true, if the file system has been correctly initialized, otherwise false
     */
    [[nodiscard]] bool isInitialized() const {
        return m_initialized;
    }

    /**
     * Returns the name of the data-file which is used as the file system storage.
     *
     * @return data-file name
     */
    const std::string &getDataFileName() const {
        return m_dataFileName;
    }
    /**
     * Sets new superblock reference.
     *
     * @param sb new superblock reference
     */
    void setSuperblock(fs::Superblock& sb) {
        m_superblock = sb;
    }

    /**
     * Sets new inode bitmap reference.
     *
     * @param inodeBitmap
     */
    void setInodeBitmap(fs::Bitmap& inodeBitmap) {
        m_inodeBitmap = inodeBitmap;
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
     * root folder only. Requires open output stream to data file passed. If the output stream is closed,
     * returns a failure.
     *
     * @param dataFile open output file stream
     * @return true when successfully written bitmap, otherwise false
     */
    bool initializeInodeBitmap(std::ofstream& dataFile);
    /**
     * Initializes inode bitmap from existing data file and loads it into memory. Requires open input stream from existing
     * data file passed. If the input stream is closed, returns a failure. If the reading process fails, throws an exception.
     *
     * @param dataFile open input file stream
     * @return true when successfully read inode bitmap, otherwise false
     */
    bool initializeInodeBitmap(std::ifstream& dataFile);
    /**
     * Initializes and writes bitmap of data into the file-system. Bitmap corresponds to a filesystem with
     * root folder only. Requires open output stream to data file passed. If the output stream is closed,
     * returns a failure.
     *
     * @param dataFile open output file stream
     * @return true when successfully written bitmap, otherwise false
     */
    bool initializeDataBitmap(std::ofstream& dataFile);
    /**
     * Initializes data bitmap from existing data file and loads it into memory. Requires open input stream from existing
     * data file passed. If the input stream is closed, returns a failure. If the reading process fails, throws an exception.
     *
     * @param dataFile open input file stream
     * @return true when successfully read data bitmap, otherwise false
     */
    bool initializeDataBitmap(std::ifstream& dataFile);
    /**
     * Writes bitmap into the data file with the offset relative to the start of the file system.
     * Requires open output stream to data file passed. If the output stream is closed, returns a failure.
     *
     * @param dataFile open output file stream
     * @param bitmap bitmap to write
     * @param offset offset relative to the start of the file system
     * @return true when successfully written bitmap, otherwise false
     */
    bool writeBitmap(std::ofstream& dataFile, const fs::Bitmap& bitmap, int offset);
    /**
     * Reads bitmap from the data file from the position relative to the start of the file system with given offset.
     * Requires open input stream from data file passed. If the input stream is closed, returns a failure. Passed bitmap has to be
     * already initialized into default state - array pointer has to be initialized and length has to be set. Length attribute will be used
     * to determine how many bytes to read from the data file.
     *
     * @param dataFile open input file stream
     * @param bitmap bitmap to read into
     * @param offset offset relative to the start of the file system
     * @return true when successfully read bitmap, otherwise false
     */
    bool readBitmap(std::ifstream& dataFile, fs::Bitmap& bitmap, int offset);
};


#endif //PRIMITIVE_FS_FILESYSTEM_H
