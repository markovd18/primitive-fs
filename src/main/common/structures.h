//
//  Author: markovda@students.zcu.cz
//

#ifndef PRIMITIVE_FS_STRUCTURES_H
#define PRIMITIVE_FS_STRUCTURES_H

#include <cstdint>
#include <cstring>
#include <array>

/**
 * ID of free i-node
 */
constexpr int32_t FREE_INODE_ID = -1;
/**
 * Default cluster size in bytes
 */
constexpr int32_t CLUSTER_SIZE = 512;

/**
 * One I-Node describes exactly one file. Directory is also file and can be described by I-Node. It also
 * describes all the data that the file is containing with direct and indirect links. Inode structure has
 * 5 direct links allowed and 2 indirect links allowed, if one stored big files and would run out of direct links.
 * Every Inode is completely represented and can be found by it's ID.
 */
struct inode {
private:
    static constexpr size_t DIRECT_LINKS_COUNT = 5;     //number of allowed direct links to data blocks
    static constexpr size_t INDIRECT_LINKS_COUNT = 2;   // number of allowed indirect links to data blocks
public:
    int32_t nodeId;                     //i-node id - if nodeId = FREE_INODE_ID, then the inode is free
    bool isDirectory;                   //file or directory
    int8_t references;                  //number of references on i-node - used for hardlinks
    int32_t fileSize;                   //size of file in bytes
    std::array<int32_t, DIRECT_LINKS_COUNT> directLinks; // direct links to data blocks
    std::array<int32_t, INDIRECT_LINKS_COUNT> indirectLinks;   // indirect links to data blocks
} typedef Inode;

/**
 * Super-block is the most important structure in the file system. It contains all the much needed information
 * such as size of the disk, number of I-Nodes, addresses of bitmaps and data blocks etc. It is always stored
 * at the front of file system.
 */
struct superblock {
private:
    static constexpr size_t SIGNATURE_LENGTH = 10;          //length of author's signature
    static constexpr size_t VOLUME_DESC_LENGTH = 20;        //volume description length
public:
    std::array<char, SIGNATURE_LENGTH> signature;               //FS author login
    std::array<char, VOLUME_DESC_LENGTH> volumeDescription;     //FS description
    int32_t diskSize;                   //FS size
    int32_t clusterSize;                //cluster size
    int32_t clusterCount;               //number of clusters in FS
    int32_t inodeBitmapStartAddress;    //start address of inode bitmap
    int32_t dataBitmapStartAddress;     //start address of data bitmap
    int32_t inodeStartAddress;          //start address of i-nodes
    int32_t dataStartAddress;           //start address of data blocks

    /**
     * Initializes super-block to default values
     *
     * @param diskSize      size of file system to be represented by this super-block
     */
    void initialize(const size_t newDiskSize){
        strcpy(signature.data(), "markovda");
        strcpy(volumeDescription.data(), "Primitive file system");
        diskSize = newDiskSize;
        clusterSize = CLUSTER_SIZE;
        //clusterCount; //TODO dopočítat??
        inodeBitmapStartAddress = sizeof(struct superblock);
        dataBitmapStartAddress = inodeBitmapStartAddress + 100;
        inodeStartAddress = dataBitmapStartAddress + 100;
        dataStartAddress = inodeStartAddress + 100 * sizeof(struct inode);
    }
} typedef Superblock;

/**
 * Represents one item in directory. Only carries information about name of the item
 * and id of corresponding i-node. All the other information, like if it is directory, data links, etc. is
 * carried by the inode structure with provided id.
 */
struct directoryItem {
private:
    static constexpr size_t DIR_ITEM_NAME_LENGTH = 12; //length of directory item name (8 chars + 3 chars for extension + \0)
public:
    int32_t inodeId;                    //id of corresponding i-node
    std::array<char, DIR_ITEM_NAME_LENGTH> itemName;   //name of directory item
} typedef Directory_Item;

#endif
