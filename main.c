#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "string_parsing.h"

#define MAXSIZE 1024

int check_instruction(char **args) {
    if (strcmp(args[0], "exit") == 0) {
        return 0;  // Exit command
    } 
    else if (strcmp(args[0], "cd") == 0 || 
             strcmp(args[0], "echo") == 0 || 
             strcmp(args[0], "export") == 0) {
        return 1;  // Built-in command
    } 
    else {
        return 2;  // Use execvp
    }
    return 3;
}

void execute_external(char **args) {
    int pid = fork();
    if (pid < 0) {
        printf("Error executing command\n");
    } else if (pid == 0) {
        printf("Hello from child process (before execvp)\n");
        execvp(args[0], args);
        perror("execvp failed");  // Print error if execvp fails
        exit(1);
    } else {
        printf("Hello from parent\n");
        waitpid(pid, NULL, 0);  // Correct waitpid usage
    }
}

void execute_built_in(char **args) {
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            printf("cd: missing argument\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd failed");
            }
        }
    } else if (strcmp(args[0], "echo") == 0) {
        for (int i = 1; args[i] != NULL; i++) {
            printf("%s ", args[i]);
        }
        printf("\n");
    } else if (strcmp(args[0], "export") == 0) {
        // Placeholder for future export functionality
        printf("export command not implemented yet\n");
    } else {
        printf("Unknown built-in command\n");
    }
}

void shell() {
    char input[MAXSIZE];
    char **args;
    chdir(getenv("HOME"));
    printf("WELCOME TO MY SHELL\n");

    while (1) {
        printf("> ");  // Shell prompt

        if (scanf(" %[^\n]%*c", input) != 1) {
            printf("Error reading input.\n");
            continue;
        }

        args = parse_input(input);
        print_string_array(args);
        int instruction_type = check_instruction(args);

        switch (instruction_type) {
            case 0:
                exit(0);
                break;
            case 1:
                execute_built_in(args);
                break;
            case 2:
                execute_external(args);
                break;
            default:
                printf("Unknown command, try again\n");
        }
        
        for (int i = 0; args[i] != NULL; i++) {
            free(args[i]);
        }
        free(args);
    }
}

int main() {
    shell();
    return 0;
}
