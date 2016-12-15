#ifndef RECORD_H
#define RECORD_H

int setup_console(void);
void rputc(int character);
void rputs(const char *str);
void rprintf(const char *format, ...);

#endif
