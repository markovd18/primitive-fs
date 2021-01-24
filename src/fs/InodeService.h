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
        std::string m_dataFileName;
        fs::Bitmap m_inodeBitmap;
        int32_t m_inodeBitmapAddress = -1;
        int32_t m_inodeStartAddress = -1;

    public: // public methods
        InodeService() = default;
        InodeService(std::string mDataFileName, fs::Bitmap inodeBitmap,
                     int32_t inodeBitmapAddress, int32_t inodeStartAddress);

        [[nodiscard]] int32_t getInodeId() const;
        [[nodiscard]] fs::Inode createInode(bool isDirectory, int32_t fileSize) const;
        void saveInode(const fs::Inode &inode);
        [[nodiscard]] fs::Inode findInode(int inodeId) const;
        void removeInode(const fs::Inode &inode);
        void getRootInode(fs::Inode &rootInode) const;

    private: // private methods
    };
}



#endif //PRIMITIVE_FS_INODESERVICE_H
