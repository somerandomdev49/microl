#ifndef MICROL_INTR_H
#define MICROL_INTR_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ast.h"

#define dintr_puts(...) //puts(__VA_ARGS__)
#define dintr_printf(...) //printf(__VA_ARGS__)

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

typedef struct ctx_t ctx_t;
struct ctx_t
{
	size_t count;
	var_t *vars;
	ctx_t *parent;
	bool in_loop;
};

ctx_t create_context(ctx_t *parent)
{ return (ctx_t){ 0, malloc(0), parent, 0 }; }

void free_context(ctx_t *ctx)
{
	for(size_t i = 0; i < ctx->count; ++i)
		free_var(&ctx->vars[i]);
	free(ctx->vars);
	ctx->count = 0;
}

ctx_t *context_get_loop(ctx_t *ctx)
{
	if(ctx->in_loop)
		return ctx;
	if(ctx->parent)
		return context_get_loop(ctx->parent);
	return NULL;
}

var_t *get_var(ctx_t *ctx, char *name)
{
	for(size_t i = 0; i < ctx->count; ++i)
		if(strcmp(ctx->vars[i].name, name) == 0)
			return &ctx->vars[i];
	if(ctx->parent)
		return get_var(ctx->parent, name);
	return NULL;
}


void add_var(ctx_t *ctx, var_t var)
{
	for(size_t i = 0; i < ctx->count; ++i)
		if(strcmp(ctx->vars[i].name, var.name) == 0)
		{
			fprintf(stderr, "variable with name \"%s\" already exists.\n", var.name);
			return;
		}
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
			dintr_printf("NUM %f\n", ((node_num_t*)node)->value);
			return ((node_num_t*)node)->value;
		case nt_var:
		{
			node_var_t* nv = ((node_var_t*)node);
			var_t *v = get_var(ctx, nv->value);
			dintr_printf("VAR %f\n", v->value);

			// ? TODO: maybe didn't free something? check later!
			if(!v)
			{
				fprintf(stderr, "error: eval -> no such variable: \"%s\".\n",
					nv->value);
				return -1;
			}

			return v->value;
		}
		case nt_set:
		{
			node_set_t* ns = (node_set_t*)node;
			double val = eval_node(ns->value, ctx);
			get_var(ctx, ns->name)->value = val;
			return val;
		}
		case nt_let:
		{
			node_let_t* nl = (node_let_t*)node;
			double val = eval_node(nl->value, ctx);
			add_var(ctx, create_var(nl->name, val));
			return val;
		}
		case nt_iff:
		{
			node_iff_t* ni = (node_iff_t*)node;
			if(eval_node(ni->cond, ctx))
				return eval_node(ni->body, ctx);
			else if(ni->otherwise)
				return eval_node(ni->otherwise, ctx);
			return -1;
		}
		case nt_brk:
		{
			ctx_t *lctx = context_get_loop(ctx);
			if(lctx) lctx->in_loop = false;
			else fprintf(stderr, "error: eval -> break not inside loop.\n");
			return -1;
		}
		case nt_ret:
		case nt_dbg:
		{
			node_dbg_t* ng = (node_dbg_t*)node;
		 	printf("[debug] -> %f\n", eval_node(ng->value, ctx));
			return -1;
		}
		case nt_for:
		{
			dintr_puts("m");
			node_for_t* ni = (node_for_t*)node;
			double val;
			ctx->in_loop = 1;
			while(ctx->in_loop)
			{
				double c = eval_node(ni->cond, ctx);
				dintr_printf("V: %f\n", c);
				if(!c) break;
				dintr_puts("while");
				val = eval_node(ni->body, ctx);
			}
			ctx->in_loop = 0;
			return val;
		}
		case nt_seq:
		{
			node_seq_t* nq = (node_seq_t*)node;
			double v;
			struct node_seq_cell_t *ptr = nq->head;
			ctx_t *ctxp = ctx;
			ctx_t ctxq;

			if(nq->new)
			{
				ctxq = create_context(ctx);
				ctxp = &ctxq;
			}

			while(ptr)
			{
				v = eval_node(ptr->value, ctxp);
				ptr = ptr->next;
			}
			if(nq->new)
				free_context(&ctxq);
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
				case bin_op_sub:
					return eval_node(nb->lhs, ctx) -
						   eval_node(nb->rhs, ctx);
				case bin_op_div:
					return eval_node(nb->lhs, ctx) /
						   eval_node(nb->rhs, ctx);
				case bin_op_ieq:
					return eval_node(nb->lhs, ctx) ==
						   eval_node(nb->rhs, ctx);
				case bin_op_neq:
					return eval_node(nb->lhs, ctx) !=
						   eval_node(nb->rhs, ctx);
				case bin_op_grt:
					dintr_puts("grt");
					return eval_node(nb->lhs, ctx) >
						   eval_node(nb->rhs, ctx);
				case bin_op_lst:
					dintr_puts("less");
					return eval_node(nb->lhs, ctx) <
						   eval_node(nb->rhs, ctx);
				case bin_op_lte:
					return eval_node(nb->lhs, ctx) >=
						   eval_node(nb->rhs, ctx);
				case bin_op_gte:
					return eval_node(nb->lhs, ctx) <=
						   eval_node(nb->rhs, ctx);
				//

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
