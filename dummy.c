#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int i;
    int j;
    char progress[10];
    int plen;
    int left, right;
    int ms;
    
    fputs("How many milliseconds?\n", stdout);
    fflush(stdout);
    scanf("%d", &ms);
    
    for (i = 0; i <= 100; ++i) {
        sprintf(progress, " %d %% ", i);
        plen = strlen(progress);
        left = 29 - plen;
        right = left + plen;
        
        /*fputs("\x1b[G[", stdout);*/
        for (j = 0; j < i / 2; ++j) {
            if (j >= left && j < right) {
                fputc(progress[j - left], stdout);
            } else {
                fputc('=', stdout);
            }
        }
        for (j = i / 2; j < 50; ++j) {
            if (j == i / 2 && i < 100) {
                fputc('|', stdout);
            } else if (j >= left && j < right) {
                fputc(progress[j - left], stdout);
            } else {
                fputc(' ', stdout);
            }
        }
        fputc(']', stdout);
        fputc('\n', stdout);
        fflush(stdout);
        usleep(ms * 10);
    }
    fputc('\n', stdout);
    fflush(stdout);
    
    return 0;
}
