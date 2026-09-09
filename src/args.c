#include "args.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

int parse_pid(const char *text, unsigned int *pid)
{
    char *end;
    unsigned long value;

    if (text == NULL || pid == NULL) {
        return 0;
    }

    errno = 0;

    value = strtoul(text, &end, 10);

    if (text == end) {
        return 0;
    }

    if (*end != '\0') {
        return 0;
    }

    if (errno == ERANGE) {
        return 0;
    }

    if (value == 0 || value > UINT_MAX) {
        return 0;
    }

    *pid = (unsigned int)value;

    return 1;
}