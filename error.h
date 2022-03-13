#ifndef _ERROR_H
#define _ERROR_H

enum err_code {
        ELIB = 1,
        EBADINPUT = 2,
        EREAD = 3,
        EBADARG = 4
};

/*
 * Description: Print error message corresponding to given error code.
 * 
 * Parameters: - msg = message to be printed.
 *             - err_code = the error code to be interpreted.
 */
void printerr(const char *msg, int err_code);


#endif // _ERROR_H