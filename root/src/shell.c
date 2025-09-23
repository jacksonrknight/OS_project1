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
*/
bool expandTokens(tokenlist* tokens)
{
    // expand variables


    // expand tilde

    
    // return T/F
}


