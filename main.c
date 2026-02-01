#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_ARGS 64

#define C_GREEN   "\001\033[1;32m\002"
#define C_BLUE    "\001\033[1;34m\002"
#define C_MAGENTA "\001\033[1;35m\002"
#define C_RESET   "\001\033[0m\002"

void handle_signal(int sig)
{
    (void)sig;

    write(STDOUT_FILENO, "\n", 1);

    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}

int get_git_branch(char *branch_buffer, size_t size)
{
    FILE *fp = popen("git branch --show-current 2> /dev/null", "r");

    if(fp == NULL)
    {
        return 0;
    }

    if(fgets(branch_buffer, size, fp) != NULL)
    {
        branch_buffer[strcspn(branch_buffer, "\n")] = 0;

        pclose(fp);
        return(strlen(branch_buffer) > 0);
    }
    pclose(fp);
    return(strlen(branch_buffer) > 0);
}

void build_prompt(char *buffer, size_t size)
{
    char hostName[256];
    char cwd[1024];
    char gitBranch[64] = {0};

    char *home = getenv("HOME");
    char *user = getenv("USER");

    gethostname(hostName, sizeof(hostName));

    char *dot = strchr(hostName, '.');
    if(dot)
    {
        *dot = '\0';
    }

    if(!user) user = "user";

    if(getcwd(cwd, sizeof(cwd)) != NULL)
    {
        char* path_to_show = cwd;

        if (home && strncmp(cwd, home, strlen(home)) == 0)
        {
            path_to_show = cwd + strlen(home); 
        
            if (get_git_branch(gitBranch, sizeof(gitBranch))) 
            {
                 snprintf(buffer, size, "%s%s@%s%s:%s~%s%s (%s%s%s) $ ", 
                    C_GREEN, user, hostName, C_RESET,
                    C_BLUE, path_to_show, C_RESET,
                    C_MAGENTA, gitBranch, C_RESET);
            } else 
            {
                snprintf(buffer, size, "%s%s@%s%s:%s~%s%s $ ", 
                    C_GREEN, user, hostName, C_RESET,
                    C_BLUE, path_to_show, C_RESET);
            }
        } else
        {
            if (get_git_branch(gitBranch, sizeof(gitBranch))) 
            {
                 snprintf(buffer, size, "%s%s@%s%s:%s%s%s (%s%s%s) $ ", 
                    C_GREEN, user, hostName, C_RESET,
                    C_BLUE, cwd, C_RESET,
                    C_MAGENTA, gitBranch, C_RESET);
            } else 
            {
                snprintf(buffer, size, "%s%s@%s%s:%s%s%s $ ", 
                    C_GREEN, user, hostName, C_RESET,
                    C_BLUE, cwd, C_RESET);
            }
        }
    }else
    {
        snprintf(buffer, size, "myshell> ");
    }
}
int tokenize(char *input, char **args)
{
    int i = 0;
    char *ptr = input;

    while(*ptr && i < MAX_ARGS -1)
    {
        while(*ptr == ' ' || *ptr == '\t') ptr++;

        if(*ptr == '\0') break;

        if(*ptr == '"')
        {
            ptr ++;
            args[i++] = ptr;

            char *end = strchr(ptr, '"');
            if(end)
            {
                *end = '\0';
                ptr = end++;
            } else {
                break;
            }
        } else
        {
            args[i++] = ptr;

            while(*ptr && *ptr != ' ' && *ptr != '\t') ptr++;

            if(*ptr)
            {
                *ptr = '\0';
                ptr++;
            }
        }
    }
    args[i] = NULL;
    return i;
}
int main() 
{

    signal(SIGINT, handle_signal);

    char *input;
    char prompt[2048];
    char *args[MAX_ARGS];


    while (1) 
    {

        build_prompt(prompt, sizeof(prompt));

        input = readline(prompt);

        if(!input)
        {
            printf("\nLogout\n");
            break;
        }

        if(strlen(input) > 0)
        {
            add_history(input);
        }

        tokenize(input, args);

        if(args[0] == NULL)
        {
            free(input);
            continue;
        }

        if(args[0] == NULL)
        {
            free(input);
            continue;
        }

        if(strcmp(args[0], "exit") == 0)
        {
            free(input);
            break;
        }

        if(strcmp(args[0], "cd") == 0)
        {
            char *target = args[1] ? args[1] : getenv("HOME");
            if(chdir(target) != 0)
            {
                perror("cd error");
            }
            free(input);
            continue;
        }

        const pid_t pid = fork();
        if(pid < 0)
        {
            perror("fork error");
        } else if(pid == 0)
        {
            execvp(args[0], args);
            perror("command not found");
            exit(1);
        } else
        {
            wait(NULL);
        }
        free(input);
    }
    return 0;
}
