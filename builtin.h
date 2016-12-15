#ifndef BUILTIN_H
#define BUILTIN_H

typedef int (*BUILTIN_FUNC) (int, const char * []);

int builtin_cd(int argc, const char *argv[]);

#endif
