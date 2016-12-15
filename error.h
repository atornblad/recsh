#ifndef ERROR_H
#define ERROR_H

enum ErrorCode {
    SUCCESS,
    OUT_OF_MEMORY,
    BAD_SHELL_ARGUMENTS,
    SCRIPT_FILE_CREATION_FAILED
};

void show_error(int result);

#endif
