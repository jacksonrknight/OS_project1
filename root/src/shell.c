#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* expandToken(const char* token);
// char* findPath(tokenlist* tokens);
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

        // read and user's input
        printTokenlist(input);

        // execute commands

        // printf("\n-----BEFORE FREE_TOKENS----\nTOKENLIST&= %p\nsize=%lu\n-------------------\n", &input, input->size);
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
            printf("--%i: %s\n",i, expandToken(tokens->items[i]));
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
char* expandToken(const char* inputToken)
{
    size_t inputTokenLen = strlen(inputToken);
    char* token = (char*)malloc(inputTokenLen+1);
    strcpy(token, inputToken);
    char* result = NULL;    // will be the expanded string that is returned
    char* homePath = getenv("HOME");      // temp str to store the current home path ('~' expanded)
    // printf("EXPANDTOKEN: token: %s\nstrlen(token):%ld\ninputTokenLen:%ld\n\n", token, strlen(token), inputTokenLen);

    // token validation at start if the token passed is NULL
    if (token == NULL)
    {
        printf("ERROR expandToken(const char*): token argument is NULL\n");
        return NULL;
    }

    // case of Variable found
    if (token[0] == '$') 
    {
        const char* varName = token+1;            // point to the value inside the string starting AFTER the '$'
        const char* variableValue = getenv(varName);     // the expanded value of varName
        size_t varValueLen = strlen(variableValue) +1;
        // printf("\n----EXPANDTOKEN:\n token: %s \n variableValue: %s\n-------------------\n\n", token, variableValue);
        
        // if a valid expansion is found...
        if (variableValue != NULL)
        {
            // realloc the size of result, who's size accounts for the difference in size accounting for the '$'.. (expanding with getenv() will make it smaller or larger)
            result = (char*)realloc(result, varValueLen);
            strcpy(result, variableValue);
            // printf("\n----EXPANDTOKEN:\n token: %s \n variableValue: %s\n result: %s\n-----------------\n\n", token, variableValue, result);
            free(token);
            return result;
        }
    }
    // case of tilde used symbolically
    if ( token[0] == '~' && ( token[1] == '/' || token[1] == '\n' ) )
    {
        // error to handle issues with getenv() or missing env variables
        if (homePath == NULL)
        {
            printf("ERROR expandToken(const char*): homePath is NULL\n");
            return result;
        }
        // realloc result to fit the original token (excluding the ~) and the expanded homePath + 1 for delimiter
        result = (char*)realloc(result, ( strlen(homePath)+strlen(token+1)+1 ) );
        // construct the final result
        strcpy(result, homePath);  
        strcat(result, token+1);
        free(token);
        return result;

    }
    // case of command token that needs to be expanded.

    // case other..
    
    return NULL;
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

// char* findPath(char* token)
// {
//     // char* tempToken = NULL;
//     // - check for slashes at start of token './' or '/'
//     if (token[0] == '/' || ( token[0] == '.' && token[1] == '/' ) )
//     {

//     }
//     // ... if it doesn't, don't search, just verify the path is valid
//     char* expandedPath = getenv(token);
//     if (expandedPath == NULL)
//     {
//         printf("ERROR in findPath(char*): non-path found, but returned NULL in getenv()\n");
//         return NULL;
//     }
    
//     // - if no slash and its not valid as-is, search $PATH
//     else
//     {
//     // - tokenize the $PATH using strtok() using : as a delimiter
    
//     // - identify the token that matches corresponds with the command
        
//     }
    
//     // - check the final path 

//     return token;
// }





