#ifndef MICROL_INTR_H
#define MICROL_INTR_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "builtin.h"
#include "token.h"
#include "parser.h"
#include "lexer.h"
#include "ast.h"
#include "ctx.h"

#define dintr_puts(...) //puts(__VA_ARGS__)
#define dintr_printf(...) //printf(__VA_ARGS__)


char *string_node(node_t *node)
{
	if(!node)
	{
		fprintf(stderr, "error: eval str -> internal error (node is NULL).\n");
		return "(null)";
	}

	switch(node->type)
	{
		case nt_num: return "num";
		case nt_var: return "var";
		case nt_set: return "set";
		case nt_get: return "get";
		case nt_let: return "let";
		case nt_iff: return "iff";
		case nt_brk: return "brk";
		case nt_ret: return "for";
		case nt_for: return "for";
		case nt_fun: return "fun";
		case nt_cll: return "cll";
		case nt_seq: return "seq";
		case nt_bin:
		{
			node_bin_t* nb = (node_bin_t*)node;
			switch(nb->value)
			{
				case bin_op_add: return "bin_op_add";
				case bin_op_sub: return "bin_op_sub";
				case bin_op_mul: return "bin_op_mul";
				case bin_op_div: return "bin_op_div";
				case bin_op_ieq: return "bin_op_ieq";
				case bin_op_neq: return "bin_op_neq";
				case bin_op_grt: return "bin_op_grt";
				case bin_op_lst: return "bin_op_lst";
				case bin_op_lte: return "bin_op_lte";
				case bin_op_gte: return "bin_op_gte";
				default:
					fprintf(stderr, "error: eval -> bin op str not implemented.\n");
					return NULL;
			}
		}
		default:
			fprintf(stderr, "error: eval str -> not implemented.\n");
			return NULL;
	}
}


obj_t *eval_node(node_t *node, ctx_t *ctx);
ctx_t *run_file(const char *filename, bool free_all)
{
	dintr_printf("run file: %s\n", filename);
	FILE *fptr = fopen(filename, "r");
	if(!fptr)
	{
		fprintf(stderr, "error -> file: could not run '%s'.\n", filename);
		return NULL;
	}
	token_list_t toks = lex(fptr);
	fclose(fptr);

	dmain_puts("creating parser...");
	parser_t parser = create_parser(&toks);

	dmain_puts("parsing...");
	node_t *n = parse(&parser);

	dmain_puts("deleting excess tokens...");
	del_all_tokens(&toks);
	if(parser.fail)
	{
		if(n) free_node_imp(n, free_all);
		return NULL;
	}


	// TODO Variables aren't added to `ctx` itself?
	dmain_puts("context init");
	ctx_t *ctx = malloc(sizeof(ctx_t));
	*ctx = create_context(NULL);
	dmain_puts("stdlib init");
	stdlib_context(ctx);
	dmain_puts("eval");
	eval_node(n, ctx);
	
	dmain_puts("free nodes");
	if(n) free_node_imp(n, free_all);

	dmain_printf("%d variables in total\n", ctx->count);
	for(size_t i = 0; i < ctx->count; ++i)
	{
		dmain_printf("VARIABLE %s!\n", ctx->vars[i].name);
	}
	dmain_puts("-- end file eval --");
	return ctx;
}

