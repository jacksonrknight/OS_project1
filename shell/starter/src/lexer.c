#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	while (1) {
		printf("> ");

		/* input contains the whole command
		 * tokens contains substrings from input split by spaces
		 */
		char *input = get_input();			// expects user input to stdin and extracts tokens from stdin to string
		printf("whole input: %s\n", input);		// prints the user input grabbed to stdout

		tokenlist *tokens = get_tokens(input);	// creates a new token object, then 
		for (int i = 0; i < tokens->size; i++) {
			printf("token %d: (%s)\n", i, tokens->items[i]);
		}

		// deacllocate input and recursively deallocate tokenlist
		free(input);
		free_tokens(tokens);
	}

	return 0;
}

/*
This function prompts the user for an input in stdin, stores it in a buffer, then returns the buffer after stdin is full extracted OR if a '\n' is found

what it does: 
- initialize empty temp buffer "buffer" and the var to store the buffer size "bufsize"
- initialize char array "line" of size 5 (line[5]) 

- while loop to grab 5 character tokens at a time to "line" until a '\n' is found
- after while loop, reallocate the buffer to match the size including the grabbed tokens
- return buffer
*/
char *get_input(void) {
	char *buffer = NULL;
	int bufsize = 0;
	char line[5];	
	// 5 characters at a time from stdin to line, returns a ptr to line's location in memory
	while (fgets(line, 5, stdin) != NULL)
	{
		int addby = 0;	// will be used to temp hold the # of tokens being added. used for recalculating/reallocating the new buffer size.
		char *newln = strchr(line, '\n');	// returns ptr to where '\n' is found in memory. (returns null if not found)

		// if '\n' is found 
		if (newln != NULL)
			addby = newln - line;	// if '\n' is found, by subtracting the location of at end of the string to the location of the string, the result is the size in memory of the string
		else
			addby = 5 - 1;	// if newline is NOT found, decrement the size of the string being added 1 (\n is not included in the buffer)
		buffer = (char *)realloc(buffer, bufsize + addby);	// reallocate the size of the buffer to fit the new line of tokens
		memcpy(&buffer[bufsize], line, addby);				// copy the data from line to the buffer (using addby to exclude '\n' or account for smaller "line")
		bufsize += addby; 		// update the buffer size variable accounting for the new set of tokens
		// if '\n' is found, then exit while extraction loop
		if (newln != NULL)
			break;
	}
	buffer = (char *)realloc(buffer, bufsize + 1); 	// reallocate the buffer using the updated buffer size var
	buffer[bufsize] = 0;	// set the last element to 0
	return buffer;
}

/*
tokenlist* new_tokenlist(void)
- instatiate, then allocate memory for an object tokenlist* "tokens"
- initialize tokens->size to 0
- allocate space for the object's array of strings "tokens->items"
- null terminate tokens->items
*/
tokenlist *new_tokenlist(void) {
	tokenlist *tokens = (tokenlist *)malloc(sizeof(tokenlist));
	tokens->size = 0;			// initilize size to 0
	tokens->items = (char **)malloc(sizeof(char *));	// allocate space for the object's array of strings "items"
	tokens->items[0] = NULL; /* make tail element NULL terminated */
	return tokens;	
}

/*
-- accepts a ptr to tokenlist obj "tokens", and accepts a char* (string) "item"
- temp store the token's current size into i -- will be used as a reference for adding new tokens (the size-1 is also the location of the end of array)
- reallocate the the ENTIRE tokenlist's array to include another token string
- allocate a chunk INSIDE the array for the token string -- space in the overall array already allocated, but this assigns a specific size to the specific element
- create a new NULL tail element after the newly allocated space
- copy the string item into the newly allocated element
- increment tokens->size

*/
void add_token(tokenlist *tokens, char *item) {
	int i = tokens->size;

	tokens->items = (char **)realloc(tokens->items, (i + 2) * sizeof(char *));
	tokens->items[i] = (char *)malloc(strlen(item) + 1);
	tokens->items[i + 1] = NULL;
	strcpy(tokens->items[i], item);

	tokens->size += 1;
}

/*
-- char* (string) 'input' accepted as the argument
- allocate a buffer sized (strlen(input) + 1), copy the string to the buffer.
- instantiate tokenlist object "tokens"
- create a temporary char array and iteratively grab each token of the original input deliminated by a " "
- copy each token from the temp char array to the tokenlist's array
- free the temp buffer
- return the tokenlist "tokens"
*/
tokenlist *get_tokens(char *input) {
	char *buf = (char *)malloc(strlen(input) + 1);	 // allocate a buffer
	strcpy(buf, input);						// copy input into buffer
	tokenlist *tokens = new_tokenlist();	// instantiate an empty tokenlist obj
	
	char *tok = strtok(buf, " ");		// Create a temporary char array "tok" containing each token of the original input deliminated by a " "	
	// add each individual token to the token list until no more tokens remain in temp array "tok"
	while (tok != NULL)	
	{
		add_token(tokens, tok);		// add token to tokenlist object
		tok = strtok(NULL, " ");	// replace tokens with null characters
	}
	free(buf);		// free up the space used by the buffer
	return tokens;
}


/*
-- accepts a tokenlist object as an argument
- recursively free tokens from the obj

*/
void free_tokens(tokenlist *tokens) {
	// for each token inside the tokenlist object's "char** items" array, free each token
	for (int i = 0; i < tokens->size; i++)
		free(tokens->items[i]);
	free(tokens->items);	// free the array holding the tokens
	free(tokens);		// free the tokenlist object
}
