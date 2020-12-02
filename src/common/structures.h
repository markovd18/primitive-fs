//
//  Author: markovda@students.zcu.cz
//

#ifndef PRIMITIVE_FS_STRUCTURES_H
#define PRIMITIVE_FS_STRUCTURES_H

#include <cstdint>
#include <cstring>
#include <array>

/**
* Namespace with fundamental parts of the file system.
*/
namespace fs {

    /**
     * ID of free i-node
     */
    constexpr int32_t FREE_INODE_ID = -1;
    /**
     * Value of empty direct of indirect link to data file in i-node.
     */
    constexpr int32_t EMPTY_LINK = -1;

    /**
     * One I-Node describes exactly one file. Directory is also file and can be described by I-Node. It also
     * describes all the data that the file is containing with direct and indirect links. Inode structure has
     * 5 direct links allowed and 2 indirect links allowed, if one stored big files and would run out of direct links.
     * Every Inode is completely represented and can be found by it's ID.
     */
    class Inode {
    public: //public attributes
        static constexpr size_t DIRECT_LINKS_COUNT = 5;     //number of allowed direct links to data blocks
        static constexpr size_t INDIRECT_LINKS_COUNT = 2;   // number of allowed indirect links to data blocks
    private: //private attributes
        int32_t m_nodeId;                     //i-node id - if nodeId = FREE_INODE_ID, then the inode is free
        bool m_isDirectory;                   //file or directory
        int8_t m_references;                  //number of references on i-node - used for hardlinks
        int32_t m_fileSize;                   //size of file in bytes
        std::array<int32_t, DIRECT_LINKS_COUNT> m_directLinks; // direct links to data blocks
        std::array<int32_t, INDIRECT_LINKS_COUNT> m_indirectLinks;   // indirect links to data blocks
    public: //public methods
        Inode() = default;
        Inode(int32_t nodeId, bool isDirectory, int32_t fileSize);

        [[nodiscard]] int32_t getNodeId() const;

        void setNodeId(int32_t nodeId);

        [[nodiscard]] bool isDirectory() const;

        void setIsDirectory(bool isDirectory);

        [[nodiscard]] int8_t getReferences() const;

        void setReferences(int8_t references);

        [[nodiscard]] int32_t getFileSize() const;

        void setFileSize(int32_t fileSize);

        [[nodiscard]] const std::array<int32_t, DIRECT_LINKS_COUNT> &getDirectLinks() const;

        [[nodiscard]] const std::array<int32_t, INDIRECT_LINKS_COUNT> &getIndirectLinks() const;

        bool addDirectLink(int32_t index);

        bool addIndirectLink(int32_t index);
    };

    /**
     * Super-block is the most important structure in the file system. It contains all the much needed information
     * such as size of the disk, number of I-Nodes, addresses of bitmaps and data blocks etc. It is always stored
     * at the front of file system.
     */
    class Superblock {
    public: //public attributes
        static constexpr size_t CLUSTER_SIZE = 1024;            //default cluster size in bytes
    private: //private attributes
        static constexpr size_t SIGNATURE_LENGTH = 10;          //length of author's signature
        static constexpr size_t VOLUME_DESC_LENGTH = 20;        //volume description length
        inline static const char* AUTHOR_NAME = "markovda";     //author's name
        inline static const char* VOLUME_DESCRIPTION = "Primitive file system"; //fs description

        std::array<char, SIGNATURE_LENGTH> m_signature;               //FS author login
        std::array<char, VOLUME_DESC_LENGTH> m_volumeDescription;     //FS description
        int32_t m_diskSize;                   //FS size
        int32_t m_inodeCount;                 //maximum number of i-nodes in file system
        int32_t m_clusterCount;               //number of clusters in FS
        int32_t m_inodeBitmapStartAddress;    //start address of inode bitmap
        int32_t m_dataBitmapStartAddress;     //start address of data bitmap
        int32_t m_inodeStartAddress;          //start address of i-nodes
        int32_t m_dataStartAddress;           //start address of data blocks

