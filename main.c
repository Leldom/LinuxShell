#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#define MAX_LINE 1024
#define MAX_ARGS 64

int main() {
    char line[MAX_LINE];
    char hostName[256];
    char* args[MAX_ARGS];

    gethostname(hostName, sizeof(hostName));

    while (1) {
        printf("\033[0;32m%s\033[0m @MyShell~> ", hostName);

        if (!fgets(line, MAX_LINE, stdin)) {
            break; // Exit on EOF or error
        }

        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0) continue;

        int i = 0;
        char* token = strtok(line, " ");

        while (token != NULL && i < MAX_ARGS - 1) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL; // For execvp

        if (strcmp(args[0], "exit") == 0) break; // Exit the shell

        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                fprintf(stderr, "cd: missing argument\n");
            } else {
                if (chdir(args[1]) != 0) {
                    perror("cd error");
                }
                continue;
            }
        }
        const pid_t pid = fork();
        if (pid < 0) {
            perror("fork error");
        } else if (pid == 0) {
            execvp(args[0], args);
            perror("command not found");
            return 1;
        } else {
            wait(NULL); // Wait for child process to finish
        }
    }
    return 0;
}
