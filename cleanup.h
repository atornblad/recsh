#ifndef CLEANUP_H
#define CLEANUP_H

void clean_up(void);
void add_cleanup_func(void (*f)(void));

#endif
