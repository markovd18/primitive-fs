//
// Author: markovd@students.zcu.cz
//

#include <vector>
#include <iostream>
#include "function.h"

void fnct::format(const std::vector <std::string>& parameters, const DataFile& dataFile) {
    for (const auto &parameter : parameters) {
        std::cout << parameter << '\n';
    }
    std::cout << dataFile.getFileName();
    //TODO format
}