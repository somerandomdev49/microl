// microl language
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define VERSION_MAJOR 0
#define VERSION_MINOR 1

#define dmain_printf(...) //printf(__VA_ARGS__)
#define dmain_puts(...) //puts(__VA_ARGS__)

#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "intr.h"

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

	dmain_puts("eval...");
	ctx_t ctx = create_context();
	if(!parser.fail)
	{
		eval_node(n, &ctx);
		var_t *output = get_var(&ctx, "output");
		if(output)
			printf("%f\n", output->value);
		else
			fprintf(stderr, "No variable with name \"output\" found!\n");
	}

	free_context(&ctx);
	dmain_puts("freeing...");
	if(n) free_node(n);
}

int main(int argc, char **argv)
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
}
