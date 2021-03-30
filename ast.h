#ifndef MICROL_AST_H
#define MICROL_AST_H
#include <stdlib.h>
#define alloc_node(x) malloc(sizeof(node_##x##_t))

#define dast_printf(...) //printf(__VA_ARGS__)
#define dast_puts(...) //puts(__VA_ARGS__)

typedef enum
{
	nt_num, nt_var, nt_str,
	nt_bin, nt_let, nt_seq,
	nt_set, nt_iff, nt_for,
	nt_brk, nt_ret, nt_cll,
	nt_fun, // TODO!
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

	size_t len = strlen(value) + 1; // null-terminator included!
	n->value = malloc(len);
	memcpy(n->value, value, len);
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

// TODO: Bug, needed to switch rhs/lhs: check what is wrong...
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

	size_t len = strlen(name) + 1; // null-terminator included!
	n->name = malloc(len);
	memcpy(n->name, name, len);
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
		case nt_num: free(node); break;
		case nt_brk: free(node); break;
		case nt_var:
		{
			node_var_t* nv = (node_var_t*)node;
			free(nv->value);
			free(node);
			break;
		}
		case nt_seq:
		{
			node_seq_t* nl = (node_seq_t*)node;
			node_seq_del_all(nl);
			free(node);
			break;
		}
		case nt_iff:
		{
			node_iff_t* ni = (node_iff_t*)node;
			free_node(ni->cond);
			free_node(ni->body);
			free_node(ni->otherwise);
			free(node);
			break;
		}
		case nt_for:
		{
			node_for_t* nr = (node_for_t*)node;
			free_node(nr->cond);
			free_node(nr->body);
			free(node);
			break;
		}
		case nt_let:
		{
			node_let_t* nl = (node_let_t*)node;
			free(nl->name);
			free_node(nl->value);
			free(node);
			break;
		}
		case nt_set:
		{
			node_set_t* nl = (node_set_t*)node;
			free(nl->name);
			free_node(nl->value);
			free(node);
			break;
		}
		case nt_bin:
		{
			node_bin_t* nb = (node_bin_t*)node;
			free_node(nb->lhs); free_node(nb->rhs);
			free(node);
			break;
		}
		default:
			fprintf(stderr, "error: free -> not implemented %d.\n", node->type);
			break;
	}
}

#endif//MICROL_AST_H
