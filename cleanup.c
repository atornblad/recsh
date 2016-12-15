#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#include "cleanup.h"

static int functions_count = 0;
static int functions_array_length = 0;
static void (**functions)(void) = NULL;

void clean_up(void) {
    int i = 0;
    
    for (i = 0; i < functions_count; ++i) {
        (functions[i])();
    }
    
    free((void *)functions);
}

void add_cleanup_func(void (*f)(void)) {
    if (functions == NULL) {
        functions_array_length = 16;
        functions = malloc(functions_array_length * sizeof(void *));
    } else if (functions_count == functions_array_length) {
        functions_array_length += 16;
        functions = realloc(functions, functions_array_length * sizeof(void *));
    }
    
    functions[functions_count++] = f;
}
