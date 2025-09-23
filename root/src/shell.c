#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool expandTokens(tokenlist* tokens);


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
    exit(0);*

}


/*
expandTokens(tokenlist&)
*****
passing in a tokenlist(tokenization of the user input) will handle parts 2 and 3
- will expand token variable's symbolic representation ($USER)->(<username>), ($HOME)->(/home/username), ($SHELL)->(/bin/bash)
- will expand token tilde's symbolic representation (~)->(/home/username), (~/path/of/folders)->(/home/username/path/of/folders)
- NOTE: PROJECT DOES NOT SAY HOW TO HANDLE EXPANDING VARIABLES THAT DON'T EXIST. NULL IS RETURNED, BUT IM NOT ENTIRELY SURE IF THEY WANT AN ERROR OR TO JUST IGNORE THE NULL RETURN AND CONTINUE RUNNING--DESPITE MOST LIKELY BREAKING THE FUNCTIONALITY.
*/
bool expandTokens(tokenlist* tokens)
{
    bool result = true;    // will be used as a return
    const char* homePath = getenv("HOME");    // temp str to store the current home path ('~' expanded)

    // iteratively expand variables 
    for (int i = 0; i < tokens->size; i++)
    {
        // add a temporary variable to store the current token
        char* currentToken = tokens->items[i]; 
        char* expandedToken;

        // if a '$' is found inside the current token
        if (strchr(currentToken, '$') != NULL) 
        {
            strcpy(currentToken, getenv(currentToken) );     // replace variable token with it's expanded form
        }
        // if a '~' is found and matches the requirements...
        if ( currentToken[0] == '~' && ( currentToken[1] == '/' || currentToken[1] == '\n' ) )
        {
            // start by creating a new string to hold the expanded path
            // copy the expanded homepath into expandedPath
            strcpy(expandedToken, homePath);     

            // if the token length is more than 2 (i.e it is not a lone '~') ...
            if (strlen(currentToken) > 1)
                strcat(expandedToken, (currentToken)+1);     // concatenate the remaining original string excluding the '~' element
        } 

        // this will be a catch case to see if any tokens are "ruined" in the function.
        // if a token is empty or == NULL
        if (strlen(expandedToken) == 0 || expandedToken == NULL)
        {
            printf("ERROR in expandTokens(): i=%d, tokens->items[i]=%s, strlen()=%d", i, (expandedToken), strlen(expandedToken) );
            result=false;
            break;
        }
    }


    // return T/F
    return result;
}


