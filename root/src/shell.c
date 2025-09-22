#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char** argv)
{
    while (1)
    {
        // print the shell's "prompt" using getenv() (format: USER@MACHINE:PWD>)
        printf("%s@%s:%s>", getenv("$USER"), getenv("$MACHINE"), getenv("$PWD") )
        // read and user's input
        // execute commands


    }


}