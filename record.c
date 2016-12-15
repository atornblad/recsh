#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include "record.h"
#include "cleanup.h"
#include "script.h"

static SCRIPT *output;
static int cursor_left;
static int cursor_top;
static int terminal_width;
static int terminal_height;

static void cleanup_console() {
    struct termios oldt;
    struct termios newt;
    
    #ifdef DEBUG_VERBOSE
    fprintf(stderr, "cleanup_console\n");
    #endif
    
    tcgetattr(STDIN_FILENO, &oldt);
    
    newt = oldt;
    newt.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    close_script(output, 50);
}

void setup_console(void) {
    struct termios oldt;
    struct termios newt;
    struct winsize ws;
    
    tcgetattr(STDIN_FILENO, &oldt);
    
    fputs("\033[H\033[J", stdout);
    
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    add_cleanup_func(cleanup_console);
    
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    terminal_height = ws.ws_row;
    terminal_width = ws.ws_col;
    
    output = create_script("recsh.script");
    
    script_SetBufferHeight(output, terminal_height);
    script_SetBufferWidth(output, terminal_width);
    script_SetCursorLeft(output, 0);
    script_SetCursorTop(output, 0);
    script_SetCursorSize(output, 25);
    script_SetCursorVisible(output, 1);
    script_SetWindowHeight(output, terminal_height);
    script_SetWindowWidth(output, terminal_width);
    script_SetWindowLeft(output, 0);
    script_SetWindowTop(output, 0);
    script_Clear(output);
    
    cursor_left = 0;
    cursor_top = 0;
}

#define escape_numbers_count 16

