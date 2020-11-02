//
// Author: markovd@students.zcu.cz
//

#include "PrimitiveFsApp.h"
#include "../command/Command.h"

void PrimitiveFsApp::run(const std::string& fileName) {
    fileSystem = std::make_unique<FileSystem>(fileName);

    CommandType requestToExit;
    /**
     * We run until there is request from user to shut down. Then we quit.
     */
    do {
        requestToExit = manageUserInput();

    } while (requestToExit != CommandType::EXIT);
}

CommandType PrimitiveFsApp::manageUserInput() {
    printCliMarker();

    /**
     * We get the command user wants to run.
     */
    Command command = getUserInput();
    if (command.getName() == Command::exitCommand) {
        return CommandType::EXIT;
    }
    /**
     * If empty command was passed, we ignore it.
     */
    if (!command.getName().empty()) {
        //TODO run command
    }

    return CommandType::RUN_COMMAND;
}

Command PrimitiveFsApp::getUserInput() {
    std::string input;
    /**
     * We read the user input.
     */
    getline(std::cin, input);
    if (input.empty()) {
        return Command();
    }
    /**
     * We need to parse the input by the space delimiter to get the name of the command
     * and passed parameters.
     * First we get the name of the command.
     */
    size_t position;
    Command command;
    std::vector<std::string> parameters;
    if ((position = input.find(' ')) != std::string::npos) {
        command.setName(input.substr(0, position));
        input.erase(0, position + 1); /** +1 for the length of space delimiter */
        /**
         * Now we get all the parameters from input line.
         */
        while ((position = input.find(' ')) != std::string::npos) {
            parameters.push_back(input.substr(0, position));
            input.erase(0, position + 1); /** +1 for the length of space delimiter */
        }
        parameters.push_back(input); /** Saving the last parameter*/
        command.setParameters(parameters);
        return command;
    } else {
        /**
         * No space in input means that there were no parameters passed.
         * We return just plain command.
         */
        command.setName(input);
        return command;
    }
}
