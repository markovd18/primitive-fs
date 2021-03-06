//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_FUNCTIONMAPPER_H
#define PRIMITIVE_FS_FUNCTIONMAPPER_H

#include <map>
#include <functional>

#include "function.h"

/** For simplicity we define new type */
typedef std::function<void(const std::vector<std::string>&, FileSystem*)> Function;

/**
 * Utility class used for executing individual functions upon the file system. FunctionMapper contains a map
 * of function names referencing the function itself.
 */
class FunctionMapper {
private: //private attributes
    /**
     * Function names mapped to callable function references.
     */
    inline static const std::map<const std::string, Function> nameToFunctionMap = {
            {"format", &fnct::format },
            {"incp", &fnct::incp},
            {"pwd", &fnct::pwd},
            {"cd", &fnct::cd},
            {"ls", &fnct::ls},
            {"rm", &fnct::rm},
            {"cat", &fnct::cat},
            {"outcp", &fnct::outcp},
            {"info", &fnct::info},
            {"mkdir", &fnct::mkdir},
            {"rmdir", &fnct::rmdir},
            {"cp", &fnct::cp},
            {"mv", &fnct::mv},
            {"load", &fnct::load},
            {"check", &fnct::check},
            {"break", &fnct::breakData}
    };
public: //public methods

    /**
     * Returns function reference according to a given name. If name with no associated function is given, still
     * returns executable Function, which will throw std::bad_function_call when called.
     *
     * @param functionName name of the function
     * @return reference to a function with given name
     */
    static Function getFunction(const std::string& functionName){
        return nameToFunctionMap.find(functionName)->second;
    }
};


#endif //PRIMITIVE_FS_FUNCTIONMAPPER_H
