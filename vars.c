#include "vars.h"

/* Create a new var-bind instruction, with the names from the
 * <names> ANameSeqNode. */
AVarBind *varbind_new(ANameSeqNode *names, AWordSeqNode *words) {
    if (names->length == 0) return NULL; /* bind no new names */

    AVarBind *new_bind = malloc(sizeof(AVarBind));
    new_bind->count = names->length;
    new_bind->words = words;

    ANameNode *curr = names->first;
    unsigned int index = 0;
    while (curr) {
        if (index == new_bind->count) {
            fprintf(stderr, "internal error: received too many symbols at bind "
                            "(expected only %d)\n", new_bind->count);
            return new_bind;
        }
        index++;
        curr = curr->next;
    }

    return new_bind;
}