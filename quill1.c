#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define MAX_BUFFER 10000
#define MAX_COMMAND 256

struct termios orig_termios;
char text_buffer[MAX_BUFFER];
int buffer_len = 0;
int in_command_mode = 0;
char command_buffer[MAX_COMMAND];

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON); 
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void print_header() {
    write(STDOUT_FILENO, "---------------------------------------------------------\n", 58);
    write(STDOUT_FILENO, " WELCOME TO QUILL TEXT EDITOR - PRESS CTRL C TO BREAK\n", 54);
    write(STDOUT_FILENO, " QUILL - YOU ARE ON INSERT MODE PRESS ESC TO CHANGE\n", 53);
    write(STDOUT_FILENO, "---------------------------------------------------------\n", 58);
}

void save_file(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("UNABLE TO SAVE FILE");
        return;
    }
    fwrite(text_buffer, 1, buffer_len, fp);
    fclose(fp);
    printf("\nSAVED TO: %s\n", filename);
}

void handle_command() {
    if (strncmp(command_buffer, ":quill save ", 12) == 0) {
        char* filename = command_buffer + 12;
        save_file(filename);
    } else if (strcmp(command_buffer, ":clear") == 0) {
        buffer_len = 0;
        memset(text_buffer, 0, sizeof(text_buffer));
        write(STDOUT_FILENO, "\033[2J\033[H", 7);
        print_header();
    } else {
        printf("\nUNKNOWN COMMAND: %s\n", command_buffer);
    }
    memset(command_buffer, 0, sizeof(command_buffer));
    in_command_mode = 0;
    printf("\n");
}

int main() {
    enable_raw_mode();
    print_header();

    while (1) {
        char c;
        read(STDIN_FILENO, &c, 1);

        if (in_command_mode) {
            if (c == '\n') {
                handle_command();
            } else if (c == 127) {
                int len = strlen(command_buffer);
                if (len > 0) {
                    command_buffer[len - 1] = '\0';
                    write(STDOUT_FILENO, "\b \b", 3);
                }
            } else if (strlen(command_buffer) < MAX_COMMAND - 1) {
                int len = strlen(command_buffer);
                command_buffer[len] = c;
                command_buffer[len + 1] = '\0';
                write(STDOUT_FILENO, &c, 1);
            }
        } else {
            if (c == 27) {
                in_command_mode = 1;
                memset(command_buffer, 0, sizeof(command_buffer));
                command_buffer[0] = ':'; 
                command_buffer[1] = '\0';
                write(STDOUT_FILENO, "\n\n---------------------------------------------------------\n:", 61);
            } else if (c == 127) {
                if (buffer_len > 0) {
                    buffer_len--;
                    write(STDOUT_FILENO, "\b \b", 3);
                }
            } else if (buffer_len < MAX_BUFFER - 1) {
                text_buffer[buffer_len++] = c;
                write(STDOUT_FILENO, &c, 1);
            }
        }
    }

    return 0;
}