void rputc(int character) {
    static int mode = 0;
    static double mlast = 0.0;
    struct timeval tv;
    double mnow;
    int frames_since_last;
    static int escape_numbers[escape_numbers_count];
    static int current_escape_number_index = 0;
    static int foreground = 7;
    static int background = 0;
    int i;
    
    if (mlast == 0.0) {
        /* Don't use this around midnight, or daylight savings time */
        gettimeofday(&tv, NULL);
        mlast = tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
    }
    
    gettimeofday(&tv, NULL);
    mnow = tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
    
    frames_since_last = (int)((mnow - mlast) / 33.33333333);
    
    if (frames_since_last > 0) {
        script_Render(output, frames_since_last);
        mlast += 33.33333333 * frames_since_last;
    }
    
    #ifdef DEBUG_VERBOSE
    fprintf(stderr, "Character %d : '%c'\n", character, (char)character);
    #else
    fputc(character, stdout);
    fflush(stdout);
    #endif

    if (mode == 0) {
        switch (character) {
            case '\b':
                --cursor_left;
                if (cursor_left < 0) {
                    cursor_left = terminal_width - 1;
                    --cursor_top;
                    if (cursor_top < 0) {
                        /* What to do here!?? */
                        ++cursor_top;
                    }
                    script_SetCursorTop(output, cursor_top);
                }
                script_SetCursorLeft(output, cursor_left);
                break;
            case '\n':
                cursor_left = 0;
                ++cursor_top;
                while (cursor_top >= terminal_height) {
                    /* What to do here!?? */
                    script_ScrollUp(output, terminal_width, terminal_height, foreground, background);
                    --cursor_top;
                }
                script_SetCursorLeft(output, cursor_left);
                script_SetCursorTop(output, cursor_top);
                break;
            case '\x1b':
                mode = 1;
                for (i = 0; i < escape_numbers_count; ++i) {
                    escape_numbers[i] = 0;
                }
                current_escape_number_index = 0;
                break;
            default:
                ++cursor_left;
                if (cursor_left >= terminal_width) {
                    cursor_left = 0;
                    cursor_top++;
                    if (cursor_top >= terminal_height) {
                        script_ScrollUp(output, terminal_width, terminal_height, foreground, background);
                        --cursor_top;
                    }
                    script_SetCursorLeft(output, cursor_left);
                    script_SetCursorTop(output, cursor_top);
                }
                script_Write_char(output, (char)character);
        }
    } else if (mode == 1) {
        if (character == '[') {
            mode = 2;
        } else {
            mode = 0;
        }
    } else if (mode == 2) {
        if (isalpha(character)) {
            int bold = 0;
            switch (character) {
                /* CUU - Cursor Up */
                case 'A':
                    cursor_top -= escape_numbers[0] ? escape_numbers[0] : 1;
                    while (cursor_top < 0) {
                        script_ScrollDown(output, terminal_width, terminal_height, foreground, background);
                        ++cursor_top;
                    }
                    script_SetCursorTop(output, cursor_top);
                    break;
                /* CUU - Cursor Down */
                case 'B':
                    cursor_top += escape_numbers[0] ? escape_numbers[0] : 1;
                    while (cursor_top >= terminal_height) {
                        script_ScrollUp(output, terminal_width, terminal_height, foreground, background);
                        cursor_top --;
                    }
                    script_SetCursorTop(output, cursor_top);
                    break;
                /* CUF - Cursor Forward */
                case 'C':
                    cursor_left += escape_numbers[0] ? escape_numbers[0] : 1;
                    while (cursor_left >= terminal_width) {
                        cursor_left -= terminal_width;
                        cursor_top ++;
                    }
                    while (cursor_top >= terminal_height) {
                        script_ScrollUp(output, terminal_width, terminal_height, foreground, background);
                        cursor_top --;
                    }
                    script_SetCursorLeft(output, cursor_left);
                    script_SetCursorTop(output, cursor_top);
                    break;
                /* CUB - Cursor Back */
                case 'D':
                    cursor_left -= escape_numbers[0] ? escape_numbers[0] : 1;
                    while (cursor_left < 0) {
                        cursor_left += terminal_width;
                        cursor_top --;
                    }
                    while (cursor_top < 0) {
                        script_ScrollDown(output, terminal_width, terminal_height, foreground, background);
                        cursor_top ++;
                    }
                    script_SetCursorLeft(output, cursor_left);
                    script_SetCursorTop(output, cursor_top);
                    break;
                /* CHA - Cursor Hosizontal Absolute */
                case 'G':
                    cursor_left = (escape_numbers[0] ? escape_numbers[0] : 1) - 1;
                    script_SetCursorLeft(output, cursor_left);
                    break;
                /* CUP - Cursor Position */
                case 'H':
                    cursor_top = (escape_numbers[0] ? escape_numbers[0] : 1) - 1;
                    cursor_left = (escape_numbers[1] ? escape_numbers[1] : 1) - 1;
                    script_SetCursorLeft(output, cursor_left);
                    script_SetCursorTop(output, cursor_top);
                    break;
                /* ED - Erase Display */
                case 'J':
                    /* TODO: E[0J : Clear from cursor to end of screen
                     * TODO: E[1J : Clear from cursor to beginning of screen
                     * TODO: E[2J : Clear entire screen */
                     script_Clear(output);
                    break;
                /* SGR - Select Graphic Rendition */
                case 'm':
                    {
                        int oldfg = foreground;
                        int oldbg = background;
                        for (i = 0; i <= current_escape_number_index; ++i) {
                            int n = escape_numbers[i];
                            #ifdef DEBUG_VERBOSE
                            fprintf(stderr, "SGR %d\n", n);
                            #endif
                            if (n == 0) {
                                bold = 0;
                                foreground = 7;
                                background = 0;
                            } else if (n == 1) {
                                bold = 1;
                                foreground |= 8;
                                background |= 8;
                            } else if (n >= 30 && n <= 37)
                                foreground = escape_numbers[i] - 30 + (bold * 8);
                            else if (n == 39)
                                foreground = 7 + (bold * 8);
                            else if (n >= 40 && n <= 47)
                                background = escape_numbers[i] - 40 + (bold * 8);
                            else if (n == 49)
                                background = 7 + (bold * 8);
                            else if (n >= 90 && n <= 97)
                                foreground = escape_numbers[i] - 90 + 8;
                            else if (n >= 100 && n <= 107)
                                background = escape_numbers[i] - 100 + 8;
                        }
                        if (oldbg != background)
                            script_SetBackgroundColor(output, background);
                        if (oldfg != foreground)
                            script_SetForegroundColor(output, foreground);
                    }
                    break;
            }
            mode = 0;
        } else if (isdigit(character)) {
            escape_numbers[current_escape_number_index] = (escape_numbers[current_escape_number_index] * 10) + (character - '0');
        } else if (character == ';') {
            ++current_escape_number_index;
            if (current_escape_number_index == 16) --current_escape_number_index;
        }
    }
}

void rputs(const char *str) {
    int i = 0;

    #ifdef DEBUG_VERBOSE
    fprintf(stderr, "Calling rputs...\n");
    #endif

    if (str == NULL) return;
    
    while (str[i] != '\0') {
        rputc(str[i]);
        ++i;
    }
}

void rprintf(const char *format, ...) {
    int i = 0;
    va_list arguments;
    const char *string_argument;

    #ifdef DEBUG_VERBOSE
    fprintf(stderr, "Calling rprintf...\n");
    #endif
    
    va_start(arguments, format);

    if (format == NULL) return;
    
    while (format[i] != '\0') {
        if (format[i] == '%') {
            ++i;
            switch (format[i]) {
                case 's':
                    string_argument = va_arg(arguments, const char *);
                    rputs(string_argument);
                    break;
                default:
                    fprintf(stderr, "rprintf: Not yet implemented: %%%c\n", format[i]);
            }
            ++i;
        } else {
            rputc(format[i]);
            ++i;
        }
    }
    
    va_end(arguments);
}
