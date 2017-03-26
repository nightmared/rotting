#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "token.h"


/*
// Hope the compiler inlines the function, I don't want to use gcc extension 'always_inline'
size_t min(size_t a, size_t b) {
    if (a>b)
        return b;
    else
        return a;
}
*/

struct token* new_token(void) {
	struct token *tok = malloc(sizeof(struct token));
	if (!tok)
		return NULL;
	tok->tokens = NULL;
	tok->name = NULL;
	tok->value = NULL;
    tok->props = NULL;
    tok->tokens = malloc(sizeof(struct token_list));
    memset(tok->tokens, 0, sizeof(struct token_list));
	return tok;
}


void free_token(struct token *tok) {
	if (!tok)
		return;

	// free all subtokens
	free_token_list(tok->tokens);

	free(tok->name);
	free(tok->value);
// WARNING: DOUBLE free !	free(tok->props);
	free(tok);
}

// expected to start with '<token>'
struct token* get_token(char* src) {
    // get token name
	size_t count = 0;

    // closing token or description section, shouldn't handle it
    if (*(src+1) == '/' || *(src+1) == '!') {
        return NULL;
    }

	while (*(src+count) != '>')
        count++;

    struct token* tok = new_token();
    tok->name = malloc(sizeof(char)*(count));
    memcpy(tok->name, src+1, count-1);
    for (int i = 0; i < strlen(tok->name) - 1; i++) {
        if (tok->name[i] == ' ') {
            tok->name[i] = '\0';
            tok->props = tok->name+i+1;
            break;
        }
    }
    tok->name[count-1] = '\0';

    if (tok->name[count-2] == '/') {
        tok->name[count-2] = '\0';
        return tok;
    }

    // get token value
    char *pos = src + count + 1;
    count = 0;
    while (*(pos+count) != '\0') {
        char *cur = pos + count;
        count++;
        if (*cur == '<') {
            // check if this is the closing tag of our token (and not another one)            
            if (*(cur+1) == '/') {
                 if (strncmp(cur+2, tok->name, strlen(tok->name))) 
                    continue;

                tok->value = malloc(sizeof(char)*count);
                memcpy(tok->value, pos, count-1);
                tok->value[count-1] = '\0';
                return tok;
            } else {
                // new tag encountered, added as a subtoken
                struct token *subtok = get_token(cur);
                if (!subtok)
                    continue;

                struct token_list *old_next = tok->tokens;
                tok->tokens = malloc(sizeof(struct token));
                tok->tokens->tok = subtok;
                tok->tokens->next = old_next;
            }
        }
    }

	return NULL;
}


int cmp_token_to_name(struct token* tok, char* name) {
    if (!tok)
        return -1;

    return strncmp(tok->name, name, strlen(name));
}

void add_token_to_list(struct token_list *l, struct token* tok) {
    if (!l)
        return;

    struct token_list* next = l;
    while (next->next != NULL)
        next = next->next;
    
    if (!next->tok) {
       next->tok = tok; 
    } else {
        next->next = malloc(sizeof(struct token_list));
        next->next->tok = tok;
        next->next->next = NULL;
    }
}

void free_token_list(struct token_list *l) {
	if (!l)
		return;

	struct token_list* next = l;
	struct token_list* prev = l;
	while (next) {
		prev = next;
		next = next->next;
		free_token(prev->tok);
		free(prev);
	}

}
