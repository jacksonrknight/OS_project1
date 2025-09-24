#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool expandTokens(tokenlist* tokens);
char* findPath(tokenlist* tokens)



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

        // execute commands

        free_tokens(input);
        count++;
    }
    exit(0);

}


/*
expandTokens(tokenlist&)
*****
passing in a tokenlist(tokenization of the user input) will handle parts 2 and 3
- will expand token variable's symbolic representation ($USER)->(<username>), ($HOME)->(/home/username), ($SHELL)->(/bin/bash)
- will expand token tilde's symbolic representation (~)->(/home/username), (~/path/of/folders)->(/home/username/path/of/folders)
- NOTE: PROJECT DOES NOT SAY HOW TO HANDLE EXPANDING VARIABLES THAT DON'T EXIST. NULL IS RETURNED, BUT IM NOT ENTIRELY SURE IF THEY WANT AN ERROR OR TO JUST IGNORE THE NULL RETURN AND CONTINUE RUNNING--DESPITE MOST LIKELY BREAKING THE FUNCTIONALITY.
*/
char* expandToken(const char* token)
{
    char* result = NULL;    // will be the expanded string that is returned
    const char* homePath = getenv("HOME");      // temp str to store the current home path ('~' expanded)

    // token validation at start if the token passed is NULL
    if (token == NULL)
    {
        printf("ERROR expandToken(const char*): token argument is NULL\n");
        return strdup(token);
    }

    // case of Variable found
    if (token[0] == '$') 
    {
        const char* varName = token+1;            // point to the value inside the string starting AFTER the '$'
        const char* variableValue = getenv(varName);     // the expanded value of varName
        // if a valid expansion is found...
        if (variableValue != NULL)
        {
            // realloc the size of result, who's size accounts for the difference in size accounting for the '$'.. (expanding with getenv() will make it smaller or larger)
            result = (char*)realloc(result, strlen(variableValue)+1);
            strcpy(result, variableValue);
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
        return result;

    }
    // case of command token that needs to be expanded.

    // case other..
    
    return strdup(token);   // cannot return a const, creates a dummy duplicate
}

/*
char* findPath(char* token)
accepts a token and returns an expanded path for the command (ls -> /usr/bin/ls)
*****
- check for slashes at start of token './' or '/'... if it does, don't search, just verify the path is valid.
- if no slash, get the $PATH
- tokenize the $PATH using strtok() using : as a delimiter
- identify the token that matches corresponds with the command
- check the final path 

*/

char* findPath(char* token)
{
    // - check for slashes at start of token './' or '/'... if it does, don't search, just verify the path is valid.

    // - if no slash, get the $PATH
    
    // - tokenize the $PATH using strtok() using : as a delimiter
    
    // - identify the token that matches corresponds with the command
    
    // - check the final path 


}





