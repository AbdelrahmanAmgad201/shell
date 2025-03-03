#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAXSIZE 1024
#define GREEN "\033[1;32m"
#define BLUE  "\033[1;34m"
#define RESET "\033[0m"

#define CMD_EXIT 0
#define CMD_BUILTIN 1
#define CMD_EXTERNAL 2
#define CMD_UNKNOWN 3


void sigchld_handler(int sig) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // Print termination message to terminal
        printf("[Shell] Process %d terminated\n", pid);
        
        // Get current timestamp
        time_t now = time(NULL);
        char *timestamp = ctime(&now);
        timestamp[strlen(timestamp) - 1] = '\0';
        
        // Log to file
        FILE *fptr = fopen("/home/abdelrahman-amgad/dev/projects/shell/shell_child_log.txt", "a"); // Open in append mode
        if (fptr) {
            fprintf(fptr, "[%s] Process %d terminated with status %d\n", 
                   timestamp, pid, status);
            fclose(fptr);
        }
    }
}

void print_prompt() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf(GREEN "[my-shell] " RESET BLUE "%s" RESET " $ ", cwd);
    } else {
        perror("getcwd");
    }
}

char **parse_input(char *input) {
    char *input_copy = strdup(input);
    if (input_copy == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    int len = strlen(input_copy);
    
    int max_tokens = 0;
    int i = 0;
    int in_quotes = 0;
    char quote_char = '\0';
    
    while (i < len) {
        if ((input_copy[i] == '"' || input_copy[i] == '\'') && !in_quotes) {
            in_quotes = 1;
            quote_char = input_copy[i];
        }
        else if (in_quotes && input_copy[i] == quote_char) {
            in_quotes = 0;
        }
        else if (!in_quotes && isspace((unsigned char)input_copy[i]) && 
                 (i == 0 || !isspace((unsigned char)input_copy[i-1]))) {
            max_tokens++;
        }
        
        i++;
    }
    
    max_tokens++;
    
    char **args = (char **)malloc((max_tokens + 1) * sizeof(char *));
    if (args == NULL) {
        free(input_copy);
        printf("Memory allocation failed\n");
        exit(1);
    }
    
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
        if (i == len || (!in_quotes && isspace((unsigned char)input_copy[i]))) {
            if (buf_pos > 0) {
                buffer[buf_pos] = '\0';
                args[arg_index++] = strdup(buffer);
                buf_pos = 0;
            }
        }
        else if (input_copy[i] == '"' || input_copy[i] == '\'') {
            if (!in_quotes) {
                in_quotes = 1;
                quote_char = input_copy[i];
            } 
            else if (input_copy[i] == quote_char) {
                in_quotes = 0;
            }
            else {
                buffer[buf_pos++] = input_copy[i];
            }
        }
        else {
            buffer[buf_pos++] = input_copy[i];
        }
        
        i++;
    }
    
    args[arg_index] = NULL;
    
    free(buffer);
    free(input_copy);
    
    return args;
}

int check_instruction(char **args) {
    if (!args || !args[0]) {
        return CMD_UNKNOWN;
    }
    
    if (strcmp(args[0], "exit") == 0) {
        return CMD_EXIT;
    } 
    
    if (strcmp(args[0], "cd") == 0 || 
        strcmp(args[0], "echo") == 0 || 
        strcmp(args[0], "export") == 0) {
        return CMD_BUILTIN;
    }
    
    return CMD_EXTERNAL;
}

void execute_external(char **args) {
    int background = 0;
    int i = 0;
    
    while (args[i] != NULL) i++;
    if (i > 0 && strcmp(args[i - 1], "&") == 0) {
        background = 1;
        args[i - 1] = NULL; 
    }

    pid_t pid = fork();
    if (pid < 0) {
        printf("Error executing command\n");
    } else if (pid == 0) {
        // Child process
        execvp(args[0], args);
        perror("execvp failed");
        exit(1);
    } else {
        // Parent process
        if (background) {
            printf("[Background] Process ID: %d\n", pid);
        } else {
            waitpid(pid, NULL, 0);
        }
    }
}

