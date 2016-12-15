#include <stdio.h>
#include "error.h"

void show_error(int result) {
    switch (result) {
        case OUT_OF_MEMORY:
            fprintf(stderr, "Out of memory error");
            break;
        case BAD_SHELL_ARGUMENTS:
            fprintf(stderr, "Usage: recsh [output-filename]");
            break;
        case SCRIPT_FILE_CREATION_FAILED:
            fprintf(stderr, "Could not create script file");
            break;
        default:
            fprintf(stderr, "Unknown error occurred");
    }
}
