//
//  Author: markovda@students.zcu.cz
//

#ifndef PRIMITIVE_FS_STRUCTURES_H
#define PRIMITIVE_FS_STRUCTURES_H

#include <cstdint>
#include <cstring>
#include <array>
#include <string>
#include <vector>
#include "../utils/ObjectNotFound.h"

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
        Superblock(const Superblock& other) = default;
        Superblock(Superblock&& other) = default;
        Superblock& operator=(const Superblock& other) = default;
        Superblock& operator=(Superblock&& other) = default;
        ~Superblock() = default;
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

    class DataLinks;

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
        static constexpr size_t LINKS_IN_INDIRECT = fs::Superblock::CLUSTER_SIZE / sizeof(int32_t); //number of direct links that fit into indirect link
    private: //private attributes
        int32_t m_inodeId = fs::FREE_INODE_ID;                     //i-node id - if nodeId = FREE_INODE_ID, then the inode is free
        bool m_isDirectory = false;                   //file or directory
        int8_t m_references = 1;                  //number of references on i-node - used for hardlinks
        int32_t m_fileSize = 0;                   //size of file in bytes
        std::array<int32_t, DIRECT_LINKS_COUNT> m_directLinks{}; // direct links to data blocks
        std::array<int32_t, INDIRECT_LINKS_COUNT> m_indirectLinks{};   // indirect links to data blocks
    public: //public methods
        Inode();
        Inode(int32_t nodeId, bool isDirectory, int32_t fileSize);
        Inode(Inode& inode) = default;
        Inode(Inode&& inode) = default;
        Inode& operator=(const Inode& inode) = default;
        Inode& operator=(Inode&& inode) = default;
        ~Inode() = default;

        [[nodiscard]] int32_t getInodeId() const;

        void setInodeId(int32_t nodeId);

        [[nodiscard]] bool isDirectory() const;

        void setIsDirectory(bool isDirectory);

        [[nodiscard]] int8_t getReferences() const;

        void setReferences(int8_t references);

        [[nodiscard]] int32_t getFileSize() const;

        void setFileSize(int32_t fileSize);

        [[nodiscard]] const std::array<int32_t, DIRECT_LINKS_COUNT> &getDirectLinks() const;

        [[nodiscard]] const std::array<int32_t, INDIRECT_LINKS_COUNT> &getIndirectLinks() const;

        [[nodiscard]] int32_t getLastFilledDirectLinkValue() const;

        [[nodiscard]] int32_t getFirstFreeDirectLink() const;

        [[nodiscard]] int32_t getLastFilledIndirectLinkValue() const;

        [[nodiscard]] int32_t getFirstFreeIndirectLink() const;

        bool addDirectLink(int32_t index);

        bool addIndirectLink(int32_t index);

        /**
         * Clears all direct and indirect links, setting them to @a fs::EMPTY_LINK
         */
        void clearData();

        /**
         * Fills this inode's direct and indirect links with given data links.
         *
         * @param dataLinks data links to fill inode with
         */
        void setData(const fs::DataLinks& dataLinks);

    private://private methods
        void init() {
            m_directLinks.fill(EMPTY_LINK);
            m_indirectLinks.fill(EMPTY_LINK);
        }
    };

    /**
     * Returns value of last filled index from given array-like container (std::array, std::vector, ...).
     * Index is considered empty when it's value equals to given @a emptyValue. Then returned index is the previous one
     * to the one considered to be filled. If every index is filled, returns the value of last index. If no index is filled,
     * returns @a emptyValue.
     *
     * @tparam T array-like container
     * @tparam V type stored by @a T
     * @param array @a T-typed array-like container instance to iterate through
     * @param emptyValue @a V-typed value marking an empty index
     * @return value of last filled index or @a emptyValue
     */
    template<typename T, typename V>
    V getLastFilledIndexValue(const T& container, V emptyValue);

    /**
     * Returns first free index from given array-like container (std::array, std::vector, ...).
     * Index is considered empty when it's value equals to given @a emptyValue. If no index is free, returns the size of
     * given continer
     *
     * @tparam T array-like container
     * @tparam V type stored by @a T
     * @param array @a T-typed array-like container instance to iterate through
     * @param emptyValue @a V-typed value marking an empty index
     * @return container size or first emtpy index
     */
    template<typename T, typename V>
    V getFirstFreeIndex(const T& conteiner, V emptyValue);

    /**
     * Represents one item in directory. Only carries information about name of the item
     * and id of corresponding i-node. All the other information, like if it is directory, data links, etc. is
     * carried by the inode structure with provided id.
     */
    class DirectoryItem {
    public: //public attributes
        static constexpr size_t DIR_ITEM_NAME_LENGTH = 12; //length of directory item name (8 chars + 3 chars for extension + \0)
    private: //private attributes
        int32_t m_inodeId = FREE_INODE_ID;                    //id of corresponding i-node
        std::array<char, DIR_ITEM_NAME_LENGTH> m_itemName{};   //name of directory item
    public: //public methods
        /**
         * Default constructor for initialization of directory item into non-viable state.
         */
        DirectoryItem() = default;

        /**
         * Default constructor for initialization of directory item.
         *
         * @param itemName name of the file/folder
         * @param inodeId id of corresponding inode
         */
        explicit DirectoryItem(const std::string& itemName, int32_t inodeId);
        DirectoryItem(const DirectoryItem& other) = default;
        DirectoryItem(DirectoryItem&& other) = default;
        DirectoryItem& operator=(const DirectoryItem& other) = default;
        DirectoryItem& operator=(DirectoryItem&& other) = default;
        ~DirectoryItem() = default;
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
        /**
         * Copy constructor.
         *
         * @param otherBitmap copied instance
         */
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

        /**
         * Assignment operator.
         *
         * @param otherBitmap instance to assign
         * @return this instance
         */
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

        /**
         * Returns given number of free indexes. Throws ObjectNotFound if none or less than given number of indexes is found.
         *
         * @param count number of free indexes to find
         * @return vector if free indexes
         * @throw ObjectNotFound if none or less than `count` free indexes is found
         */
        [[nodiscard]] std::vector<int32_t> findFreeIndexes(const std::size_t count) const {
            std::vector<int32_t> freeIndexes;
            /// Iterating through the data bitmap
            for (int i = 0; i < m_length; ++i) {
                for (int j = 7; j >= 0; --j) {
                    if (!((m_bitmap[i] >> j) & 0b1)) {
                        freeIndexes.push_back(i + (7 - j));
                        if (freeIndexes.size() == count) {
                            return freeIndexes;
                        }
                    }
                }
            }

            /// No free index found or not enough of free indexes
            throw pfs::ObjectNotFound("Nepodařilo se najít zadané množství volných indexů bitmapy");
        }

        /**
         * Finds first free index in this bitmap. If none is found, throws ObjectNotFound.
         *
         * @return first free index
         * @throw ObjectNotFound if no free index is found
         */
        [[nodiscard]] int32_t findFirstFreeIndex() const {
            /// We go through the entire bitmap
            for (std::size_t i = 0; i < m_length; ++i) {
                for (std::size_t j = 7; j >= 0; --j) {
                    if (!((m_bitmap[i] >> j) & 0b1)) {
                        /// If the bit is 0, we return it's index as the inode id
                        return i + (7 - j);
                    }
                }
            }
            /// If we get here, the bitmap is full
            throw pfs::ObjectNotFound("Nenalezen žádný volný index v bitmapě");
        }

        /**
         * Sets index-th bit in this bitmap to 1
         * @param index index to set to 1
         */
        void setIndexFilled(const std::size_t index) {
            std::size_t bitmapIndex = index / 8;
            if (bitmapIndex >= m_length) {
                return;
            }

            std::size_t subIndex = 7 - (index % 8);
            m_bitmap[bitmapIndex] |= 1UL << subIndex;
        }

        /**
         * Sets index-th bit in this bitmap to 0
         * @param index index to set to 0
         */
        void setIndexFree(const std::size_t index) {
            std::size_t bitmapIndex = index / 8;
            if (bitmapIndex >= m_length) {
                return;
            }

            std::size_t subIndex =  7 - (index % 8);
            m_bitmap[bitmapIndex] &= ~(1UL << subIndex);
        }
    };

    /**
     * Class uniting direct and indirect links to data blocks of a file.
     */
    class DataLinks {
    private://private attributes
        std::array<int32_t, fs::Inode::DIRECT_LINKS_COUNT> m_directLinks{};
        std::array<int32_t, fs::Inode::INDIRECT_LINKS_COUNT> m_indirectLinks{};

    public://public methods
        /**
         * Creates an instance of data links from given vector of data block indexes.
         * First @a fs::Inode::DIRECT_LINKS_COUNT indexes will be treated as direct links.
         * Then, repeatedly, following index will be treated as indirect link and next
         * @a fs::Inode::DIRECT_LINKS_COUNT indexes will be skipped - should be stored in previous
         * indirect link. Following index is again treated as another indirect link and so on.
         *
         * @param dataClusterIndexes
         */
        explicit DataLinks(const std::vector<int32_t>& dataClusterIndexes);

        [[nodiscard]] std::array<int32_t, fs::Inode::DIRECT_LINKS_COUNT> getDirectLinks() const noexcept {
            return m_directLinks;
        }

        [[nodiscard]] std::array<int32_t, fs::Inode::INDIRECT_LINKS_COUNT> getIndirectLinks() const noexcept {
            return m_indirectLinks;
        }

    private://private methods
        void init() {
            m_directLinks.fill(fs::EMPTY_LINK);
            m_indirectLinks.fill(fs::EMPTY_LINK);
        }
    };


}

#endif
