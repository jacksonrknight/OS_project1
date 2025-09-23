#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

#include "lexer.h" 

#define MAX_JOBS 100 // max 10 in project req
#define MAX_HISTORY 10

typedef struct {
    int job_id;
    pid_t pid;
    char *command_line;
    int completed;
} job_t;

job_t jobs[MAX_JOBS];
int job_count = 0;
int next_job_id = 1;

char *command_history[MAX_HISTORY];
int history_count = 0;

//FUNCTION DECLARATIONS

//P1:PRINT PROMPT
void print_prompt();

//P2 $ expanded
tokenlist *expand_env_vars(tokenlist *original);
char *expand_env_var(char *token);

//PART 3: EXPAND TILDES
char *expand_tilde(char *token);
tokenlist *expand_tildes(tokenlist *original);

//P4: PATH SEARCH
char *search_path(const char *cmd);

//P5 ETC: COMMAND EXECUTIONS
void execute_command(tokenlist *tokens);
int handle_builtins(tokenlist *tokens);

//P6: I/O REDIRECTION
tokenlist *parse_redirections(tokenlist *tokens, char **input_file, char **output_file);
int setup_redirections(const char *input_file, const char *output_file);
void restore_redirections(int stdin_copy, int stdout_copy);

//P7: PIPING
tokenlist **split_by_pipe(tokenlist *tokens, int *command_count);
void execute_pipeline(tokenlist **commands, int cmd_count, int is_background, char *original_command);
int is_builtin_command(const char *cmd);
void execute_builtin_with_pipes(tokenlist *tokens, int pipe_out);

//P8: BACKGROUND PROCESSING
int is_background_job(tokenlist *tokens);
int add_job(pid_t pid, char *cmd_line);
void check_completed_jobs();
void cleanup_jobs();
char *build_command_line(tokenlist *tokens);
char *build_pipeline_command_line(tokenlist **commands, int cmd_count);

//COMMAND HISTORY
void add_to_history(char *cmd_line);


int main() {
    printf("shell starting\n");
    
    while (1) {
        //check 4 completed jobs
        check_completed_jobs();

        //print prompt and get input
        print_prompt();
        char *input = get_input();

        //check for exit sequence
        if(input == NULL) {
            printf("byebye shell\n");
            break; // EXIT on EOF
        }

        //save cmd to history if not empty
        if(strlen(input) > 0) {
            add_to_history(input);
        }

        //get tokens from input
        tokenlist *tokens = get_tokens(input);

        //expand env vars
        tokenlist *expanded_env = expand_env_vars(tokens);

        //then tildes
        tokenlist *tilde_expanded = expand_tildes(expanded_env);
        execute_command(tilde_expanded);

        //cleanup
        free_tokens(tokens);
        free_tokens(expanded_env);
        free_tokens(tilde_expanded);
        free(input);
    }

    for (int i = 0; i < history_count; i++) {
        free(command_history[i]);
    }
    
    return 0;
}

//PART 1 : PRINT PROMPT

void print_prompt(){
    char hostname[256];
    char cwd[1024];

    gethostname(hostname, sizeof(hostname));
    getcwd(cwd, sizeof(cwd));

    char *username = getenv("USER");

    printf("%s@%s:%s> ", username, hostname, cwd);
    fflush(stdout);
}

//PART 4: PATH SEARCH

char *search_path(const char *cmd) {

    // handle null command
    if (cmd == NULL) {
        fprintf(stderr, "Error: null command passed to search_path\n");
        return NULL;
    }

    //if cmd has slash, its path alr, return copy
    if (strchr(cmd, '/') != NULL) {
        return strdup(cmd);
    }

    //get path env var
    char *path_env = getenv("PATH");
    if(path_env == NULL){
        fprintf(stderr, "Warning: PATH environment variable not set\n");
        return NULL; // path not set then
    }

    //make copy path since strtok modifies string
    char *path_copy = strdup(path_env);
    if (path_copy == NULL){
        return NULL;
    }

    char *path_token;
    char *result = NULL;

    //tokenize path
    path_token = strtok(path_copy, ":");
    while (path_token != NULL) {

        //construct full path to test
        int path_len = strlen(path_token);
        int cmd_len = strlen(cmd);
        char *full_path = malloc(path_len + cmd_len + 2); // 2 for '/' and null

        if (full_path == NULL){
            path_token = strtok(NULL, ":");
            continue;
        }

        //full path
        strcpy(full_path, path_token);

        //add slash
        if (path_token[path_len - 1] != '/') {
            strcat(full_path, "/");
        }

        strcat(full_path, cmd);

        //check if file exists and is executable
        if(access(full_path, X_OK) == 0) {
            result = full_path; //found
            break;
        }

        free(full_path);
        path_token = strtok(NULL, ":");
    }

    free(path_copy);
    return result;
}

