// microl language
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define MICROL_STR_OBJ_LIM 1024

#define bool char
#define true 1
#define false 0

#define dmain_printf(...) printf(__VA_ARGS__)
#define dmain_puts(...) puts(__VA_ARGS__)

#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "intr.h"
#include "stdlib.h"

void help(const char *name)
{
	printf(
		"Usage:\n"
		"  %s run <filename>\n"
		"  %s help\n"
		"  %s about\n",

		name, name, name
	);
}

void about()
{
	printf("microl language\nversion %d.%d\n", VERSION_MAJOR, VERSION_MINOR);
}

void run(const char *filename)
{
	FILE *fptr = fopen(filename, "r");
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
		if(n) free_node(n);
		return;
	}

	dmain_puts("context init");
	ctx_t ctx = create_context(NULL);
	dmain_puts("stdlib init");
	stdlib_context(&ctx);
	dmain_puts("eval");
	eval_node(n, &ctx);
	dmain_puts("output");
	var_t *output = get_var(&ctx, "output");
	if(output)
	{
		if(!output->value)
		{
			fprintf(stderr, "could not display \"output\" due to errors!\n");
		}
		else
		{
			print_obj(output->value);
			putc('\n', stdout);
		}
	}
	else
		fprintf(stderr, "No variable with name \"output\" found!\n");

	free_context(&ctx);
	dmain_puts("freeing...");
	if(n) free_node(n);
}

int cli(int argc, char **argv)
{
	const char *program_name = argv[0];

	if(argc < 2) return help(program_name), 1;

	char *command = argv[1];
	char **args = argv + 2;
	int args_count = argc - 2;

	if(strcmp(command, "run") == 0)
	{
		if(args_count != 1) return help(program_name), 1;
		run(args[0]);
	}
	else if(strcmp(command, "help") == 0)
	{
		help(program_name);
	}
	else if(strcmp(command, "about") == 0)
	{
		if(args_count != 0) return help(program_name), 1;
		about();
	}
	else
	{
		return help(program_name), 1;
	}
	return 0;
}

#include <time.h>
// benckmark
int main(int argc, char **argv)
{
	clock_t begin = clock();
	int o = cli(argc, argv);
	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("[finished in %fs]!\n", time_spent);
	return o;
}
