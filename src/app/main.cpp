//
// Author: markovd@students.zcu.cz
//

#include <iostream>

#include "../utils/InputParamsValidator.h"
#include "PrimitiveFsApp.h"

int main(int argc, char** argv) {

    /**
     * Validating passed parameters before we start.
     */
    if(!InputParamsValidator::validateCommandLineParameters(argc)){
        std::cout << "Invalid number of parameters passed!\n"
                     "At least one parameter has to be passed - name of the file representing file system\n";
        return InputParamsValidator::EXIT_INVALID_ARG_COUNT;
    }

    /**
     * We have valid arguments, we can run.
     */
    PrimitiveFsApp application(argv[1]);
    application.run();
    return EXIT_SUCCESS;
}
