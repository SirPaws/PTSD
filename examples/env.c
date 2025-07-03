#include "pio.h"
#include "pplatform.h"
#include "pstring.h"

int main(void) {
    pstring_t variable;
    if ((variable = penv("JAVA_HOME")).length) {//NOLINT
        pprintf("found java home at '%S'\n", variable);
        pfree_string(&variable);
    } else {
        pprintf("unable to find java home\n", variable);
    }
}
