#ifndef MICROL_LEXER_H
#define MICROL_LEXER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "token.h"
#define dlex_printf(...) //printf(__VA_ARGS__)

token_list_t lex(FILE *fptr)
{
	token_list_t toks = create_token_list();
	// test_token_list();
	char c;
	while((c = getc(fptr)) != EOF)
	{
		dlex_printf("Read character: '%c'\n", c);
		// Skip whitespace.
		while(isspace(c)) c = getc(fptr);
		if(c == EOF) break;
		dlex_printf("Not space\n");

		// Tokenize.
		if(c == '_' || isalpha(c))
		{
			long size = 0;
			do { c = (++size, getc(fptr)); }
			while(c == '_' || isalnum(c));
			fseek(fptr, -size - 1, SEEK_CUR);
			char *str = malloc(size + 1);
			str[size] = 0;
			fread(str, 1, size, fptr);
			add_token(&toks, (token_t){ str, tt_var });
		}
		else if(isdigit(c))
		{
			//printf("Digit first@%ld: '%c'\n", ftell(fptr), c);
			long size = 0;
			do { c = (++size, getc(fptr)); }
			while(c == '_' || isdigit(c));
			fseek(fptr, -size - 1, SEEK_CUR);
			char *str = malloc(size + 1);
			str[size] = 0;
			//printf("Size@%ld: %ld\n", ftell(fptr), size);
			fread(str, 1, size, fptr);
			//printf("fread %ld bytes -> \"%s\"\n", size, str);
			add_token(&toks, (token_t){ str, tt_num });
		}
		else if(c == '(') add_token(&toks, (token_t){ NULL, tt_open_paren });
		else if(c == ')') add_token(&toks, (token_t){ NULL, tt_close_paren });
		else if(c == '+') add_token(&toks, (token_t){ NULL, tt_add });
		else if(c == '-') add_token(&toks, (token_t){ NULL, tt_sub });
		else if(c == '*') add_token(&toks, (token_t){ NULL, tt_mul });
		else if(c == '/') add_token(&toks, (token_t){ NULL, tt_div });
		else if(c == '=') add_token(&toks, (token_t){ NULL, tt_eql });
		else
		{
			fprintf(stderr, "error: lexer -> unknown character: '%c'\n", c);
		}
		// TODO: Other symbols like ; : . = and so on.
	}
	return toks;
}

#endif//MICROL_LEXER_H
