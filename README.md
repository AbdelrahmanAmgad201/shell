
# Simple Shell (Multi-Processing)

## Overview

This project is an implementation of a simple Unix shell. The shell supports running external commands, built-in commands, background execution, and signal handling. It also logs terminated child processes to a log file.

## Features

-   **Command Execution**: Runs both built-in and external commands.
-   **Built-in Commands**: Supports `cd`, `echo`, and `export`.
-   **Background Execution**: Allows commands to be run in the background using `&`.
-   **Signal Handling**: Handles `SIGCHLD` signals to clean up zombie processes and log terminated processes.
-   **Environment Variable Expansion**: Expands variables prefixed with `$` in user input.
-   **Custom Prompt**: Displays the current working directory in the prompt.

## Getting Started

### Prerequisites

-   A Unix-based operating system (Linux, macOS)
-   GCC compiler

### Compilation

To compile the shell, use:

```sh
gcc -o my_shell my_shell.c

```

### Running the Shell

To start the shell, run:

```sh
./my_shell

```

## Usage

### Running Commands

You can run any Unix command as you would in a normal shell:

```sh
ls -l

```

### Built-in Commands

-   **`cd [directory]`**: Changes the current directory. Example:
    
    ```sh
    cd /home/user
    
    ```
    
-   **`echo [text]`**: Prints text to the console. Example:
    
    ```sh
    echo Hello, world!
    
    ```
    
-   **`export VAR=VALUE`**: Sets an environment variable. Example:
    
    ```sh
    export PATH=/usr/local/bin:$PATH
    
    ```
    

### Background Execution

Append `&` to a command to run it in the background:

```sh
sleep 10 &

```

### Exiting the Shell

Use the `exit` command to close the shell:

```sh
exit

```

## Logging

Terminated child processes are logged to:

```sh
/home/{username}/dev/projects/shell/shell_child_log.txt

```

Each entry includes the process ID and termination status.

## Future Improvements

-   Support for command history
