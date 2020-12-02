//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_INVALIDARGUMENTEXCEPTION_H
#define PRIMITIVE_FS_INVALIDARGUMENTEXCEPTION_H

#include <exception>

namespace pfs {
/**
 * Exception indicating that there was an invalid argument passed to some function.
 */
    class InvalidArgumentException : public std::exception {

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
        explicit InvalidArgumentException(const char* cause = "") : cause(cause) {
            //
        }

        [[nodiscard]] const char * what() const noexcept override {
            return cause;
        }
    };

}
#endif //PRIMITIVE_FS_INVALIDARGUMENTEXCEPTION_H
