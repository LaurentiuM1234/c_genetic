#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "error.h"

void printerr(const char *msg, int err_code)
{
        switch (err_code)
        {
        case -ELIB:
                fprintf(stderr, "[ERROR] %s: %s\n", msg, strerror(errno));
                break;
        case -EBADINPUT:
                fprintf(stderr, "[ERROR] %s: Bad input.\n", msg);
                break;
        case -EREAD:
                fprintf(stderr, "[ERROR] %s: Failed to read values.\n", msg);
                break;
        case -EBADARG:
                fprintf(stderr, "Usage: ./tema1_par <input_file> "
                "<gen_count> <thread_count>\n");
                break;
        
        default:
                fprintf(stderr, "[ERROR] Unknown error code.\n");
        }
}