#ifndef SCRIPT_H
#define SCRIPT_H

struct script_output {
    char *filename;
    FILE *file;
};

typedef struct script_output SCRIPT;

SCRIPT *create_script(const char *filename);
void close_script(SCRIPT *script, int extra_render_frames);

void script_Clear(SCRIPT *script);
void script_ResetColor(SCRIPT *script);
void script_Render(SCRIPT *script, const int frames);
void script_Write(SCRIPT *script, const char *string);
void script_Write_char(SCRIPT *script, const char character);
void script_SetCursorTop(SCRIPT *script, const int value);
void script_SetCursorLeft(SCRIPT *script, const int value);
void script_SetCursorSize(SCRIPT *script, const int value);
void script_SetCursorVisible(SCRIPT *script, const int value);
void script_SetBufferHeight(SCRIPT *script, const int value);
void script_SetBufferWidth(SCRIPT *script, const int value);
void script_SetWindowHeight(SCRIPT *script, const int value);
void script_SetWindowWidth(SCRIPT *script, const int value);
void script_SetWindowTop(SCRIPT *script, const int value);
void script_SetWindowLeft(SCRIPT *script, const int value);
void script_SetForegroundColor(SCRIPT *script, const int value);
void script_SetBackgroundColor(SCRIPT *script, const int value);

#endif
