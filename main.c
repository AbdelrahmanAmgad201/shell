#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#include "string_parsing.h"

#define MAXSIZE 1024

int check_instruction(char **args) {
    // returns the operation type 
    if (strcmp(args[0], "exit") == 0){
        return 0;
    }else if (strcmp(args[0], "cd") == 0 || 
             strcmp(args[0], "echo") == 0 || 
             strcmp(args[0], "export") == 0){
        return 1;
    }else{
        return 2;
    }    
    return 3;
}

void execute_external(char **args) {
    int background = 0;
    int i = 0;
    
    // extracts the & symbol
    while (args[i] != NULL) i++;
    if (i > 0 && strcmp(args[i - 1], "&") == 0) {
        background = 1;
        args[i - 1] = NULL; 
    }

    // execution
    int pid = fork();
    if (pid < 0) {
        printf("Error executing command\n");
    } else if (pid == 0) {
        execvp(args[0], args);
        perror("execvp failed");
        exit(1);
    } else if(!background){
            waitpid(pid, NULL, 0);
    }
}

void execute_built_in(char **args) {
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            printf("Missing argument\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd failed");
            }
        }
    } else if (strcmp(args[0], "echo") == 0) {
        printf("\Echo not implemented \n");
    } else if (strcmp(args[0], "export") == 0) {
        printf("export command not implemented yet\n");
    } else {
        printf("Unknown  command\n");
    }
}

void setup_env(){
    chdir(getenv("HOME"));
}

void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
    printf("Child process reaped\n");
}


void shell() {
    char input[MAXSIZE];
    char **args;
    signal(SIGCHLD, sigchld_handler);
    setup_env();
    // test_child_processes();

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
