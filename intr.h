#ifndef MICROL_INTR_H
#define MICROL_INTR_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "ctx.h"

#define dintr_puts(...) //puts(__VA_ARGS__)
#define dintr_printf(...) //printf(__VA_ARGS__)


obj_t *eval_node(node_t *node, ctx_t *ctx)
{
	if(!node)
	{
		fprintf(stderr, "error: eval -> internal error (node is NULL).\n");
		return create_nil_obj(ctx);
	}

	switch(node->type)
	{
		case nt_num:
			dintr_printf("NUM %f\n", ((node_num_t*)node)->value);
			return create_num_obj(ctx, ((node_num_t*)node)->value);
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
				return NULL;
			}

			return v->value;
		}
		case nt_set:
		{
			node_set_t* ns = (node_set_t*)node;
			obj_t *val = eval_node(ns->value, ctx);
			get_var(ctx, ns->name)->value = val;
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
			if(lhs_o->type != ot_num)
			{
				fprintf(stderr, "error: eval -> left-hand side of binary operator is not a number!.\n");
				return NULL;
			}

			if(rhs_o->type != ot_num)
			{
				fprintf(stderr, "error: eval -> right-hand side of binary operator is not a number!.\n");
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
