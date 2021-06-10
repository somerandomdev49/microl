#ifndef MICROL_PARSER_H
#define MICROL_PARSER_H
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "token.h"
#include "ast.h"

#define dparse_printf(...) //printf(__VA_ARGS__)
#define dparse_puts(...) //puts(__VA_ARGS__)

typedef struct
{
	token_list_t *toks;
	struct token_list_cell_t *ptr, *last;
	bool fail;
} parser_t;
char *copy_string(const char *other);

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
node_t *parse_mul(parser_t *p);
node_t *parse_add(parser_t *p);
node_t *parse_cmp(parser_t *p);
node_t *parse_eql(parser_t *p);
node_t *parse_expr(parser_t *p);


typedef struct
{
	size_t count;
	void **data;
} ptr_list_t;

ptr_list_t create_ptr_list()
{ return (ptr_list_t){ 0, malloc(0) }; }

void ptr_list_add(ptr_list_t *l, void *v)
{
	l->data = realloc(l->data, ++l->count);
	l->data[l->count - 1] = v;
}

void free_ptr_list(ptr_list_t *l, void (*free_f)(void*))
{
	if(!l) return;
	if(free_f && l->data)
		for(size_t i = 0; i < l->count; ++i)
			if(l->data[i])
			{
				// printf("wtf %s\n", (char*)l->data[i]);
				free_f(l->data[i]);
				l->data[i] = NULL;
			}
	free(l->data);
}


node_t *parse_atom(parser_t *p)
{
	dparse_puts("parse atom");
	token_t tok = p_get(p);
	dparse_printf("val: '%s'\n", tok.value);
	switch(tok.type)
	{
		// case tt_eof:
		// 	return NULL;
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
			dparse_printf(" ___ got: \"%s\"\n/\nv\n", tok.value);
			return parser_error(p, "expected an atom");
	}
}


ptr_list_t parse_cargs(parser_t *p) // cargs -> call args.
{
	ptr_list_t l = create_ptr_list();
	if(p_peek(p).type != tt_open_paren)
		return parser_error(p, "expected an opening parenthesis"), l;
	p_del(p);

	while(p_peek(p).type != tt_close_paren)
	{
		ptr_list_add(&l, parse_expr(p));
		if(p_peek(p).type == tt_close_paren) break;
		if(p_peek(p).type != tt_cma)
			return parser_error(p, "expected a comma"), l;
		p_del(p);
	}

	// if loop finishes, this error is not needed.
	// if(p_peek(p).type != tt_close_paren)
	// 		return parser_error(p, "expected a closing parenthesis"), l;
	p_del(p);
	return l;
}


node_t *parse_suffix(parser_t *p)
{
	node_t *n = parse_atom(p);
	for(;;)
	{
		if(p_peek(p).type == tt_open_paren)
		{ // <expr> <cargs>
			ptr_list_t args = parse_cargs(p);
			node_cll_t *m = create_cll_node(n, args.count, (node_t**)args.data);
			free_ptr_list(&args, NULL);
			return (node_t*)m;
		}
		else if(p_peek(p).type == tt_dot)
		{ // <expr> . <atom>
		  //            ^
		  //             \___ eval result must be a string [?] TODO
			puts("DOT!");
			p_del(p); // remove '.'
			node_t *a = parse_atom(p);
			printf("VARIABLE VALUE: '%s'\n\n", ((node_var_t*)a)->value);
			n = (node_t*)create_get_node(n, a);
		}
		else return n;
	}
}

node_t *parse_prefix(parser_t *p)
{
	// token_t tok = p_peek(p);
	// if(tok.type == tt_not)
	// {
	// 	// TODO! node_unr_t or something similar.
	// }
	/*else*/ return parse_suffix(p);
}

node_t *parse_mul(parser_t *p)
{
	node_t *lhs = parse_prefix(p);
	token_t t = p_peek(p);
	while(t.type == tt_mul || t.type == tt_div)
	{
		p_del(p);
		lhs = (node_t*)create_bin_node(
			t.type == tt_mul ? bin_op_mul : bin_op_div,
			lhs, parse_prefix(p));
		t = p_peek(p);
	}
	return lhs;
}

node_t *parse_add(parser_t *p)
{
	node_t *lhs = parse_mul(p);
	token_t t = p_peek(p);
	while(t.type == tt_add || t.type == tt_sub)
	{
		p_del(p);
		lhs = (node_t*)create_bin_node(
			t.type == tt_add ? bin_op_add : bin_op_sub,
			lhs, parse_mul(p));
		t = p_peek(p);
	}
	return lhs;
}


node_t *parse_cmp(parser_t *p)
{
	dparse_puts("parse cmp");
	node_t *lhs = parse_add(p);
	token_t t = p_peek(p);
	while(t.type == tt_grt || t.type == tt_lst)
	{
		p_del(p);
		dparse_puts("parse cmp loop");
		lhs = (node_t*)create_bin_node(
			t.type == tt_grt ? bin_op_grt : bin_op_lst,
			lhs, parse_add(p));
		t = p_peek(p);
	}
	dparse_puts("parse cmp ret");
	return lhs;
}