//check if command should be run in background
int is_background_job(tokenlist *tokens){
    if(tokens->size > 0 && strcmp(tokens->items[tokens->size - 1], "&") == 0) {
        //remove the & from cmd tokens
        free(tokens->items[tokens->size - 1]);
        tokens->items[tokens->size - 1] = NULL;
        tokens->size--;
        return 1;
    }
    return 0;
}

//build cmd line string from tokens
char *build_command_line(tokenlist *tokens){
    //calc total length needed
    int total_len = 0;
    for( int i = 0; i < tokens->size; i++) {
        total_len += strlen(tokens->items[i]) + 1; //+1 for space
    }

    //malloc
    char *cmd_line = malloc(total_len + 1); //+1 for null term
    if (!cmd_line) return NULL;

    //build string
    cmd_line[0] = '\0';
    for(int i = 0; i < tokens->size; i++) {
        strcat(cmd_line, tokens->items[i]);
        if (i < tokens->size - 1){
            strcat(cmd_line, " ");
        }
    }
    return cmd_line;
}

//build pipeline cmd line string
char *build_pipeline_command_line(tokenlist **commands, int cmd_count) {
    //calc total len needed
    int total_len = 0;
    for (int i = 0; i < cmd_count; i++){
        for (int j = 0; j < commands[i]->size; j++){
            total_len += strlen(commands[i]->items[j]) + 1;
        }

        if(i < cmd_count - 1){
            total_len += 3; // " | "
        }
    }

    //allocate mem
    char *cmd_line = malloc(total_len + 1);
    if(!cmd_line) return NULL;

    //build string
    cmd_line[0] = '\0';
    for(int i = 0; i < cmd_count; i++){
        for (int j = 0; j < commands[i]->size; j++){
            strcat(cmd_line, commands[i]->items[j]);
            if (j < commands[i]->size - 1){
                strcat(cmd_line, " ");
            }
        }

        if(i < cmd_count - 1){
            strcat(cmd_line, " | ");
        }
    }

    return cmd_line;
}

//add new background job to list
int add_job(pid_t pid, char *cmd_line){
    if(job_count >= MAX_JOBS){
        fprintf(stderr, "Error: Maximum number of jobs reached\n");
        return -1;
    }

    int job_id = next_job_id++;
    jobs[job_count].job_id = job_id;
    jobs[job_count].pid = pid;
    jobs[job_count].command_line = strdup(cmd_line);
    jobs[job_count].completed = 0;

    job_count++;
    return job_id;
}

//check all completed jobs for completion
void check_completed_jobs() {
    for (int i = 0; i < job_count; i++) {
        if (!jobs[i].completed) {
            int status;
            pid_t result = waitpid(jobs[i].pid, &status, WNOHANG);
            
            if (result == jobs[i].pid) {
                //job finished
                jobs[i].completed = 1;
                printf("[%d] + done %s\n", jobs[i].job_id, jobs[i].command_line);
            }
        }
    }
    
    //cleanup completed jobs
    cleanup_jobs();
}

//take completed jobs off array
void cleanup_jobs() {
    int i = 0;
    while (i < job_count) {
        if (jobs[i].completed) {
            free(jobs[i].command_line);
            
            //shift jobs in array
            for (int j = i; j < job_count - 1; j++) {
                jobs[j] = jobs[j + 1];
            }
            job_count--;
        } else {
            i++;
        }
    }
}

