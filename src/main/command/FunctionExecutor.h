//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_FUNCTIONEXECUTOR_H
#define PRIMITIVE_FS_FUNCTIONEXECUTOR_H

#include <map>
#include <functional>

#include "function.h"

/** For simplicity we define new type */
typedef std::function<void(const std::vector<std::string>&, const DataFile&)> Function;

/**
 * Utility class used for executing individual functions upon the file system. FunctionExecutor contains a map
 * of function names referencing the function itself.
 */
class FunctionExecutor {
private: //private attributes

    inline static const std::map<const std::string, Function> nameToFunctionMap = {
            {"format", &fnct::format }
    };
public:
    static Function getFunction(const std::string& functionName){
        return nameToFunctionMap.find(functionName)->second;
    }
};


#endif //PRIMITIVE_FS_FUNCTIONEXECUTOR_H
