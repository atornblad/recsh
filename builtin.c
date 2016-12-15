#include <unistd.h>
#include <pwd.h>

#include "builtin.h"
#include "record.h"

int builtin_cd(int argc, const char *argv[]) {
    if (argc == 1) {
        struct passwd *pw = getpwuid(getuid());
        const char *homedir = pw->pw_dir;
        
        chdir(homedir);
    } else {
        chdir(argv[1]);
    }
    return 0;
}
