#include <stdlib.h>    /* calloc()...                        */
#include <string.h>    /* strlen(), strncat, strcopy()...    */
#include <assert.h>    /* assert()...                        */
#include "strextra.h"  /* Interfaz                           */

char * strmerge(char *s1, char *s2) {
    char *merge = NULL;
    size_t len_s1 = strlen(s1);
    size_t len_s2 = strlen(s2);
    assert(s1 != NULL && s2 != NULL);

    merge = calloc(len_s1 + len_s2 + 1, sizeof(char));
    
    if (merge != NULL) {
        strcpy(merge, s1);
        strcat(merge, s2);
        assert(strlen(merge) == len_s1 + len_s2);
    }
    
    return merge;
}

