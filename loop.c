#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <poll.h>

#include "loop.h"
#include "error.h"
#include "record.h"
#include "builtin.h"

#define read_user_input_buffer_size 256
#define split_user_input_array_size 16
#define cwd_buffer_size 1024
#define GETLOGIN_R_SUCCESS 0

static void print_prompt();
static int read_user_input(char **user_input);
static int split_user_input(char *user_input, const char ***user_input_parts, int *parts_count);
static const char *get_user_name();

static const char *builtin_names[] = {
    "cd",
    NULL
};

static BUILTIN_FUNC builtin_funcs[] = {
    builtin_cd
};

static int can_start(const char *command) {
    if (strstr(command, "/") != NULL) {
        return (access(command, X_OK) != -1);
    } else {
        /* TODO: Check PATH */
        return 1;
    }
}

int main_loop(void) {
    char *user_input;
    const char **user_input_parts;
    int parts_count;
    int error_code;
    int do_exit = 0;
    int i;
    
    do {
        print_prompt();
        
        error_code = read_user_input(&user_input);
        
        if (error_code == SUCCESS) {
            #ifdef DEBUG_VERBOSE
            printf("You typed: \"%s\"\n", user_input);
            #endif
            error_code = split_user_input(user_input, &user_input_parts, &parts_count);
            
            if (error_code == SUCCESS) {
                #ifdef DEBUG_VERBOSE
                printf("Your input was split into %d parts:\n", parts_count);
                for (i = 0; i < parts_count; ++i) {
                    printf("    argv[%d]: %s\n", i, user_input_parts[i]);
                }
                #endif
                
                if (parts_count == 1 && strcmp("exit", user_input_parts[0]) == 0) {
                    rputs("\x1b[91mExit\x1b[0m\n");
                    do_exit = 1;
                } else if (parts_count >= 1) {
                    i = 0;
                    while (builtin_names[i]) {
                        if (strcmp(builtin_names[i], user_input_parts[0]) == 0) {
                            (builtin_funcs[i])(parts_count, user_input_parts);
                            break;
                        } else {
                            ++i;
                        }
                    }
                    if (!builtin_names[i]) {
                        if (can_start(user_input_parts[0])) {
                            pid_t pid, wpid;
                            int status;
                            int stdin_pipe[2];
                            int stdout_pipe[2];
                            int stderr_pipe[2];
                            
                            pipe(stdin_pipe);
                            pipe(stdout_pipe);
                            pipe(stderr_pipe);
                            
                            pid = fork();
                            
                            if (pid == 0) {
                                /* Child process! */
                                /* Close WRITE end of stdin, and READ ends of stdout and stderr */
                                close(stdin_pipe[1]);
                                close(stdout_pipe[0]);
                                close(stderr_pipe[0]);
                                /* Change child's streams to use pipes */
                                dup2(stdin_pipe[0], STDIN_FILENO);
                                dup2(stdout_pipe[1], STDOUT_FILENO);
                                dup2(stderr_pipe[1], STDERR_FILENO);
                                if (execvp(user_input_parts[0], (char * const *)user_input_parts) == -1) {
                                    exit(EXIT_FAILURE);
                                }
                            } else {
                                char buffer[8192];
                                int stdin_eof = 0;
                                int stdout_eof = 0;
                                int stderr_eof = 0;
                                struct pollfd fds[4];
                                int pollret;
                                int readret;
                                
                                /* Parent process */
                                /* Close READ end of stdin, and WRITE ends of stdout and stderr */
                                close(stdin_pipe[0]);
                                close(stdout_pipe[1]);
                                close(stderr_pipe[1]);
                                
                                fds[0].fd = STDIN_FILENO;
                                fds[0].events = POLLIN;
                                fds[1].fd = stdout_pipe[0];
                                fds[1].events = POLLIN | POLLHUP;
                                fds[2].fd = stderr_pipe[0];
                                fds[2].events = POLLIN | POLLHUP;

                                #ifdef DEBUG_VERBOSE
                                fprintf(stderr, "Forward pipes\n");
                                #endif
                                while (!stdout_eof && !stderr_eof) {
                                    int found_any = 0;
                                    
                                    if (!stdin_eof) {
                                        pollret = poll(&fds[0], 1, 0);
                                        
                                        /* TODO: Figure out how to handle eof for stdin */
                                        if (pollret == 1) {
                                            readret = read(STDIN_FILENO, buffer, sizeof(buffer));
                                            write(stdin_pipe[1], buffer, readret);
                                            for (i = 0; i < readret; ++i) {
                                                rputc(buffer[i]);
                                            }
                                            found_any = 1;
                                        }
                                    }
                                    
                                    if (!stdout_eof) {
                                        pollret = poll(&fds[1], 1, 0);
                                        
                                        if (fds[1].revents & POLLHUP) {
                                            readret = read(stdout_pipe[0], buffer, sizeof(buffer));
                                            for (i = 0; i < readret; ++i) {
                                                rputc(buffer[i]);
                                            }
                                            stdout_eof = 1;
                                            close(stdout_pipe[0]);
                                        } else if (pollret == 1) {
                                            readret = read(stdout_pipe[0], buffer, sizeof(buffer));
                                            for (i = 0; i < readret; ++i) {
                                                rputc(buffer[i]);
                                            }
                                            found_any = 1;
                                        }
                                    }
                                    
                                    if (!stderr_eof) {
                                        pollret = poll(&fds[2], 1, 0);
                                        
                                        if (fds[2].revents & POLLHUP) {
                                            readret = read(stderr_pipe[0], buffer, sizeof(buffer));
                                            for (i = 0; i < readret; ++i) {
                                                rputc(buffer[i]);
                                            }
                                            stderr_eof = 1;
                                            close(stderr_pipe[0]);
                                        } else if (pollret == 1) {
                                            readret = read(stderr_pipe[0], buffer, sizeof(buffer));
                                            for (i = 0; i < readret; ++i) {
                                                rputc(buffer[i]);
                                            }
                                            found_any = 1;
                                        }
                                    }
                                    
                                    if (!found_any) {
                                        usleep(5000);
                                    }
                                }

                                #ifdef DEBUG_VERBOSE
                                fprintf(stderr, "Close remaining pipes\n");
                                #endif
                                if (!stdin_eof) {
                                    close(stdin_pipe[1]);
                                }
                                if (!stdout_eof) {
                                    close(stdout_pipe[0]);
                                }
                                if (!stderr_eof) {
                                    close(stderr_pipe[0]);
                                }
                                
                                /* Wait for child to finish */
                                #ifdef DEBUG_VERBOSE
                                fprintf(stderr, "Parent process waitpid\n");
                                #endif
                                do {
                                    wpid = waitpid(pid, &status, WUNTRACED);
                                    if (wpid == -1) exit (EXIT_FAILURE);
                                } while (!WIFEXITED(status) && !WIFSIGNALED(status));
                            }
                        } else {
                            rprintf("%s: Command not found\n", user_input_parts[0]);
                        }
                    }
                }
                
                free((void *)user_input_parts);
            }
            
            free((void *)user_input);
        }
    } while (error_code == SUCCESS && !do_exit);
    
    return error_code;
}

