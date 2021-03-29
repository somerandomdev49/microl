#ifndef MICROL_TOKEN_H
#define MICROL_TOKEN_H

#define dtok_printf(...) //printf(__VA_ARGS__)

typedef enum
{
	tt_var, tt_num, tt_str, tt_eof,
	tt_add, tt_sub, tt_mul, tt_div, // '+', '-', '*', '/'
	tt_eql, tt_neq, tt_ieq, tt_smc, // '=', '!=', '==', ';'
	tt_grt, tt_lst, tt_gte, tt_lte, // '>', '<', '>=', '<='
	tt_open_paren,  tt_close_paren, // '(', ')'
} token_type_t;

typedef struct
{
	char *value;
	token_type_t type;
} token_t;

token_t token_eof()
{
	return (token_t){ "EOF", tt_eof };
}

struct token_list_cell_t
{
	token_t token;
	struct token_list_cell_t *next;
};

typedef struct
{
	struct token_list_cell_t *head, *tail;
} token_list_t;

token_list_t create_token_list()
{
	return (token_list_t) { NULL, NULL };
}

void add_token(token_list_t *list, token_t token)
{
	struct token_list_cell_t *cell = malloc(sizeof(struct token_list_cell_t));
	cell->token = token;
	cell->next = NULL;
	if(list->tail == NULL)
	{
		list->head = list->tail = cell;
	}
	else
	{
		list->tail->next = cell;
		list->tail = cell;
	}
}

token_t *get_token(token_list_t *list, int index)
{
	struct token_list_cell_t *tmp = list->head;
	for(int i = 0; i < index; ++i)
		tmp = tmp->next;
	return &tmp->token;
}

void set_token(token_list_t *list, int index, token_t value)
{
	struct token_list_cell_t *tmp = list->head;
	for(int i = 0; i < index; ++i)
		tmp = tmp->next;
	struct token_list_cell_t *tmp_next = tmp->next;
	free(tmp->token.value);
	memcpy(&tmp->token, &value, sizeof(token_t));
	tmp->next = tmp_next;
}

void del_token(token_list_t *list)
{
	struct token_list_cell_t *tmp = list->head;
	list->head = list->head->next;
	dtok_printf("Token %d, \"%s\"\n", tmp->token.type, tmp->token.value);
	if(tmp->token.value != NULL) free(tmp->token.value);
	free(tmp);
}

void del_all_tokens(token_list_t *list)
{
	while(list->head)
		del_token(list);
	list->tail = NULL;
}

void test_token_list()
{
	token_list_t toks = create_token_list();
	puts("init");
	add_token(&toks, (token_t){ "test1", 3 });
	add_token(&toks, (token_t){ "test2", 2 });
	add_token(&toks, (token_t){ "test3", 1 });
	puts("test");
	printf("%s\n", get_token(&toks, 0)->value);
	printf("%s\n", get_token(&toks, 1)->value);
	printf("%s\n", get_token(&toks, 2)->value);

	set_token(&toks, 1, (token_t) { "new", 4 });
	printf("%s\n", get_token(&toks, 1)->value);
	puts("free");
	del_token(&toks);
	del_token(&toks);
	del_token(&toks);
	//del_all_tokens(&toks);
	printf("%s\n", toks.head == NULL ? "empty" : "non-empty");
}


#endif//MICROL_TOKEN_H
