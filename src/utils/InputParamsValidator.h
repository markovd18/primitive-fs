//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_INPUTPARAMSVALIDATOR_H
#define PRIMITIVE_FS_INPUTPARAMSVALIDATOR_H

#include <algorithm>
#include <charconv>
#include <iostream>
#include <filesystem>

/**
 * Static utility class for input parameter validation.
 */
class InputParamsValidator {
private:

    /**
     * Required number of CLI parameters passed to application.
     */
    static constexpr int REQUIRED_CLI_PARAMS_COUNT = 2;

    /**
     * Required number of parameters to pass to the fnct::format function.
     */
    static constexpr int FORMAT_FUNCTION_PARAMS_COUNT = 1;

    /**
     * Required number of parameters to pass to the fnct::incp function.
     */
    static constexpr int INCP_FUNCTION_PARAMS_COUNT = 2;

public://public methods

    /**
     * Return code for invalid number of arguments being passed from CLI.
     */
    static constexpr int EXIT_INVALID_ARG_COUNT = -1;

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

    /**
     * Validates input parameters for the "format" function - the disk size. The parameter comes from the CLI, therefore
     * it is a std::string type. The input string has to be a numeric value.
     *
     * @param parameters string value of the disk size to format
     * @return true if and only if the string value of disk size is numeric value
     */
    static bool validateFormatFunctionPatameters(const std::vector<std::string>& parameters) {
        if (parameters.size() < FORMAT_FUNCTION_PARAMS_COUNT) {
            return false;
        }

        if (!isNumber(parameters.at(0))) {
            return false;
        }

        return true;
    }

    /**
     * Validates the input parameters for the "incp" function. The function expects at least two parameters - path to file on a hard drive
     * and a path in the virtual file system. Path on the real hard drive has to exist. Path on the virtual file system cannot be validated here, because
     * this class has no access to the virtual file system structure and has to be validated inside the "incp" function itself.
     *
     * @param parameters two parameters - path to file on a hard drive and a path in the virtual file system
     * @return true, if the parameters are valid, otherwise false
     */
    static bool validateIncpFunctionParameters(const std::vector<std::string>& parameters) {
        if (parameters.size() < INCP_FUNCTION_PARAMS_COUNT) {
            return false;
        }

        if (!std::filesystem::exists(parameters.at(0)) || std::filesystem::is_directory(parameters.at(0))) {
            return false;
        }

        return true;
    }

private: //private methods

    /**
     * Checks if passed string represents a number. Iterates through every character and when non-digit character is found,
     * the string is considered to not be a number.
     *
     * @param s string to check
     * @return true if and only if there are none non-digit characters in the input string, otherwise false
     */
    static bool isNumber(const std::string& s)
    {
        return !s.empty() && std::find_if(s.begin(),
                                          s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
    }

    InputParamsValidator() = delete;
    ~InputParamsValidator() = delete;
};


#endif
