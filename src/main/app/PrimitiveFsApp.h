//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_PRIMITIVEFSAPP_H
#define PRIMITIVE_FS_PRIMITIVEFSAPP_H

#include <string>
#include <memory>
#include "../fs/FileSystem.h"
#include "../command/Command.h"

/**
 * Represents the entire application.
 */
class PrimitiveFsApp {
private: //private attributes
    /**
     * File system that is manipulated with by this app.
     */
    FileSystem* fileSystem;
    /**
     * CLI indicator of awaiting user input
     */
    inline static char const* CLI_MARKER = "$ ";
public: //public methods

    /**
     * Initializes the application. Requires name of the file representing the file system as parameter.
     * If file with given name doesn't exist, creates it in relative path to the executable.
     * If file with given name already exists, assumes that it is valid file for file system representation
     * created by this constructor.
     */
    explicit PrimitiveFsApp(const std::string& fileName);
    ~PrimitiveFsApp();

    /**
     * Launches the application and starts awaiting user input.
     */
    void run();

    /**
     * Starts waiting for user input from CLI. Manages the resolution of user requests. Returns information about whether
     * there was exit instruction or instruction to run some other function like copying a file.
     *
     * @return  UserRequest#EXIT if there was request to exit the application, otherwise UserRequest#RUN_COMMAND
     */
    CommandType manageUserInput();

    /**
     * Waits for and parses user input from CLI and returns it as an appropriate class.
     *
     * @return class representing user input from CLI
     */
    Command getUserInput();

    /**
     * Performs necessary validations before running the command and runs it.
     *
     * @param command command to run
     */
    void runCommand(const Command& command);
private: //private methods
    /**
     * Prints CLI marker on console.
     */
    static void printCliMarker(){
        std::cout << CLI_MARKER;
        //TODO printing work directory too?? and cd command
    }

    /**
     * Parses user input into one command. Input is parsed by space delimiter. First token is considered to be a function name
     * and other tokens are considered to be function parameters.
     *
     * @param input user intput from CLI
     * @return parsed command with parameters
     */
    Command parseUserInput(std::string& input);
};


#endif
