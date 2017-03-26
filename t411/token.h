#ifndef TOKEN_H
#define TOKEN_H

struct token {
	char* name;
	char* value;
    char* props;
	struct token_list* tokens;
};


struct token* new_token(void);
void free_token(struct token *tok);
struct token* get_token(char* src);
// same semantics as strcmp
int cmp_token_to_name(struct token* tok, char* name);

struct token_list {
	struct token* tok;
	struct token_list* next;
};

void free_token_list(struct token_list *l);
void add_token_to_list(struct token_list *l, struct token* tok);

#endif