//BIG EXECUTE COMMAND FUNCTION
void execute_command(tokenlist *tokens) {
    
    if(tokens == NULL || tokens->size == 0 || tokens->items[0] == NULL){
        return;
    }

    //check if background job
    int is_background = is_background_job(tokens);

    //save og cmd for job tracking if background
    char *original_command = NULL;
    if (is_background) {
        original_command = build_command_line(tokens);
    }

    //check for pipes
    int has_pipe = 0;
    for (int i = 0; i < tokens->size; i++){
        if (strcmp(tokens->items[i], "|") == 0){
            has_pipe = 1;
            break;
        }
    }

    if(has_pipe){
        //handle pipeline
        int cmd_count = 0;
        tokenlist **commands = split_by_pipe(tokens, &cmd_count);

        if(commands != NULL) {

            //if background job but dont have og cmd yet
            if(is_background && !original_command){
                original_command = build_pipeline_command_line(commands, cmd_count);
            }

            execute_pipeline(commands, cmd_count, is_background, original_command);

            //cleanup
            for(int i = 0; i < cmd_count; i++){
                free_tokens(commands[i]);
            }
            free(commands);
        }
    } else {
        //handle regular command w redirections
        char *input_file = NULL;
        char *output_file = NULL;
        tokenlist *cmd_tokens = parse_redirections(tokens, &input_file, &output_file);

        if(cmd_tokens->size == 0) {
            fprintf(stderr, "Error: No command specified\n");
            if (input_file) free(input_file);
            if (output_file) free(output_file);
            free_tokens(cmd_tokens);
            if (original_command) free(original_command);
            return;
        }

        if(handle_builtins(cmd_tokens)) {
            //cleanup
            if (input_file) free(input_file);
            if (output_file) free(output_file);
            free_tokens(cmd_tokens);
            if (original_command) free(original_command);
            return;
        }

        char *cmd_path = search_path(cmd_tokens->items[0]);
        if(cmd_path == NULL){
            fprintf(stderr, "Command not found: %s\n", cmd_tokens->items[0]);
            if (input_file) free(input_file);
            if (output_file) free(output_file);
            free_tokens(cmd_tokens);
            if (original_command) free(original_command);
            return;
        }

        pid_t pid = fork();

        if (pid < 0){
            //error on fork
            perror("fork");
            free(cmd_path);
            if (input_file) free(input_file);
            if (output_file) free(output_file);
            free_tokens(cmd_tokens);
            if (original_command) free(original_command);
            return;
        } else if (pid == 0){
            //child!!

            //set up redirections
            if(setup_redirections(input_file, output_file) < 0) {
                //redirection failed
                exit(EXIT_FAILURE);
            }

            //create args array for execv
            //execv expects array where : 
            //args[0]=programname, args[1]-args[n-1]=args and args[n]=NULL

            char **args = malloc((cmd_tokens->size + 1) * sizeof(char*));
            if (args == NULL){
                perror("malloc");
                exit(EXIT_FAILURE);
            }

            //copy all tokens to args array
            for(int i = 0; i < cmd_tokens->size; i++) {
                args[i] = cmd_tokens->items[i];
            }
            args[cmd_tokens->size] = NULL; //null terminate the array

            //execute command
            execv(cmd_path, args);

            perror("execv");
            free(args);
            free(cmd_path);
            exit(EXIT_FAILURE);
        } else {
            //parent process - waiting for child
            if(is_background){
                //add to background jobs
                int job_id = add_job(pid, original_command);
                printf("[%d] %d\n", job_id, pid);
            } else {
                //wait for foreground cmd to finish
                int status;
                if (waitpid(pid, &status, 0) == -1) {
                    perror("waitpid");
                }
            }

            free(cmd_path);
            if (input_file) free(input_file);
            if (output_file) free(output_file);
            free_tokens(cmd_tokens);
            if(original_command) free(original_command);
        }
    }
}

