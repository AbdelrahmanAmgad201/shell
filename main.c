#include <unistd.h>
#include "string_parsing.h"

#define MAXSIZE 1024
#define MAXARGS 50 



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
        return 2;  //use execvp

    }
}

void shell() {
    char input[MAXSIZE];
    char **args;

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

        switch (instruction_type)
        {
        case 0:
            exit(0);
            break;
        
        case 1:
            // built in commands;
            break;
        case 2:
            // execute
            break;
        default:
            printf("unknown command, try again");
        
        execvp(args[0], args);
        // Free allocated memory
        
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