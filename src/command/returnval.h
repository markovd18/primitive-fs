//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_RETURNVAL_H
#define PRIMITIVE_FS_RETURNVAL_H

namespace fnct {
    /** Success. */
    inline static const char* OK = "OK";
    /** Source file was not found. */
    inline static const char* FNF_SOURCE = "FILE NOT FOUND (není zdroj)";
    /** Destination path was not found. */
    inline static const char* PNF_DEST = "PATH NOT FOUND (neexistuje cílová cesta)";
    /** Error while creating a file*/
    inline static const char* CANNOT_CREATE_FILE = "CANNOT CREATE FILE";
    /** The directory is not empty. */
    inline static const char* NOT_EMPTY = "NOT EMPTY (adresář obsahuje podadresáře, nebo soubory)";
}
#endif //PRIMITIVE_FS_RETURNVAL_H
