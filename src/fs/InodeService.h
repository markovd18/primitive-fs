//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_INODESERVICE_H
#define PRIMITIVE_FS_INODESERVICE_H


#include "FileData.h"

namespace pfs {

    /**
     * Class responsible for manipulation with inodes.
     */
    class InodeService {
    private: // private attributes
        /// Data file representing the virtual file system
        std::string m_dataFileName;
        /// Inode bitmap
        fs::Bitmap m_inodeBitmap;
        /// Address where to store the inode bitmap
        int32_t m_inodeBitmapAddress = -1;
        /// Address where the inode storage begins
        int32_t m_inodeStartAddress = -1;

    public: // public methods
        InodeService() = default;
        InodeService(std::string mDataFileName, fs::Bitmap inodeBitmap,
                     int32_t inodeBitmapAddress, int32_t inodeStartAddress);
        /**
         * Returns smallest available inode id, if any is available, otherwise throws ObjectNotFound.
         * Returned id is also an offset from the beginning of the inode storage, meaning that inode with id = 3
         * will be stored at 3 * sizeof(fs::Inode) from the inode storage start.
         *
         * @throw pfs::ObjectNotFound If no free i-node id was found
         * @return smallest available inode id
         */
        [[nodiscard]] int32_t getInodeId() const;
        /**
         * Creates an instance of fs::Inode based on given parameters. This factory method should be used for creation of every
         * inode instance, which is intended to be saved into the data file, since it checks side effects of creating inode,
         * such as if there is any space left for new inode instance in out file system.
         *
         * @param isDirectory will inode represent a directory?
         * @param fileSize size of the represented file
         * @return fs::Inode instance
         */
        [[nodiscard]] fs::Inode createInode(bool isDirectory, int32_t fileSize) const;
        /**
         * Saves given inode into data file.
         *
         * @param inode inode to save
         */
        void saveInode(const fs::Inode &inode);
        /**
         * Returns the inode with given id. If inode with given doesn't exist, throws an exception.
         *
         * @param inodeId id of wanted inode
         * @return inode with given id
         */
        [[nodiscard]] fs::Inode findInode(int inodeId) const;
        /**
         * Deletes given inode from the data file. Note that this method will not delete inode's data at direct
         * and indirect links.
         *
         * @param inode inode to delete
         */
        void removeInode(const fs::Inode &inode);
        /**
         * Finds the i-node of root directory and stores it into the inode passed as a parameter.
         *
         * @param rootInode instance to store rootInode data into
         */
        void getRootInode(fs::Inode &rootInode) const;
        /**
         * Returns a vector of all inodes saved in the file system.
         * @return vector of all inodes
         */
        [[nodiscard]] std::vector<fs::Inode> getAllInodes() const;
    };
}



#endif //PRIMITIVE_FS_INODESERVICE_H
