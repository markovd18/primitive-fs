//
// Author: markovd@students.zcu.cz
//

#include <unistd.h>
#include <sys/wait.h>
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
    Command command = app::getUserInput(std::cin);
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

void PrimitiveFsApp::runCommand(const Command &command) {
    if (command.getName().empty()){
        return;
    }

    Function function = FunctionMapper::getFunction(command.getName());
    /**
     * We create new child process to execute the function.
     */
    //auto pid = fork(); //TODO uncomment creating child process
    auto pid = 0;
    if (pid < 0) {
        std::cout << "Error creating child process for function execution!\n";
    } else if (pid == 0) {
        /**
         * Child process executes the function.
         */
        if (executeFunction(function, command.getParameters()) != 0) {
            /**
             * If passed command isn't implemented, we print error message and wait for another command.
             */
            std::cout << "Command \"" << command.getName() << "\" not found\n";
        }
        /** Finishing the child process. */
        //exit(0);
    } else {
        /**
         * Parent process waits for child process to finish.
         */
        app::waitForChildProcess();
    }
}

int PrimitiveFsApp::executeFunction(const Function &function, const std::vector<std::string> &parameters) {
    try {
        function(parameters, fileSystem);
        return 0;
    } catch (const std::bad_function_call& exp) {
        return 1;
    }
}

namespace app {

    Command getUserInput(std::istream& inputStream) {
        std::string input;

        getline(inputStream, input);
        if (input.empty()) {
            return Command();
        }

        return app::parseUserInput(input);
    }

    Command parseUserInput(std::string &input) {
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

    void waitForChildProcess() {
        int status;
        auto pid = wait(&status);
        if ((pid < 0) || (status != 0)) {
            std::cout << "Error executing child process!\n";
        }
    }

}