void execute_built_in(char **args) {
     if (strcmp(args[0], "cd") == 0) {
        const char *home = getenv("HOME");
        if (args[1] == NULL || strcmp(args[1], "~") == 0) {
            chdir(home);
        } else if (args[1][0] == '~') {
            char path[1024];
            snprintf(path, sizeof(path), "%s%s", home, args[1] + 1);
            chdir(path);
        } else {
            chdir(args[1]);
        }
    }else if (strcmp(args[0], "echo") == 0) { 
        for (int i = 1; args[i] != NULL; i++) {
            printf("%s ", args[i]);
        }
        printf("\n");
    } else if (strcmp(args[0], "export") == 0) {
        if (args[1] == NULL) {
            printf("export: missing argument\n");
            return;
        }

        char *var_name = strtok(args[1], "=");
        char *var_value = strtok(NULL, "=");

        if (var_name == NULL || var_value == NULL) {
            printf("export: invalid format, use export VAR=VALUE\n");
            return;
        }

        if (setenv(var_name, var_value, 1) != 0) {
            perror("export failed");
        }
    } else {
        printf("Unknown command\n");
    }
}

void setup_env() {
    // Use signal instead of sigaction
    signal(SIGCHLD, sigchld_handler);
    
    chdir("/");
    printf("WELCOME TO MY SHELL\n");
}
void free_array(char **args) {
    if (!args) return;
    
    for (int i = 0; args[i] != NULL; i++) {
        free(args[i]);
    }
    free(args);
}

char* extract_variables_from_string(char *input) {
    if (!input) return NULL;
    
    int initial_size = strlen(input) * 2 + 1;
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
    
    result[0] = '\0';
    
    while (input[i] != '\0') {
        if ((input[i] == '"' || input[i] == '\'') && 
            (i == 0 || input[i-1] != '\\')) {
            if (!in_quotes) {
                in_quotes = 1;
                quote_char = input[i];
            } else if (input[i] == quote_char) {
                in_quotes = 0;
            }
            result[result_pos++] = input[i];
        }
        else if (input[i] == '$' && (i == 0 || input[i-1] != '\\')) {
            int j = i + 1;
            int var_len = 0;
            
            while (isalnum(input[j]) || input[j] == '_') {
                j++;
                var_len++;
            }
            
            if (var_len > 0) {
                char var_name[var_len + 1];
                strncpy(var_name, input + i + 1, var_len);
                var_name[var_len] = '\0';
                
                char *var_value = getenv(var_name);
                
                if (var_value) {
                    int value_len = strlen(var_value);
                    
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
                    
                    strcpy(result + result_pos, var_value);
                    result_pos += value_len;
                }
                
                i = j;
                continue;
            }
        }
        else {
            result[result_pos++] = input[i];
        }
        
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
    
    result[result_pos] = '\0';
    return result;
}

void shell() {
    char input[MAXSIZE];
    char **args;
    
    // Signal handling is now in setup_env
    setup_env();
    
    while (1) {
        print_prompt();

        if (scanf(" %[^\n]%*c", input) != 1) {
            clearerr(stdin);
            getchar();
            continue;
        }

        char *expanded_input = extract_variables_from_string(input);
        if (!expanded_input) {
            printf("Error expanding variables.\n");
            continue;
        }

        args = parse_input(expanded_input);
        free(expanded_input);
        
        if (!args || !args[0]) {
            free_array(args);
            continue;
        }

        int instruction_type = check_instruction(args);

        switch (instruction_type) {
            case CMD_EXIT:
                free_array(args);
                exit(0);
                break;
            case CMD_BUILTIN:
                execute_built_in(args);
                break;
            case CMD_EXTERNAL:
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