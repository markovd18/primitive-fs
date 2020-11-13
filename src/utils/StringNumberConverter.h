//
// Created by markovda on 13.11.20.
//

#ifndef PRIMITIVE_FS_STRINGNUMBERCONVERTER_H
#define PRIMITIVE_FS_STRINGNUMBERCONVERTER_H

#include <string>
#include <charconv>

/**
 * Result of string to int conversion. Used as return value from conversion functions.
 */
struct ConversionResult {
    /** True if the conversion was successful, otherwise false. */
    bool success;
    /** Integer value of the string. If the conversion wasn't successful, is always zero. */
    int value;
};

/**
 * Utility class for converting string literals to numbers.
 */
class StringNumberConverter {
public:

    /**
     * Converts the input string to integer.
     *
     * @param string string to convert
     * @return conversion result
     */
    static ConversionResult convertStringToInt(const std::string& string){
        int intValue;
        ConversionResult result {false, 0};

        auto [invalidChars, errorCode] = std::from_chars(string.data(), string.data() + string.length(), intValue);
        if (errorCode != std::errc(0)) {
            return result;
        }
        result.success = true;
        result.value = intValue;
        return result;
    }
};
#endif //PRIMITIVE_FS_STRINGNUMBERCONVERTER_H
