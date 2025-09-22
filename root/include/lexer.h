#pragma once

#include <stdlib.h>
#include <stdbool.h>

/*
---tokenlist object---
*
data:
- char ** items;
- size_t size;
*
Functions:
- char* get_input(void) -- grabs an input from stdin and returns it as a string
- tokenlist* get_tokens(char* input) -- constructs a tokenlist from an input
- tokenlist* new_tokenlist(void) -- constructs an empty tokenlist
- void add_token(tokenlist* tokens, char* item) -- adds string "item" to tokenlist "tokens"
- void free_token(tokenlist* tokens) -- deallocates "tokens" recursively
*/
typedef struct {
    char ** items;
    size_t size;
} tokenlist;

char * get_input(void);
tokenlist * get_tokens(char *input);
tokenlist * new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);
