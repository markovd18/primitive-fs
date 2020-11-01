//
//  Author: markovda@students.zcu.cz
//

#ifndef PRIMITIVE_FS_STRUCTURES_H
#define PRIMITIVE_FS_STRUCTURES_H

#include <cstdint>
#include <cstring>
#include <array>

/**
 * id of free i-node
 */
constexpr int32_t FREE_INODE_ID = -1;
/**
 * Default cluster size in bytes
 */
constexpr int32_t CLUSTER_SIZE = 32;
/**
 * Length of directory item name (8 chars + 3 chars for extension + \0)
 */
constexpr int32_t DIR_ITEM_NAME_LENGTH = 12;
/**
 * Length of author signature
 */
constexpr int32_t SIGNATURE_LENGTH = 10;
/**
 * Volume description length
 */
constexpr int32_t VOLUME_DESC_LENGTH = 20;

/**
 * Super-block is the most important structure in the file system. It contains all the much needed information
 * such as size of the disk, number of I-Nodes, addresses of bitmaps and data blocks etc. It is always stored
 * at the front of file system.
 */
struct superblock {
    std::array<char, SIGNATURE_LENGTH> signature;               //FS author login
    std::array<char, VOLUME_DESC_LENGTH> volumeDescription;     //FS description
    int32_t diskSize;                   //FS size
    int32_t clusterSize;                //cluster size
    int32_t clusterCount;               //number of clusters in FS
    int32_t inodeBitmapStartAddress;    //start address of inode bitmap
    int32_t dataBitmapStartAddress;     //start address of data bitmap
    int32_t inodeStartAddress;          //start address of i-nodes
    int32_t dataStartAddress;           //start address of data blocks
} typedef Superblock;

/**
 * One I-Node describes exactly one file. Directory is also file and can be described by I-Node. It also
 * describes all the data that the file is containing with direct and indirect links. Inode structure has
 * 5 direct links allowed and 2 indirect links allowed, if one stored big files and would run out of direct links.
 * Every Inode is completely represented and can be found by it's ID.
 */
struct inode {
    int32_t nodeId;                     //i-node id - if nodeId = FREE_INODE_ID, then the inode is free
    bool isDirectory;                   //file or directory
    int8_t references;                  //number of references on i-node - used for hardlinks
    int32_t fileSize;                   //size of file in bytes
    int32_t direct1;                    // 1. direct link to data blocks
    int32_t direct2;                    // 2. direct link to data blocks
    int32_t direct3;                    // 3. direct link to data blocks
    int32_t direct4;                    // 4. direct link to data blocks
    int32_t direct5;                    // 5. direct link to data blocks
    int32_t indirect1;                  // 1. indirect link (link - data blocks)
    int32_t indirect2;                  // 2. indirect link (link - link - data blocks)
} typedef Inode;

/**
 * Represents one item in directory. Only carries information about name of the item
 * and id of corresponding i-node. All the other information, like if it is directory, data links, etc. is
 * carried by the inode structure with provided id.
 */
struct directoryItem {
    int32_t inodeId;                    //corresponding i-node
    std::array<char, DIR_ITEM_NAME_LENGTH> itemName;   //name of directory item
} typedef Directory_Item;

/**
 * Initializes superblock to default values
 *
 * @param superblock    super-block to initialize
 * @param diskSize      size of file system to be represented by this super-block
 */
void initializeSuperblock(struct superblock& superblock, const int32_t diskSize){
    strcpy(superblock.signature.data(), "markovda");
    strcpy(superblock.volumeDescription.data(), "Primitive file system");
    superblock.diskSize = diskSize;
    superblock.clusterSize = CLUSTER_SIZE;
    superblock.clusterCount; //TODO dopočítat??
    superblock.inodeBitmapStartAddress = sizeof(struct superblock);
    superblock.dataBitmapStartAddress; //TODO dopočítat??
    //...

}
#endif
