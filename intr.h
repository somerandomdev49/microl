#ifndef MICROL_INTR_H
#define MICROL_INTR_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ast.h"

typedef struct
{
	char *name;
	double value;
} var_t;

/* Beware that create_var allocates
   the name and copies from the argument.
   (see free_var) */
var_t create_var(char *name, double value)
{
	size_t len = strlen(name) + 1; // include null-terminator!
	var_t v = (var_t){ malloc(len), value };
	memcpy(v.name, name, len);
	return v;
};

/* Frees the variable name. */
void free_var(var_t *var)
{ free(var->name); };

typedef struct
{
	size_t count;
	var_t *vars;
} ctx_t;

ctx_t create_context()
{ return (ctx_t){ 0, malloc(0) }; }

void free_context(ctx_t *ctx)
{
	for(size_t i = 0; i < ctx->count; ++i)
		free_var(&ctx->vars[i]);
	free(ctx->vars);
	ctx->count = 0;
}

var_t *get_var(ctx_t *ctx, char *name)
{
	for(size_t i = 0; i < ctx->count; ++i)
	{
		if(strcmp(ctx->vars[i].name, name) == 0)
			return &ctx->vars[i];
	}
	return NULL;
}


void add_var(ctx_t *ctx, var_t var)
{
	ctx->vars = realloc(ctx->vars, ++ctx->count * sizeof(var));
	ctx->vars[ctx->count - 1] = var;
}

double eval_node(node_t *node, ctx_t *ctx)
{
	if(!node)
	{
		fprintf(stderr, "error: eval -> internal error (node is NULL).\n");
		return -1;
	}

	switch(node->type)
	{
		case nt_num:
			return ((node_num_t*)node)->value;
		case nt_var:
		{
			node_var_t* nv = ((node_var_t*)node);
			var_t *v = get_var(ctx, nv->value);
			// ? TODO: maybe didn't free something? check later!
			if(!v)
			{
				fprintf(stderr, "error: eval -> no such variable: \"%s\".\n",
					nv->value);
				return -1;
			}

			return v->value;
		}
		case nt_let:
		{
			node_let_t* nl = (node_let_t*)node;
			double val = eval_node(nl->value, ctx);
			add_var(ctx, create_var(nl->name, val));
			return val;
		}
		case nt_seq:
		{
			node_seq_t* nq = (node_seq_t*)node;
			double v;
			struct node_seq_cell_t *ptr = nq->head;
			while(ptr)
			{
				v = eval_node(ptr->value, ctx);
				ptr = ptr->next;
			}
			return v;
		}
		case nt_bin:
		{
			node_bin_t* nb = (node_bin_t*)node;
			switch(nb->value)
			{
				case bin_op_add:
					return eval_node(nb->lhs, ctx) +
						   eval_node(nb->rhs, ctx);
				case bin_op_mul:
					return eval_node(nb->lhs, ctx) *
						   eval_node(nb->rhs, ctx);
				default:
					fprintf(stderr, "error: eval -> bin op not implemented.\n");
					return -1;
			}
		}
		default:
			fprintf(stderr, "error: eval -> not implemented.\n");
			return -1;
	}
}


#endif//MICROL_INTR_H
