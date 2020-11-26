//
// Author: markovd@students.zcu.cz
//

#ifndef PRIMITIVE_FS_RETURNVAL_H
#define PRIMITIVE_FS_RETURNVAL_H

namespace fnct {

    inline static const char* OK = "OK";

    inline static const char* FNF_SOURCE = "FILE NOT FOUND (není zdroj)";

    inline static const char* PNF_DEST = "PATH NOT FOUND (neexistuje cílová cesta)";

    inline static const char* CANNOT_CREATE_FILE = "CANNOT CREATE FILE";

    inline static const char* NOT_EMPTY = "NOT EMPTY (adresář obsahuje podadresáře, nebo soubory)";
}
#endif //PRIMITIVE_FS_RETURNVAL_H
