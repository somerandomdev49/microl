#ifndef MICROL_PARSER_H
#define MICROL_PARSER_H
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "token.h"
#include "ast.h"

#define dparse_printf(...) //printf(__VA_ARGS__)
#define dparse_puts(...) //puts(__VA_ARGS__)
#define bool char

typedef struct
{
	token_list_t *toks;
	struct token_list_cell_t *ptr, *last;
	bool fail;
} parser_t;

parser_t create_parser(token_list_t *toks)
{
	parser_t p = (parser_t){ toks };
	p.last = NULL;
	p.ptr = toks->head;
	return p;
}

token_t p_peek(parser_t *p)
{
	struct token_list_cell_t *h = p->ptr;
	return h == NULL ? token_eof() : h->token;
}

token_t p_get(parser_t *p)
{
	token_t tmp = p_peek(p);
	dparse_puts("GET");
	if(p->ptr)
	{
		p->last = p->ptr;
		p->ptr = p->ptr->next;
	}
	dparse_puts("hmm");
	dparse_printf("val: '%s'\n", tmp.value);
	return tmp;
}

void p_del(parser_t *p)
{
	if(p->ptr) p->ptr = p->ptr->next;
}

node_t *parser_error(parser_t *p, char *msg)
{
	fprintf(stderr, "error: parser -> syntax error: \"%s\".\n", msg);
	fprintf(stderr, "last token: \"%s\"\n", p->last->token.value);
	p->fail = 1;
	return NULL;
}


node_t *parse_atom(parser_t *p);
node_t *parse_expr(parser_t *p);
node_t *parse_mul(parser_t *p);
node_t *parse_add(parser_t *p);

node_t *parse_atom(parser_t *p)
{
	dparse_puts("parse atom");
	token_t tok = p_get(p);
	dparse_printf("val: '%s'\n", tok.value);
	switch(tok.type)
	{
		case tt_num:
			dparse_printf("parse atom ret \"%s\"\n", tok.value);
			return (node_t*)create_num_node(atoi(tok.value));
		case tt_var:
			return (node_t*)create_var_node(tok.value);
		case tt_open_paren:
		{
			node_t *e = parse_expr(p);
			if(p_get(p).type != tt_close_paren)
			{
				free_node(e); // leak!
				return parser_error(p, "expected a closing parenthesis");
			}
			return e;
		}
		default:
			dparse_puts("parse atom err");
			return parser_error(p, "expected an atom");
	}
}

node_t *parse_mul(parser_t *p)
{
	dparse_puts("parse mul");
	node_t *lhs = parse_atom(p);
	dparse_puts("yahhaa im stoopid");
	while(p_peek(p).type == tt_mul)
	{
		// token_t n = p_get(p);
		p_del(p);
		dparse_puts("parse mul loop");
		node_t *nn = parse_atom(p);
		dparse_puts("parse mul nn");
		lhs = (node_t*)create_bin_node(bin_op_mul, lhs, nn);
	}
	dparse_puts("parse mul ret");
	return lhs;
}

node_t *parse_add(parser_t *p)
{
	dparse_puts("parse add");
	node_t *lhs = parse_mul(p);
	while(p_peek(p).type == tt_add)
	{
		// token_t n = p_get(p);
		p_del(p);
		dparse_puts("parse add loop");
		lhs = (node_t*)create_bin_node(bin_op_add, lhs, parse_mul(p));
	}
	dparse_puts("parse add ret");
	return lhs;
}

node_t *parse_expr(parser_t *p)
{
	token_t tok = p_peek(p);
	dparse_printf("tok: %d:'%s'\n", tok.type, tok.value);
	if(tok.type == tt_eof) return NULL;
	if(tok.type == tt_var)
	{
		if(strcmp(tok.value, "let") == 0)
		{
			// "let" var "=" expr.
			p_del(p); // skip "let"
			token_t var = p_get(p);
			if(var.type != tt_var)
				return parser_error(p, "expected the name of the variable.");
			if(p_get(p).type != tt_eql)
				return parser_error(p, "expected an equals sign.");

			node_t *expr = parse_expr(p);
			return (node_t*)create_let_node(var.value, expr);

		}
	}
	return parse_add(p);
}

node_t *parse(parser_t *p)
{
	dparse_puts("parse");
	node_seq_t *seq = create_seq_node();
	while(p_peek(p).type != tt_eof)
	{
		node_t *n = parse_expr(p);
		if(n) node_seq_add_node(seq, n);
	}
	return (node_t*)seq;
}

#endif//MICROL_PARSER_H
