#ifndef MICROL_CTX_H
#define MICROL_CTX_H

typedef struct obj_t obj_t;
struct obj_t
{
	enum
	{
		ot_num, ot_str, ot_nil
	} type;
	union
	{
		struct {} nil; // does nothing, for clarity.
		double num;
		char *str;
	} value;
};

typedef struct
{
	char *name;
	obj_t *value;
} var_t;

typedef struct ctx_t ctx_t;
struct ctx_t
{
	size_t count, obj_count;
	var_t *vars;
	obj_t **pool;
	ctx_t *parent;
	bool in_loop;
};


void add_obj(ctx_t *ctx, obj_t *o)
{
    ctx->pool = realloc(ctx->pool, ++ctx->obj_count);
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
	}
    return false;
}
#define alloc_obj() malloc(sizeof(obj_t))

obj_t *create_nil_obj(ctx_t *ctx)
{
	obj_t *o = alloc_obj();
	o->type = ot_nil;
	return o;
}

obj_t *create_num_obj(ctx_t *ctx, double value)
{
	obj_t *o = alloc_obj();
	o->type = ot_num;
	o->value.num = value;
	return o;
}

obj_t *create_str_obj(ctx_t *ctx, char *value)
{
	obj_t *o = alloc_obj();
	o->type = ot_str;
	size_t len = strlen(value) + 1;
	char *str = malloc(len);
	memcpy(str, value, len);
	o->value.str = str;
	return o;
}

void free_obj(obj_t *o)
{
	switch (o->type) {
		case ot_nil:
		case ot_num: break;

		case ot_str:
			free(o->value.str);
			break;
	}
	free(o);
}

// convert to
// char *obj_to_string(obj_t *o);
void print_obj(obj_t *o)
{
    switch (o->type) {
        case ot_nil: printf("nil"); break;
        case ot_num: printf("%f", o->value.num); break;
        case ot_str: printf("%s", o->value.str); break;
        default:
            fprintf(stderr, "error: eval -> print obj not implemented\n");
    }
}


// VAR //


/* Beware that create_var allocates
   the name and copies from the argument.
   (see free_var) */
var_t create_var(char *name, obj_t *value)
{
	size_t len = strlen(name) + 1; // include null-terminator!
	var_t v = (var_t){ malloc(len), value };
	memcpy(v.name, name, len);
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
		free_var(&ctx->vars[i]);
    for(size_t i = 0; i < ctx->obj_count; ++i)
		free_obj(ctx->pool[i]);
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



#endif//MICROL_CTX_H
