#ifndef MICROL_AST_H
#define MICROL_AST_H
#include <stdlib.h>
#define alloc_node(x) malloc(sizeof(node_##x##_t))

#define dast_printf(...) //printf(__VA_ARGS__)
#define dast_puts(...) //puts(__VA_ARGS__)

char *copy_string(const char *other)
{
	if(!other) return NULL;
	size_t len = strlen(other) + 1; // null-terminator included!
	char *s = malloc(len);
	memcpy(s, other, len);
	return s;
}

typedef enum
{
	nt_num, nt_var, nt_str,
	nt_bin, nt_let, nt_seq,
	nt_set, nt_iff, nt_for,
	nt_brk, nt_ret, nt_cll,
	nt_fun, nt_dbg, nt_imp,
} node_type_t;

typedef struct
{
	node_type_t type;
}  node_t;

void free_node(node_t *node);



// NUM NODE //

typedef struct
{
	node_t node;
	double value;
} node_num_t;

node_num_t *create_num_node(double value)
{
	dast_printf("create num node %f\n", value);
	node_num_t *n = alloc_node(num);
	n->node.type = nt_num;
	n->value = value;
	dast_puts("create num node ret");
	return n;
}




// VAR NODE //

typedef struct
{
	node_t node;
	char *value;
} node_var_t;

node_var_t *create_var_node(char *value)
{
	node_var_t *n = alloc_node(var);
	n->node.type = nt_var;
	n->value = copy_string(value);
	return n;
}




// IMP NODE //

typedef struct
{
	node_t node;
	char *value;
} node_imp_t;

node_imp_t *create_imp_node(char *value)
{
	node_imp_t *n = alloc_node(var);
	n->node.type = nt_imp;
	n->value = copy_string(value);
	return n;
}



// BIN NODE //

typedef struct
{
	node_t node;
	node_t *lhs, *rhs;
	enum
	{
		bin_op_add,
		bin_op_sub,
		bin_op_mul,
		bin_op_div,
		bin_op_mod,
		bin_op_and,
		bin_op_cor,
		bin_op_ieq,
		bin_op_neq,
		bin_op_grt,
		bin_op_lst,
		bin_op_gte,
		bin_op_lte,
	} value;
} node_bin_t;

node_bin_t *create_bin_node(char op, node_t *lhs, node_t *rhs)
{
	node_bin_t *n = alloc_node(bin);
	n->node.type = nt_bin;
	n->lhs = lhs; n->rhs = rhs;
	n->value = op;
	return n;
}




// LET NODE //

typedef struct
{
	node_t node;
	char *name;
	node_t *value;
} node_let_t;

node_let_t *create_let_node(char *name, node_t *value)
{
	node_let_t *n = alloc_node(let);
	n->node.type = nt_let;
	n->value = value;
	n->name = copy_string(name);
	return n;
}




// IFF NODE //

typedef struct
{
	node_t node;
	node_t *cond, *body, *otherwise;
} node_iff_t;

node_iff_t *create_iff_node(node_t *cond, node_t *body, node_t *otherwise)
{
	node_iff_t *n = alloc_node(iff);
	n->node.type = nt_iff;
	n->cond = cond;
	n->body = body;
	n->otherwise = otherwise;
	return n;
}




// FOR NODE //

typedef struct
{
	node_t node;
	node_t *cond, *body;
} node_for_t;

node_for_t *create_for_node(node_t *cond, node_t *body)
{
	node_for_t *n = alloc_node(for);
	n->node.type = nt_for;
	n->cond = cond;
	n->body = body;
	return n;
}



// BRK NODE //

typedef struct
{
	node_t node;
} node_brk_t;

node_brk_t *create_brk_node()
{
	node_brk_t *n = alloc_node(brk);
	n->node.type = nt_brk;
	return n;
}



// RET NODE //

typedef struct
{
	node_t node;
	node_t *value;
} node_ret_t;

node_ret_t *create_ret_node(node_t *value)
{
	node_ret_t *n = alloc_node(ret);
	n->node.type = nt_ret;
	n->value = value;
	return n;
}




// DBG NODE //

typedef struct
{
	node_t node;
	node_t *value;
} node_dbg_t;

node_dbg_t *create_dbg_node(node_t *value)
{
	node_dbg_t *n = alloc_node(dbg);
	n->node.type = nt_dbg;
	n->value = value;
	return n;
}



// FUN NODE //

typedef struct
{
	node_t node;
	node_t *value;
	size_t count;
	char *args[];
} node_fun_t;

node_fun_t *create_fun_node(node_t *value, size_t count, char *args[])
{
	node_fun_t *n = malloc(sizeof(node_fun_t) + count * sizeof(char*));
	n->node.type = nt_fun;
	n->value = value;
	n->count = count;
	for(size_t i = 0; i < n->count; ++i)
		n->args[i] = copy_string(args[i]);
	return n;
}



// CLL NODE //