static void print_prompt() {
    char cwd[cwd_buffer_size];
    
    rprintf("\x1b[93m%s", get_user_name());
    
    if (getcwd(cwd, cwd_buffer_size) != NULL) {
        struct passwd *pw = getpwuid(getuid());
        const char *homedir = pw->pw_dir;
        int homedirlen = strlen(homedir);
        
        rprintf("\x1b[96m ");
        
        if (strncmp(homedir, cwd, homedirlen) == 0) {
            rputc('~');
            rputs(&cwd[homedirlen]);
        } else {
            rputs(cwd);
        }
    }
    
    rprintf("\x1b[0m $ ");
}

static int read_user_input(char **user_input) {
    int current_buffer_size = read_user_input_buffer_size;
    int position = 0;
    char *buffer = (char *)malloc(sizeof(char) * current_buffer_size);
    int input_char;
    
    if (buffer == NULL) {
        return OUT_OF_MEMORY;
    }
    
    input_char = fgetc(stdin);
    while (input_char != EOF && input_char != '\0' && input_char != '\n') {
        if (input_char == '\x7f') {
            if (position > 0) {
                rputs("\b \b");
                --position;
                buffer[position] = '\0';
            }
            input_char = fgetc(stdin);
            continue;
        }
        rputc(input_char);
        buffer[position] = (char)input_char;
        ++position;
        
        if (position >= current_buffer_size - 1) {
            current_buffer_size += read_user_input_buffer_size;
            buffer = (char *)realloc((void *)buffer, current_buffer_size);
            
            if (buffer == NULL) {
                return OUT_OF_MEMORY;
            }
        }
        
        input_char = fgetc(stdin);
    }
    
    buffer[position] = '\0';
    rputc('\n');
    
    *user_input = (char *)buffer;
    return SUCCESS;
}

static int split_user_input(char *user_input, const char ***user_input_parts, int *parts_count) {
    int array_size = split_user_input_array_size;
    const char **array = (const char **)malloc(array_size * sizeof(char *));
    int position = 0;
    int index = 0;
    
    if (array == NULL) {
        return OUT_OF_MEMORY;
    }
    
    while (user_input[position] != '\0') {
        /* Skip white-space */
        while (user_input[position] == ' ' ||
               user_input[position] == '\t') {
            ++position;
        }
        
        if (user_input[position] != '\0') {
            if (user_input[position] == '"') {
                ++position;
                array[index++] = &user_input[position];
                while (user_input[position] != '\0' && user_input[position] != '"') {
                    ++position;
                }
            } else {
                array[index++] = &user_input[position];
                while (user_input[position] != '\0' && user_input[position] != ' ' && user_input[position] != '\t') {
                    ++position;
                }
            }
            if (user_input[position] != '\0') {
                user_input[position] = '\0';
                ++position;
            }
        }
        
        if (index >= array_size - 1) {
            array_size += split_user_input_array_size;
            array = (const char **)realloc(array, array_size * sizeof(char *));
        }
    }
    
    array[index] = NULL;
    *user_input_parts = array;
    *parts_count = index;
    
    return SUCCESS;
}

static const char *get_user_name() {
    uid_t uid = geteuid();
    
    struct passwd *pw = getpwuid(uid);
    
    if (pw != NULL) {
        return pw->pw_name;
    }
    
    return "";
}

