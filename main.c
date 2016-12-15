#include "config.h"
#include "cleanup.h"
#include "loop.h"
#include "error.h"
#include "record.h"

int main(int argc, char *argv[]) {
    int result;
    
    result = load_config(argc, argv);
    
    if (result == SUCCESS) {
        result = setup_console();
    }
    
    if (result == SUCCESS) {
        result = main_loop();
    }
    
    clean_up();
    
    if (result != SUCCESS) {
        show_error(result);
    }
    
    return result;
}