//returns 0 for fail 1 for success
int handle_builtins(tokenlist *tokens) {

    if(tokens == NULL || tokens->size == 0){
        return 0;
    }

    if(strcmp(tokens->items[0], "exit") == 0) {
        //print last 3 valid commands
        printf("Last commands:\n");
        if(history_count == 0){
            printf("No commands in history\n");
        } else {
            int start = (history_count > 3) ? history_count - 3 : 0;
            for(int i = start; i < history_count; i++){
                printf("%d: %s\n", i + 1, command_history[i]);
            }
        }
        //wait for all background processes to be complete
        printf("Waiting for all background processes to finish.\n");
        for (int i = 0; i < job_count; i++) {
            if (!jobs[i].completed) {
                printf("Waiting for [%d] %s\n", jobs[i].job_id, jobs[i].command_line);
                waitpid(jobs[i].pid, NULL, 0);
            }
        }

        printf("exiting shell\n");
        exit(EXIT_SUCCESS);
        return 1;
    }

    //cd command
    if(strcmp(tokens->items[0], "cd") == 0) {
        //check for too many args
        if (tokens->size > 2) {
            fprintf(stderr, "Error: Too many arguments for cd\n");
            return 1;
        }
        
        //get target directory
        char *dir = tokens->size > 1 ? tokens->items[1] : getenv("HOME");
        if (dir == NULL) {
            fprintf(stderr, "Error: HOME environment variable not set\n");
            return 1;
        }
        
        //check if directory exists
        struct stat st;
        if (stat(dir, &st) != 0) {
            fprintf(stderr, "Error: Directory '%s' does not exist\n", dir);
            return 1;
        }
        
        //check if it's a directory
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "Error: '%s' is not a directory\n", dir);
            return 1;
        }
        
        //change directory
        if(chdir(dir) != 0) {
            perror("cd");
        }
        return 1;
    }
    
    //jobs command
    if(strcmp(tokens->items[0], "jobs") == 0) {
        if (job_count == 0) {
            printf("No active background processes\n");
        } else {
            for (int i = 0; i < job_count; i++) {
                if (!jobs[i].completed) {
                    printf("[%d]+ %d %s\n", jobs[i].job_id, jobs[i].pid, jobs[i].command_line);
                }
            }
        }
        return 1;
    }

    return 0;
}

//PART 2: expand env var

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

//PART 3: TILDE EXPANSION

//returns newly allocated string w tilde expanded
char *expand_tilde(char *token){

    if (token == NULL){
        return NULL;
    }

    if(strcmp(token, "~") == 0 || strncmp(token, "~/", 2) == 0) {
        //get HOME enviroment var
        char *home = getenv("HOME");
        if (home == NULL){
            //return og token if home not set
            return strdup(token);
        }

        if (strcmp(token, "~") == 0) {
            //token is only ~ replace w home
            return strdup(home);
        } else {
            //token starts w ~/ replace ~ w home thats it
            // +1 to skip ~
            char *path = token + 1;

            char *result = malloc(strlen(home) + strlen(path) + 1);
            if (result == NULL){
                return NULL; //malloc failed
            }

            strcpy(result, home);
            strcat(result, path);

            return result;
        }
    }

    return strdup(token); //no tilde to expand
}

tokenlist *expand_tildes(tokenlist *original) {
    if (original == NULL) {
        return NULL;
    }

    tokenlist *expanded = new_tokenlist();
    if (expanded == NULL) {
        return NULL;
    }

    for (int i = 0; i < original->size; i++) {
        char *expanded_token = expand_tilde(original->items[i]);
        if(expanded_token == NULL){
            fprintf(stderr, "malloc fail in tilde expansion");
            free_tokens(expanded);
            return NULL;
        }

        add_token(expanded, expanded_token);
        free(expanded_token);
    }

    return expanded;
}

//PART 6: I/O redirection

