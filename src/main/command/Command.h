//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_COMMAND_H
#define PRIMITIVE_FS_COMMAND_H

#include <string>
#include <utility>
#include <vector>

/**
 * Set of actions that may be requested by user from CLI.
 */
enum CommandType {
    /**
     * Request to immediately shut-down the application.
     */
    EXIT,
    /**
     * Request to run specific command
     */
    RUN_COMMAND
};

/**
 * Represents command with parameters passed from CLI by user.
 */
class Command {
public: //public attributes
    /**
     * Exit command.
     */
    inline const static std::string exitCommand = "exit";

private: //private attributes
    /**
     * Name of the command, eg. "incp".
     */
    std::string name;
    /**
     * Parameters for the command passed from CLI.
     */
    std::vector<std::string> parameters;
public:
    Command() {
        //
    }
    Command(std::string name, std::vector<std::string> parameters)
            : name(std::move(name)), parameters(std::move(parameters)){
        //
    }

    /**
     * Returns the name of the command.
     *
     * @return name of the command
     */
    [[nodiscard]] const std::string &getName() const {
        return name;
    }

    /**
     * Returns the vector of parameters passed to command.
     *
     * @return vector of parameters
     */
    [[nodiscard]] const std::vector<std::string> &getParameters() const {
        return parameters;
    }

    /**
     * Sets name attribute to new value.
     *
     * @param newName new value
     */
    void setName(const std::string& newName){
        name = newName;
    }

    /**
     * Sets parameters attribute to new value.
     *
     * @param newParameters new value
     */
    void setParameters(const std::vector<std::string> newParameters) {
        parameters = newParameters;
    }
};
#endif
