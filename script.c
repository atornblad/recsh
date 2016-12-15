#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "script.h"

static char *dot_net_console_foreground_colors[] = {
    "System.ConsoleColor.Black",
    "System.ConsoleColor.DarkRed",
    "System.ConsoleColor.DarkGreen",
    "System.ConsoleColor.DarkYellow",
    "System.ConsoleColor.DarkBlue",
    "System.ConsoleColor.DarkMagenta",
    "System.ConsoleColor.DarkCyan",
    "System.ConsoleColor.Gray",
    "System.ConsoleColor.DarkGray",
    "System.ConsoleColor.Red",
    "System.ConsoleColor.Green",
    "System.ConsoleColor.Yellow",
    "System.ConsoleColor.Blue",
    "System.ConsoleColor.Magenta",
    "System.ConsoleColor.Cyan",
    "System.ConsoleColor.White"
};

SCRIPT *create_script(const char *filename) {
    SCRIPT *result = (SCRIPT *)malloc(sizeof(SCRIPT));
    if (result == NULL) return NULL;
    result->filename = strdup(filename);
    result->file = fopen(filename, "w");
    if (result->file == NULL) {
        free(result->filename);
        free(result);
        return NULL;
        /* TODO: Correct error handling! */
    }
    
    fprintf(result->file, "Name: Default recsh output\n");
    fprintf(result->file, "Visualizer: ConsoleVisualizer.VisConsole, ConsoleVisualizer\n");

    return result;
}

void close_script(SCRIPT *script, int extra_render_frames) {
    if (script == NULL) return;
    script_Render(script, extra_render_frames);
    fflush(script->file);
    fclose(script->file);
    free(script->filename);
    free(script);
}

void script_Render(SCRIPT *script, const int frames) {
    if (frames)
        fprintf(script->file, "Render %d\n", frames);
}

void script_Clear(SCRIPT *script) {
    fprintf(script->file, "Call Clear\n");
}

void script_ResetColor(SCRIPT *script) {
    fprintf(script->file, "Call ResetColor\n");
}

void script_Write(SCRIPT *script, const char *string) {
    fprintf(script->file, "Call Write \"");
    while (*string) {
        switch (*string) {
            case '\n':
                fprintf(script->file, "\\n");
                break;
            case '\r':
                fprintf(script->file, "\\r");
                break;
            case '\"':
                fprintf(script->file, "\\\"");
                break;
            case '\\':
                fprintf(script->file, "\\\\");
                break;
            default:
                fprintf(script->file, "%c", *string);
        }
        ++string;
    }
    fprintf(script->file, "\"\n");
}

void script_Write_char(SCRIPT *script, const char character) {
    char array[2] = { 0, 0 };
    array[0] = character;
    script_Write(script, array);
}

void script_SetCursorVisible(SCRIPT *script, const int value) {
    fprintf(script->file, "Set CursorVisible %s\n", value ? "true" : "false");
}

void script_SetCursorSize(SCRIPT *script, const int value) {
    fprintf(script->file, "Set CursorSize %d\n", value);
}

void script_SetCursorLeft(SCRIPT *script, const int value) {
    fprintf(script->file, "Set CursorLeft %d\n", value);
}

void script_SetCursorTop(SCRIPT *script, const int value) {
    fprintf(script->file, "Set CursorTop %d\n", value);
}

void script_SetWindowLeft(SCRIPT *script, const int value) {
    fprintf(script->file, "Set WindowLeft %d\n", value);
}

void script_SetWindowTop(SCRIPT *script, const int value) {
    fprintf(script->file, "Set WindowTop %d\n", value);
}

void script_SetWindowWidth(SCRIPT *script, const int value) {
    fprintf(script->file, "Set WindowWidth %d\n", value);
}

void script_SetWindowHeight(SCRIPT *script, const int value) {
    fprintf(script->file, "Set WindowHeight %d\n", value);
}

void script_SetBufferWidth(SCRIPT *script, const int value) {
    fprintf(script->file, "Set BufferWidth %d\n", value);
}

void script_SetBufferHeight(SCRIPT *script, const int value) {
    fprintf(script->file, "Set BufferHeight %d\n", value);
}

void script_SetForegroundColor(SCRIPT *script, const int value) {
    fprintf(script->file, "Set ForegroundColor %s\n", dot_net_console_foreground_colors[value & 15]);
}

void script_SetBackgroundColor(SCRIPT *script, const int value) {
    fprintf(script->file, "Set BackgroundColor %s\n", dot_net_console_foreground_colors[value & 15]);
}