typedef struct
{
	node_t node;
	node_t *value;
	size_t count;
	node_t *args[];
} node_cll_t;

node_cll_t *create_cll_node(node_t *value, size_t count, node_t *args[])
{
	node_cll_t *n = malloc(sizeof(node_cll_t) + count * sizeof(char*));
	n->node.type = nt_cll;
	n->value = value;
	n->count = count;
	for(size_t i = 0; i < n->count; ++i)
		n->args[i] = args[i];
	return n;
}


// SET NODE //

typedef struct
{
	node_t node;
	char *name;
	node_t *value;
} node_set_t;

node_set_t *create_set_node(char *name, node_t *value)
{
	node_set_t *n = alloc_node(set);
	n->node.type = nt_set;
	n->value = value;

	size_t len = strlen(name) + 1; // null-terminator included!
	n->name = malloc(len);
	memcpy(n->name, name, len);
	return n;
}



// GET NODE //

typedef struct
{
	node_t node;
	char *name;
	node_t *value;
} node_get_t;

node_set_t *create_set_node(char *name, node_t *value)
{
	node_set_t *n = alloc_node(set);
	n->node.type = nt_set;
	n->value = value;

	size_t len = strlen(name) + 1; // null-terminator included!
	n->name = malloc(len);
	memcpy(n->name, name, len);
	return n;
}



// SEQ NODE //

typedef struct
{
	node_t node;
	bool new;
	struct node_seq_cell_t
	{
		node_t *value;
		struct node_seq_cell_t *next;
	} *head, *tail;
} node_seq_t;

node_seq_t *create_seq_node(bool new)
{
	node_seq_t *n = alloc_node(seq);
	n->node.type = nt_seq;
	n->head = NULL;
	n->tail = NULL;
	n->new = new;
	return n;
}

void node_seq_add_node(node_seq_t *seq, node_t *node)
{
	struct node_seq_cell_t *cell = malloc(sizeof(struct node_seq_cell_t));
	cell->value = node;
	cell->next = NULL;
	if(seq->tail == NULL)
	{
		seq->head = seq->tail = cell;
	}
	else
	{
		seq->tail->next = cell;
		seq->tail = cell;
	}
}

void node_seq_del_node(node_seq_t *seq)
{
	struct node_seq_cell_t *tmp = seq->head;
	seq->head = seq->head->next;
	free_node(tmp->value);
	free(tmp);
}

void node_seq_del_all(node_seq_t *seq)
{
	while(seq->head)
		node_seq_del_node(seq);
	seq->tail = NULL;
}




// GENERAL //

void free_node(node_t *node)
{
	if(!node) return;
	switch(node->type)
	{
		case nt_num:
		case nt_brk: break;

		case nt_ret:
		case nt_dbg:
		{
			node_dbg_t *n = (node_dbg_t*)node;
			free_node(n->value); n->value = NULL;
			break;
		}
		case nt_var:
		{
			node_var_t *n = (node_var_t*)node;
			free(n->value);
			break;
		}
		case nt_imp:
		{
			node_imp_t *n = (node_imp_t*)node;
			free(n->value);
			break;
		}
		case nt_seq:
		{
			node_seq_t *n = (node_seq_t*)node;
			node_seq_del_all(n);
			break;
		}
		case nt_iff:
		{
			node_iff_t *n = (node_iff_t*)node;
			free_node(n->cond); n->cond = NULL;
			free_node(n->body); n->body = NULL;
			free_node(n->otherwise); n->otherwise = NULL;
			break;
		}
		case nt_for:
		{
			node_for_t *n = (node_for_t*)node;
			free_node(n->cond); n->cond = NULL;
			free_node(n->body); n->body = NULL;
			break;
		}
		case nt_let:
		{
			node_let_t *n = (node_let_t*)node;
			free(n->name);
			free_node(n->value); n->value = NULL;
			break;
		}
		case nt_fun:
		{
			node_fun_t *n = (node_fun_t*)node;
			free_node(n->value); n->value = NULL;
			for(size_t i = 0; i < n->count; ++i)
				free(n->args[i]);
			break;
		}
		case nt_cll:
		{
			node_cll_t *n = (node_cll_t*)node;
			free_node(n->value); n->value = NULL;
			for(size_t i = 0; i < n->count; ++i)
			{
				free_node(n->args[i]);
				n->args[i] = NULL;
			}
			break;
		}
		case nt_set:
		{
			node_set_t *n = (node_set_t*)node;
			free(n->name);
			free_node(n->value); n->value = NULL;
			break;
		}
		case nt_bin:
		{
			node_bin_t *n = (node_bin_t*)node;
			free_node(n->lhs); n->lhs = NULL;
			free_node(n->rhs); n->rhs = NULL;
			break;
		}
		default:
			fprintf(stderr, "error: free -> not implemented %d.\n", node->type);
			break;
	}
	free(node);
}

#endif//MICROL_AST_H