obj_t *eval_node(node_t *node, ctx_t *ctx)
{
	if(!node)
	{
		fprintf(stderr, "error: eval -> internal error (node is NULL).\n");
		return create_nil_obj(ctx);
	}

	// printf("NODE: %s\n", string_node(node));

	switch(node->type)
	{
		case nt_num:
			//dintr_printf("NUM %f\n", ((node_num_t*)node)->value);
			return create_num_obj(ctx, ((node_num_t*)node)->value);
		case nt_var:
		{
			node_var_t* nv = ((node_var_t*)node);
			var_t *v = get_var(ctx, nv->value);

			// ? TODO: maybe didn't free something? check later!
			if(!v)
			{
				fprintf(stderr, "error: eval -> no such variable: \"%s\".\n",
					nv->value);
				return NULL;
			}
			//dintr_printf("VAR %x\n", v->value);

			return v->value;
		}
		case nt_set:
		{
			node_set_t* ns = (node_set_t*)node;
			obj_t *val = eval_node(ns->value, ctx);
			var_t *v = get_var(ctx, ns->name);
			if(!v)
			{
				fprintf(stderr, "error: eval -> no such variable: \"%s\".\n",
					ns->name);
				return NULL;
			}
			v->value = val;
			return val;
		}
		case nt_get:
		{
			node_get_t* n = (node_get_t*)node;
			dintr_printf("create_get_node -> 0x%x\n", n);
			dintr_printf("from node: %s\nval node: %s\n", string_node(n->from), string_node(n->value));
			obj_t *from = eval_node(n->from, ctx);
			obj_t *val;
			if(n->value->type == nt_var)
			{
				dintr_printf("create string object '%s'\n", ((node_var_t*)n->value)->value);
				val = create_str_obj(ctx, ((node_var_t*)n->value)->value);
			}
			else
			{
				puts("create non-string object");
				val = eval_node(n->value, ctx);
			}

			if(from->type != ot_obj)
			{
				fprintf(stderr, "error: eval -> can only access objects "
								"using dot-notation from maps.");
				// TODO:? rename to obj_map_t?
				return NULL;
			}

			dintr_printf("Looking for '%s'.\n", ((obj_str_t*)val)->str);

			if(((obj_obj_t*)from)->count == 0)
			{
				fprintf(stderr, "error: eval -> map count zero\n");
				return NULL;
			}

			dintr_printf("wow such manyes %d\n", ((obj_obj_t*)from)->count);
			dintr_printf("first 0x%x\n", ((obj_obj_t*)from)->first);

			for(obj_obj_pair_t *p = ((obj_obj_t*)from)->first; p; p = p->n)
			{
				// TODO: Object == test
				dintr_printf("wow such '%s'\n", ((obj_str_t*)p->l)->str);
				if(strcmp(((obj_str_t*)p->l)->str, ((obj_str_t*)val)->str) == 0)
				{
					dintr_puts("FOUND!");
					return p->r;
				}
			}
		
			fprintf(stderr, "error: eval -> no such obj: '");
			fprint_obj(stderr, val);
			fprintf(stderr, "'.\n");
			return NULL;
		}
		case nt_let:
		{
			node_let_t* nl = (node_let_t*)node;
			obj_t *val = eval_node(nl->value, ctx);
			add_var(ctx, create_var(nl->name, val, nl->exported));
			return val;
		}
		case nt_iff:
		{
			node_iff_t* ni = (node_iff_t*)node;
			if(obj_is_true(eval_node(ni->cond, ctx)))
				return eval_node(ni->body, ctx);
			else if(ni->otherwise)
				return eval_node(ni->otherwise, ctx);
			return NULL;
		}
		case nt_brk:
		{
			ctx_t *lctx = context_get_loop(ctx);
			if(lctx) lctx->in_loop = false;
			else fprintf(stderr, "error: eval -> break not inside loop.\n");
			return NULL;
		}
		case nt_ret:
		case nt_for:
		{
			dintr_puts("m");
			node_for_t* ni = (node_for_t*)node;
			obj_t *val;
			ctx->in_loop = 1;
			while(ctx->in_loop)
			{
				obj_t *c = eval_node(ni->cond, ctx);
				// dintr_printf("V: %f\n", c);
				if(!obj_is_true(c)) break;
				dintr_puts("while");
				val = eval_node(ni->body, ctx);
			}
			ctx->in_loop = 0;
			return val;
		}
		case nt_fun:
		{
			node_fun_t* ni = (node_fun_t*)node;
			return create_fun_obj(ctx, ni->count, ni->args, ni->value);
		}
		case nt_cll:
		{
			node_cll_t* nc = (node_cll_t*)node;
			obj_t *o = eval_node(nc->value, ctx);
			dintr_printf("CALL NODE\n");
			if(!o) return NULL;
			if(o->type == ot_nat)
			{
				if(nc->count == 0) return ((obj_nat_t*)o)->nat(ctx, 0, NULL);
				obj_t *args[nc->count];
				for(size_t i = 0; i < nc->count; ++i)
					args[i] = eval_node(nc->args[i], ctx);
				return ((obj_nat_t*)o)->nat(ctx, nc->count, args);
			}
			dintr_puts("FUNCTION CALL NON-NATIVE");

			if(o->type != ot_fun)
			{
				fprintf(stderr, "error: eval -> can only call a function!\n");
				return NULL;
			}
			if(((obj_fun_t*)o)->count > nc->count)
			{
				fprintf(
					stderr,
					"error: eval -> function needs %zu argument%s, but %zu %s provided!\n",
					((obj_fun_t*)o)->count,
					((obj_fun_t*)o)->count == 1 ? "" : "s",
					nc->count,
					nc->count == 1 ? "was" : "were"
				);
				return NULL;
			}
			ctx_t fctx = create_context(ctx);
			add_var(&fctx, create_var("@", o, false));
			for(size_t i = 0; i < nc->count; ++i)
				add_var(&fctx, create_var(
					((obj_fun_t*)o)->args[i],
					eval_node(nc->args[i], ctx),
					false));
			obj_t *v = eval_node(((obj_fun_t*)o)->body, &fctx);
			obj_t *c = copy_obj(ctx, v);
			free_context(&fctx);
			dintr_puts("FUNCTION RETURN NON-NATIVE");
			return c;
		}
		case nt_seq:
		{
			node_seq_t* nq = (node_seq_t*)node;
			obj_t *v;
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
			obj_t *lhs_o = eval_node(nb->lhs, ctx);
			obj_t *rhs_o = eval_node(nb->rhs, ctx);
			if(!lhs_o || !rhs_o) return NULL;
			if(lhs_o->type != ot_num)
			{
				fprintf(stderr, "error: eval -> left-hand side of binary operator is not a number!\n");
				return NULL;
			}

			if(rhs_o->type != ot_num)
			{
				fprintf(stderr, "error: eval -> right-hand side of binary operator is not a number!\n");
				return NULL;
			}

			double lhs = ((obj_num_t*)lhs_o)->num;
			double rhs = ((obj_num_t*)rhs_o)->num;

			switch(nb->value)
			{
				case bin_op_add: return create_num_obj(ctx, lhs + rhs);
				case bin_op_sub: return create_num_obj(ctx, lhs - rhs);
				case bin_op_mul: return create_num_obj(ctx, lhs * rhs);
				case bin_op_div: return create_num_obj(ctx, lhs / rhs);
				case bin_op_ieq: return create_num_obj(ctx, lhs == rhs);
				case bin_op_neq: return create_num_obj(ctx, lhs != rhs);
				case bin_op_grt: return create_num_obj(ctx, lhs > rhs);
				case bin_op_lst: return create_num_obj(ctx, lhs < rhs);
				case bin_op_lte: return create_num_obj(ctx, lhs <= rhs);
				case bin_op_gte: return create_num_obj(ctx, lhs >= rhs);
				default:
					fprintf(stderr, "error: eval -> bin op not implemented.\n");
					return NULL;
			}
		}
		case nt_imp: // TODO! Fix stuff here...
		{
			node_imp_t* n = (node_imp_t*)node;
			static const char prefix[] = "./stdlib/";
			static const char suffix[] = ".microl";

			size_t value_len = strlen(n->value);
			char dest[sizeof(prefix) + value_len + sizeof(suffix) + 1];
			dest[sizeof(dest)] = '\0';

			strcpy(dest, prefix);
			dest[sizeof(prefix)] = '\0';

			strcat(dest, n->value);
			dest[sizeof(prefix) + value_len] = '\0';

			strcat(dest, suffix);
			dest[sizeof(prefix) + value_len + sizeof(suffix)] = '\0';

			ctx_t *fileCtx = run_file(dest, false);
			dintr_puts("ran file");
			add_var(ctx, create_var(n->value, create_obj_obj(ctx, fileCtx), false));
			free_context(fileCtx);
			dmain_puts("freeing ctx...");
			break;
		}
		default:
			fprintf(stderr, "error: eval -> not implemented %d.\n", node->type);
			return NULL;
	}
}


#endif//MICROL_INTR_H
