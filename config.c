#include <stdio.h>
#include "config.h"
#include "error.h"

configuration config;

int load_config(int argc, char *argv[]) {
    int i;
    char *output_filename;
    
    output_filename = NULL;
    
    for (i = 1; i < argc; ++i) {
        if (output_filename == NULL) {
            output_filename = argv[i];
        } else {
            return BAD_SHELL_ARGUMENTS;
        }
    }
    
    if (output_filename == NULL) {
        output_filename = "recsh.script";
    }
    
    config.script_filename = output_filename;
    
    return SUCCESS;
}
