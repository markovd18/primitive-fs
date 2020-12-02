//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_FILENOTFOUNDEXCEPTION_H
#define PRIMITIVE_FS_FILENOTFOUNDEXCEPTION_H

#include <exception>

namespace pfs {
/**
 * Exception indicating that any file was not found.
 */
class FileNotFoundException : public std::exception {

private: //private attributes
    /**
     * Cause of the exception.
     */
    const char* cause;

public: //public methods
    /**
     * Creates exception instance with given cause of the exception.
     *
     * @param cause cause of the exception
     */
    explicit FileNotFoundException(const char* cause = "") : cause(cause) {
        //
    }

    [[nodiscard]] const char * what() const noexcept override {
        return cause;
    }
};

}
#endif //PRIMITIVE_FS_FILENOTFOUNDEXCEPTION_H