//processes redirecton operators in cmd tokens
tokenlist *parse_redirections(tokenlist *tokens, char **input_file, char **output_file) {
    tokenlist *cmd_tokens = new_tokenlist();
 
    //init to NULL
    if(*input_file) {
        free(*input_file);
        *input_file = NULL;
    }

    if(*output_file){
        free(*output_file);
        *output_file = NULL;
    }

    for (int i = 0; i < tokens->size; i++) {
        //output redirection
        if (strcmp(tokens->items[i], ">") == 0) {
            //next token = output file
            if (i + 1 < tokens->size) {
                if(*output_file){
                    free(*output_file);
                }
                *output_file = strdup(tokens->items[i + 1]);
                i++; 
            } else {
                fprintf(stderr, "Syntax error: missing filename after >\n");
            }
        }
        // inp redirection
        else if (strcmp(tokens->items[i], "<") == 0) {
            // next token =inp file
            if (i + 1 < tokens->size) {
                if(*input_file){
                    free(*input_file);
                }
                *input_file = strdup(tokens->items[i + 1]);
                i++; 
            } else {
                fprintf(stderr, "Syntax error: missing filename after <\n");
            }
        }
        else {
            add_token(cmd_tokens, tokens->items[i]);
        }
    }
    
    return cmd_tokens;
}

int setup_redirections(const char *input_file, const char *output_file) {

    if(input_file != NULL) {
        struct stat st;
        if(stat(input_file, &st) != 0) {
            perror("input redirection");
            return -1;
        }

        if(!S_ISREG(st.st_mode)) {
            fprintf(stderr, "Error: %s is not a regular file\n", input_file);
            return -1;
        }

        //open inp file
        int fd = open(input_file, O_RDONLY);
        if (fd < 0) {
            perror("input redirection");
            return -1;
        }

        //redirect stdin to file
        if (dup2(fd, STDIN_FILENO) < 0) {
            perror("input redirection");
            close(fd);
            return -1;
        }

        close(fd);

    }

    if(output_file != NULL) {
        //open output file and create if dont exist
        int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC , 0600);
        if(fd < 0) {
            perror("output redirection");
            return -1;
        }

        //redirect stdout to file
        if(dup2(fd, STDOUT_FILENO) < 0) {
            perror("output redirection");
            close(fd);
            return -1;
        }
        // close og fd 
        close(fd);
    }

    return 0;
}

//restores og stdin/stdout after redirection
void restore_redirections(int stdin_copy, int stdout_copy) {
    if(stdin_copy >= 0){
        dup2(stdin_copy, STDIN_FILENO);
        close(stdin_copy);
    }

    if (stdout_copy >= 0){
        dup2(stdout_copy, STDOUT_FILENO);
        close(stdout_copy);
    }
}

tokenlist **split_by_pipe(tokenlist *tokens, int *command_count){
    int pipe_count = 0;
    for(int i = 0; i < tokens->size; i++){
        if (strcmp(tokens->items[i], "|") == 0) {
            pipe_count++;
        }
    }

    //number of commands is pipe_count + 1
    *command_count = pipe_count + 1;

    //alloc array of tokenlist ptrs
    tokenlist **commands = malloc(*command_count * sizeof(tokenlist *));
    if(commands == NULL) {
        perror("malloc");
        return NULL;
    }

    //init each tokenlist
    for (int i = 0; i < *command_count; i++) {
        commands[i] = new_tokenlist();
        if (commands[i] == NULL) {
            //clean up if alloc fail
            for(int j = 0; j < i; j++){
                free_tokens(commands[j]);
            }
            free(commands);
            return NULL;
        }
    }

    //split the tokens into seperate commands
    int current_cmd = 0;
    for(int i = 0; i < tokens->size; i++){
        if (strcmp(tokens->items[i], "|") == 0){
            //move to next command on pipe symb
            current_cmd++;
        } else {
            //add token to current cmd
            add_token(commands[current_cmd], tokens->items[i]);
        }
    }

    return commands;
}

