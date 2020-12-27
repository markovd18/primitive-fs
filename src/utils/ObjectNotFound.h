//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_OBJECTNOTFOUND_H
#define PRIMITIVE_FS_OBJECTNOTFOUND_H

#include <exception>

namespace pfs {

/**
 * Exception indicating that some object was not found.
 */
class ObjectNotFound : public std::exception {
private: //private attributes
    /**
     * Cause of the exception.
     */
    const char* cause;
public:
    /**
     * Creates an instance of this exception with given cause.
     *
     * @param cause cause of the exception
     */
    explicit ObjectNotFound(const char *cause = "") : cause(cause) {
        //
    }

    explicit ObjectNotFound(const std::string& cause) : cause(cause.data()) {
        //
    }

    /**
     * Returns the reason why this exception was thrown.
     *
     * @return cause of the exception
     */
    [[nodiscard]] const char * what() const noexcept override {
        return cause;
    }
};

}
#endif //PRIMITIVE_FS_OBJECTNOTFOUND_H
