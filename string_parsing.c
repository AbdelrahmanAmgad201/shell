#include "string_parsing.h"

void print_string_array(char *arr[]) {
    int i = 0;
    while (arr[i] != NULL) {  
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