    public: //public methods
        /**
         * Initializes super-block to default values
         *
         * @param diskSize      size of file system to be represented by this super-block
         */
        explicit Superblock(size_t newDiskSize);
        Superblock() = default;
        /** Getter for author's signature. */
        [[nodiscard]] const std::array<char, SIGNATURE_LENGTH> &getSignature() const;
        /** Getter for volume description. */
        [[nodiscard]] const std::array<char, VOLUME_DESC_LENGTH> &getVolumeDescription() const;
        /** Getter for the disk size. */
        [[nodiscard]] int32_t getDiskSize() const;
        /** Getter for the maximal cluster count. */
        [[nodiscard]] int32_t getClusterCount() const;
        /** Getter for the i-node-bitmap start address. */
        [[nodiscard]] int32_t getInodeBitmapStartAddress() const;
        /** Getter for the data-bitmap start address.  */
        [[nodiscard]] int32_t getDataBitmapStartAddress() const;
        /** Getter for the address where i-node storage begins. */
        [[nodiscard]] int32_t getInodeStartAddress() const;
        /** Getter for the address where data blocks storage begins. */
        [[nodiscard]] int32_t getDataStartAddress() const;
        /** Getter for the maximum i-node count. */
        [[nodiscard]] int32_t getInodeCount() const;
    };

    /**
     * Represents one item in directory. Only carries information about name of the item
     * and id of corresponding i-node. All the other information, like if it is directory, data links, etc. is
     * carried by the inode structure with provided id.
     */
    class DirectoryItem {
    public: //public attributes
        static constexpr size_t DIR_ITEM_NAME_LENGTH = 12; //length of directory item name (8 chars + 3 chars for extension + \0)
    private: //private attributes
        int32_t m_inodeId;                    //id of corresponding i-node
        std::array<char, DIR_ITEM_NAME_LENGTH> m_itemName;   //name of directory item
    public: //public methods
        /**
         * Default constructor for initialization of directory item.
         *
         * @param itemName name of the file/folder
         * @param inodeId id of corresponding inode
         */
        explicit DirectoryItem(const std::string& itemName, int32_t inodeId);
        /**
         * Returns the length of this directory item name.
         *
         * @return directory item name
         */
        [[nodiscard]] int getItemNameLength() const;
        /**
         * Returns true, if name of this directory item equals to passed string, otherwise false.
         *
         * @param name checked name to be equal to
         * @return true, if equal, otherwise false
         */
        [[nodiscard]] bool nameEquals(const std::string& name) const;
        /** Getter for inode ID. */
        [[nodiscard]] int32_t getInodeId() const;
        /** Getter for item name. */
        [[nodiscard]] const std::array<char, DIR_ITEM_NAME_LENGTH> &getItemName() const;
    };

    /**
     * Wrapper class for dynamic initialization of inode and data-block bitmaps.
     */
    class Bitmap {
        /**
         * The bitmap itself.
         */
        u_char *m_bitmap;
        /**
         * Length of the bitmap.
         */
        std::size_t m_length;
    public:
        explicit Bitmap(const std::size_t length = 0)
                    : m_length(length), m_bitmap(length ? new u_char[length] : nullptr){
            if (length) {
                memset(m_bitmap, 0, length);
            }
        }

        Bitmap(const fs::Bitmap &otherBitmap)
                    : m_length(otherBitmap.m_length), m_bitmap(otherBitmap.m_length ? new u_char[otherBitmap.m_length] : nullptr){
            if (m_length) {
                memset(m_bitmap, 0, m_length);
            }
        }

        /**
         * Friend swap function, required for operator= and move constructor implementation.
         *
         * @param firstBitmap
         * @param secondBitmap
         */
        friend void swap (fs::Bitmap& firstBitmap, fs::Bitmap& secondBitmap) {
            // to enable ADL - maybe not necessary here, but good practice
            using std::swap;

            swap(firstBitmap.m_length, secondBitmap.m_length);
            swap(firstBitmap.m_bitmap, secondBitmap.m_bitmap);
        }

        /**
         * Move constructor.
         *
         * @param otherBitmap rvalue bitmap
         */
        Bitmap(fs::Bitmap&& otherBitmap) noexcept
                    : Bitmap() {
            swap(*this, otherBitmap);
        }

        Bitmap &operator=(fs::Bitmap otherBitmap){
            swap(*this, otherBitmap);

            return *this;
        }

        ~Bitmap(){
            delete[] m_bitmap;
        }

        [[nodiscard]] u_char *getBitmap() const {
            return m_bitmap;
        }

        [[nodiscard]] size_t getLength() const {
            return m_length;
        }
    };
}

#endif
