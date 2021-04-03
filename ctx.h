#ifndef MICROL_CTX_H
#define MICROL_CTX_H
#include "ast.h"

char *copy_string(const char *other);

typedef struct obj_t obj_t;
typedef struct ctx_t ctx_t;

struct obj_t
{
	enum
	{
		ot_num, ot_str, ot_nil,
		ot_fun, ot_obj, ot_arr,
		ot_nat, //   TODO!   //
	} type;
	union
	{
		struct {} nil; // does nothing, for clarity.
		double num;
		char *str;
		obj_t *(*nat)(ctx_t*, size_t, obj_t**);
		struct
		{
			size_t count;
			char **args;
			node_t *body;
		} fun;
	} value;
};

typedef struct
{
	char *name;
	obj_t *value;
} var_t;

struct ctx_t
{
	size_t count, obj_count;
	var_t *vars;
	obj_t **pool;
	ctx_t *parent;
	bool in_loop;
};

void print_obj(obj_t *o);
void add_obj(ctx_t *ctx, obj_t *o)
{
	printf("Adding object %zx (", (size_t)o);
	print_obj(o);
	printf(") to %zx\n", (size_t)ctx);
    ctx->pool = realloc(ctx->pool, ++ctx->obj_count * sizeof(obj_t*));
    ctx->pool[ctx->obj_count - 1] = o;
}



// OBJ //

bool obj_is_true(obj_t *o)
{
    if(!o) return false;
    switch (o->type) {
		case ot_nil: return false;
		case ot_num: return o->value.num;
		case ot_str: return o->value.str && strlen(o->value.str);
		default: return false;
	}
}
#define alloc_obj() malloc(sizeof(obj_t))

obj_t *create_nil_obj(ctx_t *ctx)
{
	obj_t *o = alloc_obj();
	o->type = ot_nil;
	add_obj(ctx, o);
	return o;
}

obj_t *create_num_obj(ctx_t *ctx, double value)
{
	obj_t *o = alloc_obj();
	o->type = ot_num;
	o->value.num = value;
	add_obj(ctx, o);
	return o;
}

obj_t *create_str_obj(ctx_t *ctx, const char *value)
{
	obj_t *o = alloc_obj();
	o->type = ot_str;
	o->value.str = copy_string(value);
	add_obj(ctx, o);
	return o;
}

obj_t *create_fun_obj(ctx_t *ctx, size_t count, char **args, node_t *body)
{
	obj_t *o = alloc_obj();
	o->type = ot_fun;
	o->value.fun.count = count;
	o->value.fun.body = body;
	o->value.fun.args = malloc(sizeof(char*) * count);
	for(size_t i = 0; i < count; ++i)
		o->value.fun.args[i] = copy_string(args[i]);
	add_obj(ctx, o);
	return o;
}

obj_t *create_nat_obj(ctx_t *ctx, obj_t *(*nat)(ctx_t*, size_t, obj_t**))
{
	puts("create nat");
	puts("create nat alloc");
	obj_t *o = alloc_obj();
	puts("create nat type");
	o->type = ot_nat;
	puts("create nat type");
	o->value.nat = nat;
	puts("create nat add");
	add_obj(ctx, o);
	puts("create nat ret");
	return o;
}

obj_t *copy_obj(ctx_t *ctx, obj_t *o)
{
	if(!o) return NULL;
	switch (o->type) {
		case ot_nil: return create_nil_obj(ctx);
		case ot_num: return create_num_obj(ctx, o->value.num);

		case ot_str:
			return create_str_obj(ctx, copy_string(o->value.str));
		case ot_nat:
			return create_nat_obj(ctx, o->value.nat);
		case ot_fun:
			return create_fun_obj(ctx,
					o->value.fun.count,
					o->value.fun.args,
					o->value.fun.body);
		default:
            fprintf(stderr, "error: eval -> copy obj not implemented\n");
			return NULL;
	}
}

void free_obj(obj_t *o)
{
	if(!o) return;
	switch (o->type) {
		case ot_nil:
		case ot_nat:
		case ot_num: break;
		case ot_str:
			free(o->value.str);
			break;
		case ot_fun:
			for(size_t i = 0; i < o->value.fun.count; ++i)
				free(o->value.fun.args[i]);
			free(o->value.fun.args);
			break;
		default:
            fprintf(stderr, "error: eval -> free obj %d not implemented\n", o->type);
			return;
	}
	free(o);
}

// value returned is a constant.
char *string_type(char type)
{
    switch(type) {
        case ot_nil: return "nil";
        case ot_fun: return "function";
        case ot_nat: return "native";
        case ot_num: return "number";
        case ot_str: return "string";
        default:
            fprintf(stderr, "error: eval -> string type not implemented\n");
			return NULL;
    }
}

// Beware: bufsz is ignored for ot_nil, ot_fun, ot_nat.
// at most 'bufsz' bytes are written to buf.
void string_obj(char **buf, size_t bufsz, obj_t *o)
{
	if(!o) return;
    switch(o->type) {
        case ot_nil: snprintf(*buf, bufsz, "nil"); break;
        case ot_fun: snprintf(*buf, bufsz, "[ function ]"); break;
        case ot_nat: snprintf(*buf, bufsz, "[ native ]"); break;
        case ot_num: snprintf(*buf, bufsz, "%f", o->value.num); break;
        case ot_str: snprintf(*buf, bufsz, "\"%s\"", o->value.str); break;
        default:
            fprintf(stderr, "error: eval -> string obj %d not implemented\n", o->type);
			break;
    }
}

// convert to
// char *obj_to_string(obj_t *o);
void print_obj(obj_t *o)
{
	if(!o) return;
    char *str = malloc(MICROL_STR_OBJ_LIM);
	string_obj((char**)&str, MICROL_STR_OBJ_LIM, o);
	printf("%s", str);
	free(str);
}



// VAR //


/* Beware that create_var allocates
   the name and copies from the argument.
   (see free_var) */
var_t create_var(char *name, obj_t *value)
{
	var_t v = (var_t){ copy_string(name), value };
	return v;
};

/* Frees the variable name. */
void free_var(var_t *var)
{ free(var->name); };



// CTX //


ctx_t create_context(ctx_t *parent)
{
    return (ctx_t)
    {
        0, // count
        0, // var_count
        malloc(0), // vars
        malloc(0), // pool
        parent, // parent
        false // in_loop
    };
}

void free_context(ctx_t *ctx)
{
	for(size_t i = 0; i < ctx->count; ++i)
	{
		free_var(&ctx->vars[i]);
	}
    for(size_t i = 0; i < ctx->obj_count; ++i)
	{
		if(ctx->pool[i])
		{
			free_obj(ctx->pool[i]);
			ctx->pool[i] = NULL;
		}
	}
	free(ctx->vars);
	free(ctx->pool);
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
	ctx->vars = realloc(ctx->vars, ++ctx->count * sizeof(var_t));
	ctx->vars[ctx->count - 1] = var;
}



#endif//MICROL_CTX_H
