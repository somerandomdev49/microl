#ifndef MICROL_INTR_H
#define MICROL_INTR_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "ctx.h"

#define dintr_puts(...) puts(__VA_ARGS__)
#define dintr_printf(...) printf(__VA_ARGS__)


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
		case nt_let: return "let";
		case nt_iff: return "iff";
		case nt_brk: return "brk";
		case nt_ret: return "for";
		case nt_dbg: return "dbg";
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


obj_t *eval_node(node_t *node, ctx_t *ctx)
{
	if(!node)
	{
		fprintf(stderr, "error: eval -> internal error (node is NULL).\n");
		return create_nil_obj(ctx);
	}

	printf("NODE: %s\n", string_node(node));

	switch(node->type)
	{
		case nt_num:
			dintr_printf("NUM %f\n", ((node_num_t*)node)->value);
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
		case nt_let:
		{
			node_let_t* nl = (node_let_t*)node;
			obj_t *val = eval_node(nl->value, ctx);
			add_var(ctx, create_var(nl->name, val));
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
		case nt_dbg:
		{
			node_dbg_t* ng = (node_dbg_t*)node;
		 	printf("[debug] -> ");
			print_obj(eval_node(ng->value, ctx));
			putc('\n', stdout);
			return NULL;
		}
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
			if(!o) return NULL;
			if(o->type == ot_nat)
			{
				if(nc->count == 0) return o->value.nat(ctx, 0, NULL);
				obj_t *args[nc->count];
				for(size_t i = 0; i < nc->count; ++i)
					args[i] = eval_node(nc->args[i], ctx);
				return o->value.nat(ctx, nc->count, args);
			}
			puts("FUNCTION CALL NON-NATIVE");

			if(o->type != ot_fun)
			{
				fprintf(stderr, "error: eval -> can only call a function!\n");
				return NULL;
			}
			if(o->value.fun.count > nc->count)
			{
				fprintf(
					stderr,
					"error: eval -> function needs %zu argument%s, but %zu %s provided!\n",
					o->value.fun.count,
					o->value.fun.count == 1 ? "" : "s",
					nc->count,
					nc->count == 1 ? "was" : "were"
				);
				return NULL;
			}
			ctx_t fctx = create_context(ctx);
			//add_var(&fctx, create_var("@", o));
			for(size_t i = 0; i < nc->count; ++i)
				add_var(&fctx, create_var(
					o->value.fun.args[i],
					eval_node(nc->args[i], ctx)));
			obj_t *v = eval_node(o->value.fun.body, &fctx);
			obj_t *c = copy_obj(ctx, v);
			free_context(&fctx);
			puts("FUNCTION RETURN NON-NATIVE");
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

			double lhs = lhs_o->value.num;
			double rhs = rhs_o->value.num;

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
		default:
			fprintf(stderr, "error: eval -> not implemented.\n");
			return NULL;
	}
}


#endif//MICROL_INTR_H