void execute_pipeline(tokenlist **commands, int cmd_count, int is_background, char *original_command) {
    if (commands == NULL || cmd_count <= 0) {
        return;
    }

    int (*pipes)[2] = malloc((cmd_count - 1) * sizeof(int[2]));
    if(!pipes) {
        perror("malloc");
        return;
    }

    //create pipes
    for(int i = 0; i < cmd_count - 1;i++){
        if(pipe(pipes[i]) == -1) {
            perror("pipe");
            return;
        }
    }

    //create child processes for each command

    pid_t pids[cmd_count];

    for(int i = 0; i < cmd_count; i++){
        pids[i] = fork();

        if(pids[i] < 0) {
            //fork failed
            perror("fork");

            //close all pipes
            for(int j = 0; j < cmd_count - 1; j++){
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            free(pipes);
            return;
        } else if (pids[i] == 0) {
            //child!!
            //set up redirections here
            if (i > 0) {
                if (dup2(pipes[i-1][0], STDIN_FILENO) == -1){
                    perror("dup");
                    exit(EXIT_FAILURE);
                }
            }

            if(i < cmd_count - 1) {
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            for(int j = 0; j < cmd_count - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            //parse redirections for this command
            char *input_file = NULL;
            char *output_file = NULL;
            tokenlist *cmd_tokens = parse_redirections(commands[i], &input_file, &output_file);

            //set up addl file redirections
            if (i == 0 && input_file != NULL) {
                int fd = open(input_file, O_RDONLY);
                if(fd == -1){
                    fprintf(stderr, "Cannot open input file '%s': %s\n",
                            input_file, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            if (i == cmd_count - 1 && output_file != NULL) {
                int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0600);
                if (fd == -1) {
                    fprintf(stderr, "Cannot open output file '%s': %s\n",
                            output_file, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            if(input_file) free(input_file);
            if(output_file) free(output_file);

            //check if cmd empty after parsing
            if(cmd_tokens->size == 0){
                fprintf(stderr, "Error: Empty command in pipeline\n");
                free_tokens(cmd_tokens);
                exit(EXIT_FAILURE);
            }

            //get path to the command
            char *cmd_path = search_path(cmd_tokens->items[0]);
            if(cmd_path == NULL){
                fprintf(stderr, "Command not found: %s\n", cmd_tokens->items[0]);
                free_tokens(cmd_tokens);
                exit(EXIT_FAILURE);
            }

            char **args = malloc((cmd_tokens->size + 1) * sizeof(char *));
            if(args == NULL) {
                perror("malloc");
                free(cmd_path);
                free_tokens(cmd_tokens);
                exit(EXIT_FAILURE);
            }

            //copy tokens to args
            for(int j = 0; j < cmd_tokens->size; j++){
                args[j] = cmd_tokens->items[j];
            }
            args[cmd_tokens->size] = NULL;

            //EXECUTE
            execv(cmd_path, args);

            //if returns it failed
            perror("execv");
            free(args);
            free(cmd_path);
            free_tokens(cmd_tokens);
            exit(EXIT_FAILURE);
        }
    }

    //parent process
    //close all pipes in parent
    for(int i = 0; i < cmd_count - 1; i++){
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    if (is_background){
        //track last process in pipeline
        int job_id = add_job(pids[cmd_count - 1], original_command);
        printf("[%d] %d\n", job_id, pids[cmd_count - 1]);
    } else {
        //wait for all children to complete for foreground commands
        for(int i = 0; i < cmd_count; i++){
            int status;
            waitpid(pids[i], &status, 0);
        }
    }

    free(pipes);
}

int is_builtin_command(const char *cmd) {
    return (strcmp(cmd, "exit") == 0 ||
            strcmp(cmd, "cd") == 0 ||
            strcmp(cmd, "jobs") == 0);
}

void execute_builtin_with_pipes(tokenlist *tokens, int pipe_out) {
    // save og std out
    int stdout_copy = -1;
    if (pipe_out >= 0) {
        stdout_copy = dup(STDOUT_FILENO);
        dup2(pipe_out, STDOUT_FILENO);
    }
    
    // execute cmd
    handle_builtins(tokens);
    
    // restore stdout 
    if (stdout_copy >= 0) {
        dup2(stdout_copy, STDOUT_FILENO);
        close(stdout_copy);
    }
}

void add_to_history(char *cmd_line) {
    if(!cmd_line || strlen(cmd_line) == 0) {
        return;
    }

    //if history is full this will remove oldest entry
    if (history_count == MAX_HISTORY) {
        free(command_history[0]);
        for(int i = 0; i < MAX_HISTORY - 1; i++){
            command_history[i] = command_history[i + 1];
        }
        history_count--;
    }

    //add new command to history
    command_history[history_count++] = strdup(cmd_line);
}