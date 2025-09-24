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

    return strdup(token);   // cannot return a const, creates a dummy duplicate
}





// bool expandTokens(tokenlist* tokens)
// {
//     bool result = true;    // will be used as a return

//     // iteratively expand variables 
//     for (int i = 0; i < tokens->size; i++)
//     {
//         // add a temporary variable to store the current token
//         char* currentToken = tokens->items[i]; 
//         char* newToken;

//         //--------- PT2 EXPANDING VARIABLES -------------
//         // if a '$' is the beginning of the token, then it's treated as a variable to be expanded
//         if (currentToken[0] == '$') 
//         {
//             char* varName = currentToken+1;            // point to the value inside the string starting AFTER the '$'
//             char* variableValue = getenv(varName);     // the return value of getenv for the variableName just derived
//             // if a valid expansion is found...
//             if (variableValue != NULL)
//             {
//                 // declare and allocate a temp variable to store the new token who's size accounts for the difference in size accounting for the '$'.. (expanding with getenv() will make it smaller or larger)
//                 char* tempToken = (char*)realloc(currentToken, strlen(variableValue)+1);
//                 // point to the new memory location
//                 tokens->items[i] = tempToken;
//                 // copy the token into the tokenlist
//                 strcpy(tokens->items[i], tempToken);
//                 // update the newtoken
//                 newToken = tokens->items[i];
                
//             }
//             // strcpy(currentToken, getenv(variableName) );     // replace variable token with it's expanded form
//         }


//         //-------- PT3 EXPANDING TILDE -------------------
//         // if a '~' is found and matches the requirements...
//         if ( currentToken[0] == '~' && ( currentToken[1] == '/' || currentToken[1] == '\n' ) )
//         {
//             // start by creating a new string to hold the expanded path
//             // copy the expanded homepath into expandedPath
//             strcpy(newToken, homePath);     

//             // if the token length is more than 2 (i.e it is not a lone '~') ...
//             if (strlen(currentToken) > 1)
//                 strcat(newToken, (currentToken)+1);     // concatenate the remaining original string excluding the '~' element
//         } 

//         // this will be a catch case to see if any tokens are "ruined" in the function.
//         // if a token is empty or == NULL
//         if (strlen(newToken) == 0 || newToken == NULL)
//         {
//             printf("ERROR in expandTokens(): i=%d, tokens->items[i]=%s, strlen()=%d", i, (newToken), strlen(newToken) );
//             result=false;
//             break;
//         }
//     }


//     // return T/F
//     return result;
// }


