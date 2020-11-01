//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_INPUTPARAMSVALIDATOR_H
#define PRIMITIVE_FS_INPUTPARAMSVALIDATOR_H

/**
 * Static utility class for input parameter validation.
 */
class InputParamsValidator {
private:

    /**
     * Required number of CLI parameters passed to application.
     */
    static constexpr int8_t REQUIRED_CLI_PARAMS_COUNT = 2;

public:

    /**
     * Return code for invalid number of arguments being passed from CLI.
     */
    static constexpr int8_t EXIT_INVALID_ARG_COUNT = -1;

    /**
     * Validates parameters passed from CLI. Since there should be always only one parameter - name of the
     * file representing our file system, we are validating only to parameter count, since with non-existing file
     * parameter it would just create a new one which is desired behavior.
     *
     * @param argc  argument count
     * @return true if and only if there is more than one parameter passed from CLI
     */
    static bool validateCommandLineParameters(int argc){
        /**
         * We always want at least one input parameter from the command line - name of the file that represents
         * the file system.
         */
        if (argc < REQUIRED_CLI_PARAMS_COUNT){
            return false;
        }
        return true;
    }

    InputParamsValidator() = delete;
    ~InputParamsValidator() = delete;
};


#endif
