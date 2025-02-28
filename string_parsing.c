#include "string_parsing.h"

void print_string_array(char *arr[]) {
    int i = 0;
    while (arr[i] != NULL) {  
        printf("Element [%d]: %s\n", i, arr[i]);
        i++;
    }
}

char **parse_input(char *input) {
    // Create a working copy of the input
    char *input_copy = strdup(input);
    if (input_copy == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    int len = strlen(input_copy);
    
    // First pass: count potential tokens to allocate memory
    int max_tokens = 0;
    int i = 0;
    int in_quotes = 0;
    char quote_char = '\0';
    
    while (i < len) {
        // Handle start of quotes
        if ((input_copy[i] == '"' || input_copy[i] == '\'') && !in_quotes) {
            in_quotes = 1;
            quote_char = input_copy[i];
        }
        // Handle end of quotes
        else if (in_quotes && input_copy[i] == quote_char) {
            in_quotes = 0;
        }
        // Count token boundaries (spaces outside of quotes)
        else if (!in_quotes && isspace((unsigned char)input_copy[i]) && 
                 (i == 0 || !isspace((unsigned char)input_copy[i-1]))) {
            max_tokens++;
        }
        
        i++;
    }
    
    // Add one more for the last token
    max_tokens++;
    
    // Allocate memory for the array of pointers
    char **args = (char **)malloc((max_tokens + 1) * sizeof(char *));
    if (args == NULL) {
        free(input_copy);
        printf("Memory allocation failed\n");
        exit(1);
    }
    
    // Second pass: extract tokens
    int arg_index = 0;
    char *buffer = (char *)malloc(len + 1);
    if (buffer == NULL) {
        free(input_copy);
        free(args);
        printf("Memory allocation failed\n");
        exit(1);
    }
    
    i = 0;
    int buf_pos = 0;
    in_quotes = 0;
    quote_char = '\0';
    
    while (i <= len) {
        // End of string or space outside quotes
        if (i == len || (!in_quotes && isspace((unsigned char)input_copy[i]))) {
            if (buf_pos > 0) {
                buffer[buf_pos] = '\0';
                args[arg_index++] = strdup(buffer);
                buf_pos = 0;
            }
        }
        // Handle quotes
        else if (input_copy[i] == '"' || input_copy[i] == '\'') {
            if (!in_quotes) {
                // Start of quote
                in_quotes = 1;
                quote_char = input_copy[i];
            } 
            else if (input_copy[i] == quote_char) {
                // End of quote
                in_quotes = 0;
            }
            else {
                // Add the quote character itself if it's not the matching one
                buffer[buf_pos++] = input_copy[i];
            }
        }
        // Regular character
        else {
            buffer[buf_pos++] = input_copy[i];
        }
        
        i++;
    }
    
    // Null-terminate the array
    args[arg_index] = NULL;
    
    // Free temporary buffers
    free(buffer);
    free(input_copy);
    
    return args;
}