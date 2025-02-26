#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXSIZE 1024
#define MAXARGS 50 

void print_string_array(char *arr[]) {
    int i = 0;
    while (arr[i] != NULL) {  // Loop until we reach NULL
        printf("Element [%d]: %s\n", i, arr[i]);
        i++;
    }
}



char **parse_input(char *input) {
    int space_count = 0;
    for (int i = 0; i < strlen(input); i++) {
        if (input[i] == ' ') space_count++;
    }

    // Allocate memory for the array of pointers (args array)
    char **args = (char **)malloc((space_count + 2) * sizeof(char *));
    if (args == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    int i = 0;
    char *token = strtok(input, " ");
    while (token != NULL) {
        args[i] = strdup(token);  // Duplicate token (allocates memory)
        token = strtok(NULL, " ");
        i++;
    }
    args[i] = NULL;  // Null-terminate the array

    return args;
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