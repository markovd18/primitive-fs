//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_INVALIDSTATE_H
#define PRIMITIVE_FS_INVALIDSTATE_H

#include <exception>

namespace pfs {

    /**
     * Exception indicating an invalid state of the application or an object.
     */
    class InvalidState : public std::exception {
    private: //private attributes
        /**
         * Cause of the exception.
         */
        const char* m_cause;
    public:
        /**
         * Creates an instance of this exception with given cause.
         *
         * @param cause cause of the exception
         */
        explicit InvalidState(const char *cause = "") : m_cause(cause) {
            //
        }

        explicit InvalidState(const std::string& cause) : m_cause(cause.data()) {
            //
        }

        /**
         * Returns the reason why this exception was thrown.
         *
         * @return cause of the exception
         */
        [[nodiscard]] const char * what() const noexcept override {
            return m_cause;
        }
    };
}
#endif //PRIMITIVE_FS_INVALIDSTATE_H
