#ifndef MICROL_STDLIB_H
#define MICROL_STDLIB_H
#include "ctx.h"

void stdlib_error_arg_count(const char *from, size_t expected, size_t got)
{
    fprintf(stderr, "error: %s -> expected %zu argument%s, but got %zu\n",
        from,
        expected,
        expected == 1 ? "" : "s", got);
}

// no more than 256 types i suppose.
void stdlib_error_arg_type(const char *from, char expected, char got)
{
    fprintf(stderr, "error: %s -> expected %s, but got %s\n",
        from, string_type(expected), string_type(got));
}

obj_t *stdlib_decl_tostring(ctx_t *ctx, size_t count, obj_t *args[])
{
    if(count != 1)
    {
        stdlib_error_arg_count("tostring()", 1, count);
        return NULL;
    }
    char *str = malloc(MICROL_STR_OBJ_LIM);
    //str[0] = 0;
    string_obj((char**)&str, MICROL_STR_OBJ_LIM, args[0]);
	//printf("oh look here's my string: \"%s\"\n", str);
    free(str);
    obj_t *o = create_str_obj(ctx, str);
    return o;
}

obj_t *stdlib_decl_write(ctx_t *ctx, size_t count, obj_t *args[])
{
    if(count != 1)
    {
        stdlib_error_arg_count("__write()", 1, count);
        return NULL;
    }
    if(args[0]->type != ot_str)
    {
        stdlib_error_arg_type("__write()", ot_str, args[0]->type);
        return NULL;
    }

    printf("%s\n", ((obj_str_t*)args[0])->str);
    return NULL;
}

void stdlib_context(ctx_t *ctx)
{
    if(!ctx) return;
    add_var(ctx, create_var("__write", create_nat_obj(ctx, stdlib_decl_write)));
    add_var(ctx, create_var("tostring", create_nat_obj(ctx, stdlib_decl_tostring)));
}

#endif//MICROL_STDLIB_H
