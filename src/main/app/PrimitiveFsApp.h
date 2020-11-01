//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_PRIMITIVEFSAPP_H
#define PRIMITIVE_FS_PRIMITIVEFSAPP_H

#include <string>

/**
 * Represents the entire application.
 */
class PrimitiveFsApp {
private: //TODO [markovda] create class representing the file representing the file system and set it as private attrib
public: //public methods

    PrimitiveFsApp() = default;

    /**
     * Starts the application. Requires name of the file representing the file system as parameter.
     * If file with given name doesn't exist, creates it in relative path to the executable.
     * If file with given name already exists, assumes that it is valid file for file system representation
     * created by this method.
     *
     * @param fileName  relative path (or name) to the file representing the file system
     */
    void run(const std::string& fileName);
};


#endif
