#include "token.h"

namespace tiger {



char * kTokenString[]={
"kToken_ID",
"kToken_NUM",
"kToken_STR",
"kToken_LPAR",
"kToken_RPAR",
"kToken_LBRA",
"kToken_RBRA",
/* add more tokens */
"kToken_ADD",
"kToken_SUB",
"kToken_MUL",
"kToken_DIV",
"kToken_ASSIGN",
"kToken_COMMA",

/* keywords */
"for",
"if",
"function",
"end",

"kToken_EOT",/* the end of token stream */
"kToken_Unknown",
"kToken_Unused"
};

} // namespace tiger