//
// Author: markovd@students.zcu.cz
//

#include "PrimitiveFsApp.h"
#include "../common/structures.h"
#include "../command/FunctionMapper.h"

PrimitiveFsApp::PrimitiveFsApp(const std::string& fileName) {
    fileSystem = new FileSystem(fileName);
}

PrimitiveFsApp::~PrimitiveFsApp() {
    delete fileSystem;
}

void PrimitiveFsApp::run() {
    CommandType inputCommandType;
    /**
     * We run until there is request from user to shut down. Then we quit.
     */
    do {
        inputCommandType = manageUserInput();

    } while (inputCommandType != CommandType::EXIT);
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
        runCommand(command);
    }

    return CommandType::RUN_COMMAND;
}

Command PrimitiveFsApp::getUserInput() {
    std::string input;

    getline(std::cin, input);
    if (input.empty()) {
        return Command();
    }

    return parseUserInput(input);
}

Command PrimitiveFsApp::parseUserInput(std::string &input) {
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

void PrimitiveFsApp::runCommand(const Command &command) {
    if (command.getName().empty()){
        return;
    }

    Function function = FunctionMapper::getFunction(command.getName());
    try {
        function(command.getParameters(), fileSystem);
    } catch (const std::bad_function_call& exp) {
        /**
         * If passed command isn't implemented, we print error message and wait for another command.
         */
        std::cout << "Command \"" << command.getName() << "\" not found\n";
    }
}
