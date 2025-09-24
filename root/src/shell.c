#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void expandTokens(tokenlist* tokens);
char* findPath(char* tokens);
void printTokenlist(tokenlist* tokens);



int main(int argc, char** argv)
{
    int count = 0;

    while (count < 5)
    {
        // char* temphostname;
        // print the shell's "prompt" using getenv() (format: USER@MACHINE:PWD>)
        printf("%s@%s:%s> \n", getenv("USER"), getenv("HOSTNAME"), getenv("PWD") );
        tokenlist* input = get_tokens(get_input());
        expandTokens(input);
        // read and user's input
        printTokenlist(input);

        // grab the command(item[0]) from user input
        char* commandPath = findPath(input->items[0]);

        // if a command path doesn't exist, free the input to prepare for next loop
        if (input->items[0] == NULL)
        {
            free_tokens(input);
            continue;   // then jump to next loop
        }

        
        // execution logic

        
        // printf("\n-----BEFORE FREE_TOKENS----\nTOKENLIST&= %p\nsize=%lu\n-------------------\n", &input, input->size);
        free(commandPath);
        free_tokens(input);
        // printf("\n-----AFTER FREE_TOKENS----\nTOKENLIST&= %p\nsize=%lu\n---------------------\n", &input, input->size);

        count++;
    }
    exit(0);

}

void printTokenlist(tokenlist* tokens)
{
    if (tokens->size > 0)
    {
        for (int i = 0; i < tokens->size; i++)
        {
            printf("--%i: %s\n",i, tokens->items[i]);
        }
    }
    else 
        printf("ERROR in printTokenList()\n");
    printf("\n");
}


/* expandTokens(tokenlist&)
*****
passing in a tokenlist(tokenization of the user input) will handle parts 2 and 3
- will expand token variable's symbolic representation ($USER)->(<username>), ($HOME)->(/home/username), ($SHELL)->(/bin/bash)
- will expand token tilde's symbolic representation (~)->(/home/username), (~/path/of/folders)->(/home/username/path/of/folders)
- NOTE: PROJECT DOES NOT SAY HOW TO HANDLE EXPANDING VARIABLES THAT DON'T EXIST. NULL IS RETURNED, BUT IM NOT ENTIRELY SURE IF THEY WANT AN ERROR OR TO JUST IGNORE THE NULL RETURN AND CONTINUE RUNNING--DESPITE MOST LIKELY BREAKING THE FUNCTIONALITY.
*/
void expandTokens(tokenlist* tokens)
{
    // iteratively expand variables 
    for (int i = 0; i < tokens->size ; i++)
    {    
        // add a temporary variable to store the current token
        char* currentToken = tokens->items[i];
        char* expandedToken = NULL;

        // caase of '$' is found at the beginning of the token
        if (currentToken[0] == '$') 
        {
            // varValue will store the expanded variable
            char* varValue = getenv(currentToken+1);

            if (varValue != NULL)
            {
                expandedToken = (char*)malloc(strlen(varValue) + 1);
                strcpy(expandedToken, varValue);     // replace variable token with it's expanded form
            }
        }

        // if a '~' is found and matches the requirements...
        else if ( currentToken[0] == '~' && ( currentToken[1] == '/' || currentToken[1] == '\0' ) )
        {
            // expand the value of ~
            const char* homePath = getenv("HOME");
            if (homePath != NULL)
            {
                expandedToken = (char*)malloc(strlen(homePath) + strlen(currentToken+1) + 1);
                // concatenate the homepath with the rest of the path           
                strcpy(expandedToken, homePath);                            
                strcat(expandedToken, (currentToken)+1);     // concatenate the remaining original string excluding the '~' element
            }
        } 

        // update the tokenlist
        if (expandedToken != NULL)
        {
            // printf("ERROR in expandTokens(): i=%d, tokens->items[i]=%s, strlen()=%d", i, (expandedToken), strlen(expandedToken) );
            free(tokens->items[i]);
            tokens->items[i] = expandedToken;
        }
    }
}

/* char* findPath(char* token)
accepts a token and returns an expanded path for the command (ls -> /usr/bin/ls)
*****
- check for slashes at start of token './' or '/'... if it does, don't search, just verify the path is valid.
- if no slash, get the $PATH
- tokenize the $PATH using strtok() using : as a delimiter
- identify the token that matches corresponds with the command
- check the final path 

*/

char* findPath(char* command)
{
    // case of path to command given-- check for slashes at start of token './' or '/'
    if (command[0] == '/' || ( command[0] == '.' && command[1] == '/' ) )
    {
        if (access(command, X_OK) == 0)
        {
            // create a copy that can be return
            char* commandCopy = malloc(strlen(command)+ 1);
            strcpy(commandCopy, command);
            return commandCopy;

        }
        return NULL;
    }

    // case of env variable given-- search $PATH
    char* pathValue = getenv("PATH");
    // if no $PATH env var there is no PATH to search-- return NULL
    if (pathValue == NULL)
    {
        return NULL;
    }

    // create a copy of $PATH and a temp variable to store each chunk of $PATH
    char* pathCopy = malloc(strlen(pathValue)+1);
    strcpy(pathCopy, pathValue);
    
    // strdup(pathValue);
    char* currDirectory = strtok(pathCopy, ":");
    
    // while loop that will look at each chunk of $PATH until the command's exe is found or $PATH is fully searched.
    while (currDirectory != NULL)
    {
        // reconstruct the path using the command so it can be checked-- testing each one is effectively searching
        // '+ 2' is to account for '\0' and the '/' added when concatenating
        char* fullPath = malloc(strlen(currDirectory) + strlen(command) + 2 );
        
        sprintf(fullPath, "%s/%s", currDirectory, command );    // construct full path to be tested
        
        // test path, returns fullpath if the command's path is found
        if (access(fullPath, X_OK) == 0)
        {
            free(pathCopy);
            return fullPath;
        }

        // before the next loop starts, free fullpath and grab the next chunk of $PATH
        free(fullPath);
        currDirectory = strtok(NULL, ":");
    }

    // case of nothing found, free up temp str and return NULL
    free(pathCopy);
    return NULL;
}