node_t *parse_eql(parser_t *p)
{
	node_t *lhs = parse_cmp(p);
	token_t t = p_peek(p);
	while(t.type == tt_ieq || t.type == tt_neq)
	{
		p_del(p);
		dparse_puts("parse eql loop");
		lhs = (node_t*)create_bin_node(
			t.type == tt_ieq ? bin_op_ieq : bin_op_neq,
			lhs, parse_cmp(p));
		t = p_peek(p);
	}
	dparse_puts("parse eql ret");
	return lhs;
}

node_seq_t *parse_seq(parser_t *p, token_t end, bool new)
{
	node_seq_t *seq = create_seq_node(new);
	for(;;)
	{
		token_t t = p_peek(p);
		if(t.type == end.type && strcmp(t.value, end.value) == 0)
			break;

		node_t *n = parse_expr(p);
		if(n) node_seq_add_node(seq, n);
	}
	return seq;
}

node_imp_t *parse_import(parser_t *p)
{
	return create_imp_node(p_get(p).value);
}

ptr_list_t parse_args(parser_t *p)
{
	ptr_list_t l = create_ptr_list();
	if(p_peek(p).type != tt_open_paren)
		return parser_error(p, "expected an opening parenthesis"), l;
	p_del(p);

	while(p_peek(p).type != tt_close_paren)
	{
		if(p_peek(p).type != tt_var)
			return parser_error(p, "expected an argument name"), l;
		ptr_list_add(&l, copy_string(p_get(p).value)); //
		if(p_peek(p).type == tt_close_paren) break;
		if(p_peek(p).type != tt_cma)
			return parser_error(p, "expected a comma"), l;
		p_del(p);
	}

	// if loop finishes, this error is not needed.
	// if(p_peek(p).type != tt_close_paren)
	// 		return parser_error(p, "expected a closing parenthesis"), l;
	p_del(p);
	return l;
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
				return parser_error(p, "expected the name of the variable OR \"export\".");
			
			bool exp = false;
			if(strcmp(var.value, "export") == 0)
			{
				exp = true;
				var = p_get(p);
				if(var.type != tt_var)
					return parser_error(p, "expected the name of the variable.");
			}

			dparse_printf("var: %s\n", var.value);
			if(p_peek(p).type != tt_eql)
				return parser_error(p, "expected an equals sign.");
			p_get(p);

			node_t *expr = parse_expr(p);
			return (node_t*)create_let_node(var.value, expr, exp);

		}

		if(strcmp(tok.value, "do") == 0)
		{
			// "do" seq "end"
			p_del(p); // skip "do"

			node_t *n = (node_t*)parse_seq(p, (token_t){ "end", tt_var }, true);

			p_del(p); // if parse_seq ended, the next token is "end"!
			return n;
		}

		if(strcmp(tok.value, "if") == 0)
		{
			// "if" expr "then" expr ?
			//                       \____ $
			//                       \____ "else" expr

			p_del(p); // skip "if"

			node_t *cond = parse_expr(p);

			if(strcmp(p_get(p).value, "then") != 0)
			{
				free_node(cond);
				return parser_error(p, "expected 'then'");
			}

			node_t *body = parse_expr(p);

			node_t *otherwise = NULL;
			if(strcmp(p_peek(p).value, "else") == 0)
			{
				p_del(p);
				otherwise = parse_expr(p);
			}

			return (node_t*)create_iff_node(cond, body, otherwise);
		}

		if(strcmp(tok.value, "while") == 0)
		{
			// "while" expr expr

			p_del(p); // skip "while"

			node_t *cond = parse_expr(p);
			node_t *body = parse_expr(p);

			return (node_t*)create_for_node(cond, body);
		}

		if(strcmp(tok.value, "break") == 0)
		{
			// "break"

			p_del(p); // skip "break"

			return (node_t*)create_brk_node();
		}

		// if(strcmp(tok.value, "debug") == 0)
		// {
		// 	// "break"

		// 	p_del(p); // skip "debug"
		// 	node_t *expr = parse_expr(p);

		// 	return (node_t*)create_dbg_node(expr);
		// }

		if(strcmp(tok.value, "function") == 0)
		{
			// "function" "(" args ")" expr

			p_del(p); // skip "function"
			ptr_list_t args = parse_args(p);
			node_t *expr = parse_expr(p);

			node_fun_t *f = create_fun_node(expr, args.count, (char**)args.data);
			free_ptr_list(&args, &free);
			return (node_t*)f;
		}

		if(strcmp(tok.value, "import") == 0)
		{
			// "import" name

			p_del(p); // skip "import"
			return (node_t*)parse_import(p);
		}


		if(p->ptr && p->ptr->next && p->ptr->next->token.type == tt_eql)
		{
			node_set_t *n = create_set_node(tok.value, NULL);
			p_del(p); p_del(p);
			n->value = parse_expr(p);
			return (node_t*)n;
		}
	}
	return parse_eql(p);
}

node_t *parse(parser_t *p)
{
	dparse_puts("parse");
	return (node_t*)parse_seq(p, token_eof(), false);
}

#endif//MICROL_PARSER_H
