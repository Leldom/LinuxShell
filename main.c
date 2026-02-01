#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

#define C_GREEN "\033[1;32m"
#define C_BLUE "\033[1;34m"
#define C_RESET "\033[0m"

void handleSignal(int sig)
{
    (void)sig;

    write(STDOUT_FILENO, "\n", 1);

}

int main(int argc, char *argv[]) 
{

    signal(SIGINT, handleSignal);

    char line[MAX_LINE];
    char hostName[256];
    char cwd[1024];
    char* args[MAX_ARGS];
    char* home = getenv("HOME");

    gethostname(hostName, sizeof(hostName));

    while (1) 
    {

        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s%s%s", C_GREEN, hostName, C_RESET);
            printf(":");
            
            if (home && strncmp(cwd, home, strlen(home)) == 0) {
                printf("%s~%s%s", C_BLUE, cwd + strlen(home), C_RESET);
            } else {
                printf("%s%s%s", C_BLUE, cwd, C_RESET);
            }
        }
        printf(" $ ");

        if (!fgets(line, MAX_LINE, stdin)) 
        {

            if(feof(stdin))
            {
                printf("\nLogout.\n");
                break;
            }

            clearerr(stdin);
            continue;
        }

        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0) continue;

        int i = 0;
        char* token = strtok(line, " ");

        while (token != NULL && i < MAX_ARGS - 1) 
        {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL; // For execvp

        if (strcmp(args[0], "exit") == 0) break; // Exit the shell

        if (strcmp(args[0], "cd") == 0) 
        {
            char* target = args[1];
            if (!target) target = home;

            if (chdir(target) != 0) 
            {
                perror("cd error");
            }
            continue;

        }
        const pid_t pid = fork();
        if (pid < 0) 
        {
            perror("fork error");

        } else if (pid == 0) 
        {
            execvp(args[0], args);
            perror("command not found");
            return 1; // kill child

        } else 
        {
            wait(NULL); // Wait for child process to finish
        }
    }
    return 0;
}
