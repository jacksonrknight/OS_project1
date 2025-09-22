#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lexer.h"  


void print_prompt(){
    char hostname[256];
    char cwd[1024];

    gethostname(hostname, sizeof(hostname));
    getcwd(cwd, sizeof(cwd));

    char *username = getenv("USER");

    printf("%s@%s:%s> ", username, hostname, cwd);
    fflush(stdout);
}

void execute_command(tokenlist *tokens) {
    if(tokens->size == 0){
        return;
    }

    if(strcmp(tokens->items[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }

    pid_t pid = fork();

    if (pid < 0){
        //error on fork
        perror("fork");
    } else if (pid == 0){
        //child!!
        //create args array for execvp
        char **args = malloc((tokens->size + 1) * sizeof(char*));
        if (args == NULL){
            perror("malloc");
            exit(1);
        }

        for(int i = 0; i < tokens->size; i++) {
            args[i] = tokens->items[i];
        }
        args[tokens->size] = NULL; //null terminate the array

        //execute command
        execvp(args[0], args);

        fprintf(stderr, "Command not found: %s\n", args[0]);
        free(args);
        exit(1);
    } else {
        //parent process - waiting for child
        int status;
        waitpid(pid, &status, 0);
    }
}

char *expand_env_var(char *token) {
    if (token[0] == '$'){
        char *env_value = getenv(token + 1);
        if (env_value) {
            return strdup(env_value);
        }
        return strdup(""); //return empty if not found
    }
    return strdup(token); // return og if not start w a $
}

tokenlist *expand_env_vars(tokenlist *original){
    tokenlist *expanded = new_tokenlist();

    for(int i = 0; i < original->size; i++){
        char *expanded_token = expand_env_var(original->items[i]);
        add_token(expanded, expanded_token);
        free(expanded_token);    
    }

    return expanded;
}

int main() {
    printf("shell starting\n");
    
    while (1) {
        print_prompt();
        
        char *input = get_input();
        if(input == NULL) {
            printf("byebye shell\n");
            break; // EXIT on EOF
        }

        tokenlist *tokens = get_tokens(input);

        execute_command(tokens);

        //cleanup
        free_tokens(tokens);
        free(input);
    }
    
    return 0;
}