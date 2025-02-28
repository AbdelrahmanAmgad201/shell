#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#include "string_parsing.h"

#define MAXSIZE 1024

// Chain of responsibility -> extract variables -> tokenize  -> check command type -> execute

int check_instruction(char **args) {
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
        for(int i = 1;  args[i] != NULL ; i++){
            printf("%s ", args[i]);
        }
        printf("\n");
    } else if (strcmp(args[0], "export") == 0) {
        if (args[1] == NULL) {
            printf("export: missing argument\n");
            return;
        }

        // Tokenize args[1] to get variable name and value
        char *var_name = strtok(args[1], "=");
        char *var_value = strtok(NULL, "=");

        if (var_name == NULL || var_value == NULL) {
            printf("export: invalid format, use export VAR=VALUE\n");
            return;
        }

        // Store in the environment
        if (setenv(var_name, var_value, 1) != 0) {
            perror("export failed");
        }
    } else {
        printf("Unknown  command\n");
    }
}

void setup_env(){
    chdir(getenv("HOME"));
    printf("WELCOME TO MY SHELL\n");
}

void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
    printf("Child process reaped\n");
}

void free_array(char **args){
    for (int i = 0; args[i] != NULL; i++) {
        free(args[i]);
    }
    free(args);
}

void extract_variables(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        char *pos = strchr(args[i], '$'); // Find '$' in the argument
        if (pos) {
            // Extract variable name after '$'
            char *var_name = pos + 1;  
            char *var_value = getenv(var_name);  // Get environment variable value
            
            if (var_value) {
                // Allocate space for the new argument with the variable replaced
                int prefix_len = pos - args[i];  // Length of part before '$'
                int new_size = prefix_len + strlen(var_value) + 1;
                char *new_arg = (char *)malloc(new_size);
                
                if (new_arg) {
                    strncpy(new_arg, args[i], prefix_len);  // Copy prefix
                    new_arg[prefix_len] = '\0';  // Null-terminate
                    strcat(new_arg, var_value);  // Append variable value
                    
                    free(args[i]); // Free old memory
                    args[i] = new_arg;
                }
            } else {
                // If variable is not found, remove only `$var` but keep the prefix
                int prefix_len = pos - args[i]; 
                char *new_arg = (char *)malloc(prefix_len + 1);
                
                if (new_arg) {
                    strncpy(new_arg, args[i], prefix_len);
                    new_arg[prefix_len] = '\0';
                    
                    free(args[i]);  // Free old memory
                    args[i] = new_arg;
                }
            }
        }
    }
}
// Function to expand environment variables within a string
char* extract_variables_from_string(char *input) {
    // Create a working buffer for the result, starting with enough space
    int initial_size = strlen(input) * 2 + 1; // Double the size to allow for expansion
    char *result = (char *)malloc(initial_size);
    if (!result) {
        printf("Memory allocation failed\n");
        return NULL;
    }
    
    int result_pos = 0;
    int i = 0;
    int max_size = initial_size;
    int in_quotes = 0;
    char quote_char = '\0';
    
    // Initialize the result buffer
    result[0] = '\0';
    
    while (input[i] != '\0') {
        // Handle quotes (to properly recognize $ inside quotes)
        if ((input[i] == '"' || input[i] == '\'') && 
            (i == 0 || input[i-1] != '\\')) {
            if (!in_quotes) {
                in_quotes = 1;
                quote_char = input[i];
            } else if (input[i] == quote_char) {
                in_quotes = 0;
            }
            // Always include the quote character
            result[result_pos++] = input[i];
        }
        // Handle environment variable expansion
        else if (input[i] == '$' && (i == 0 || input[i-1] != '\\')) {
            // Extract variable name
            int j = i + 1;
            int var_len = 0;
            
            // Identify variable name - allowing alphanumeric and underscore
            while (isalnum(input[j]) || input[j] == '_') {
                j++;
                var_len++;
            }
            
            // We found a variable name
            if (var_len > 0) {
                char var_name[var_len + 1];
                strncpy(var_name, input + i + 1, var_len);
                var_name[var_len] = '\0';
                
                // Get environment variable value
                char *var_value = getenv(var_name);
                
                if (var_value) {
                    int value_len = strlen(var_value);
                    
                    // Ensure we have enough space in our result buffer
                    if (result_pos + value_len >= max_size) {
                        max_size = max_size + value_len + initial_size;
                        char *new_result = (char *)realloc(result, max_size);
                        if (!new_result) {
                            printf("Memory allocation failed during expansion\n");
                            free(result);
                            return NULL;
                        }
                        result = new_result;
                    }
                    
                    // Copy the variable value into the result
                    strcpy(result + result_pos, var_value);
                    result_pos += value_len;
                }
                
                // Skip past the variable name
                i = j;
                continue;
            }
        }
        else {
            // Regular character, just copy it
            result[result_pos++] = input[i];
        }
        
        // Check if we need more space
        if (result_pos >= max_size - 1) {
            max_size *= 2;
            char *new_result = (char *)realloc(result, max_size);
            if (!new_result) {
                printf("Memory allocation failed\n");
                free(result);
                return NULL;
            }
            result = new_result;
        }
        
        i++;
    }
    
    // Null-terminate the result
    result[result_pos] = '\0';
    
    return result;
}
void shell() {
    char input[MAXSIZE];
    char **args;
    signal(SIGCHLD, sigchld_handler);
    setup_env();
    setenv("x", "l", 1);
    while (1) {
        printf("> ");  // Shell prompt

        if (scanf(" %[^\n]%*c", input) != 1) {
            printf("Error reading input.\n");
            continue;
        }

        // args = parse_input(input);
        // extract_variables(args);
        // print_string_array(args);

        char *expanded_input = extract_variables_from_string(input);
        if (!expanded_input) {
            printf("Error expanding variables.\n");
            continue;
        }

        // Now tokenize the expanded input
        args = parse_input(expanded_input);
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
                printf("Unknown command\n");
        }
        free_array(args);
        
    }
}

int main() {
    shell();
    return 0;
}
