#ifndef CONFIG_H
#define CONFIG_H

int load_config(int argc, char *argv[]);

struct configuration {
    const char *script_filename;
};

typedef struct configuration configuration;

extern configuration config;

#endif
