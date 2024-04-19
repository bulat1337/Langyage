#include <stdio.h>
#include <stdarg.h>

#include "recursive_parser_secondary.h"

static size_t sce_debt = 0;

#define FIRST_SYM_COND									\
	('a' <= *(cur_str) && *(cur_str) <= 'z') ||			\
	('A' <= *(cur_str) && *(cur_str) <= 'Z') ||			\
	(*(cur_str) == '_') ||								\
	(*(cur_str) == '$')

#define SYM_COND																\
	('a' <= *(cur_str + sym_counter) && *(cur_str + sym_counter) <= 'z') ||		\
	('A' <= *(cur_str + sym_counter) && *(cur_str + sym_counter) <= 'Z') ||		\
	('0' <= *(cur_str + sym_counter) && *(cur_str + sym_counter) <= '9') ||		\
	(*(cur_str + sym_counter) == '_') ||										\
	(*(cur_str + sym_counter) == '$')

void rec_write_log(const char *file_name, const char *fmt, ...)
{
    static FILE *log_file = fopen(file_name, "w");

    if (log_file == NULL)
	{
        perror("Error opening log_file");
        return;
    }

    va_list args = NULL;

    va_start(args, fmt);

    vfprintf(log_file, fmt, args);

    va_end(args);
}

B_tree_node *get_scope()
{
	if(CUR_TYPE == OCBR)
	{
		size_t scope_sce_debt = sce_debt;
		sce_debt = 0;

		id++;
		PARSE_LOG("There is scope.\n");
		PARSE_LOG("Getting first command.\n");



		PARSE_LOG("Getting first scope in scope.\n");

		B_tree_node *root = get_scope();
		CHECK_RET(root);

		B_tree_node *cur_node = root;

		while(CUR_TYPE != CCBR)
		{
			B_tree_node *scope_end = get_scope_end(cur_node);

			PARSE_LOG("Getting scope in scope.\n");
			scope_end->right = get_scope();

			cur_node = cur_node->right;
		}
		sce_debt++;

		PARSE_LOG("CCBR for scope ok.\n");

		id++;

		if(root->type == SCS)
		{
			root = CR_SCS(NULL, root);
		}
		else
		{
			root->type = SCS;
		}

		if(scope_sce_debt)
		{
			for(size_t debt_id = 0; debt_id < scope_sce_debt; debt_id++)
			{
				root = CR_SCE(NULL, root);
				printf("scope paying debt.\n");
			}

			printf("sce_debt: %lu, scope_sce_debt: %lu\n", sce_debt, scope_sce_debt);
		}

		return root;
	}
	else
	{
		PARSE_LOG("Getting command.\n");

		B_tree_node *root = get_cmd();
		CHECK_RET(root);

		return root;
	}
}

B_tree_node *get_cmd()
{
	B_tree_node *cmd = NULL;

	if(CUR_TYPE == KWD)
	{
		PARSE_LOG("It's KWD there, getting condition action.\n");

		cmd = get_cond();
		CHECK_RET(cmd);

		printf("cmd debt: %lu\n", sce_debt);
		if(sce_debt)
		{
			B_tree_node *cmd_parent = CR_SCE(NULL, NULL);
			B_tree_node *cur_node = cmd_parent;

			for(size_t debt_id = 0; debt_id < sce_debt - 1; debt_id++)
			{
				cur_node->right = CR_SCE(NULL, NULL);
				cur_node = cur_node->right;
			}

			cur_node->left = cmd;

			sce_debt = 0;
			return cmd_parent;
		}
		else
		{
			return CR_SMC(cmd, NULL);
		}
	}
	else
	{
		PARSE_LOG("Getting assignment.\n");

		cmd = get_ass();
		CHECK_RET(cmd);

		if(CUR_TYPE == SMC)
		{
			PARSE_LOG("SMC ok.\n");

			id++;

			printf("scope debt: %lu\n", sce_debt);
			if(sce_debt)
			{
				B_tree_node *cmd_parent = CR_SCE(NULL, NULL);
				B_tree_node *cur_node = cmd_parent;

				for(size_t debt_id = 0; debt_id < sce_debt - 1; debt_id++)
				{
					cur_node->right = CR_SCE(NULL, NULL);
					cur_node = cur_node->right;
				}

				cur_node->left = cmd;

				sce_debt = 0;
				return cmd_parent;
			}
			else
			{
				return CR_SMC(cmd, NULL);
			}
		}
		else
		{
			SYNTAX_ERROR;
		}
	}
}

B_tree_node *get_cond()
{
	PARSE_LOG("Getting id.\n");
	B_tree_node *kwd = get_id();
	CHECK_RET(kwd);

	SYNTAX_CHECK(	IS_KWD(kwd->value.var_value, "while") ||
					IS_KWD(kwd->value.var_value, "if")	);

	if(CUR_TYPE == OBR)
	{
		PARSE_LOG("OBR ok.\n");
		id++;

		PARSE_LOG("Getting brace expression.\n");
		B_tree_node *br_expr = get_add();
		CHECK_RET(br_expr);

		if(CUR_TYPE == CBR)
		{
			PARSE_LOG("CBR ok\n");
			id++;

			if(CUR_TYPE == OCBR)
			{
				PARSE_LOG("OCBR detected.\n");
				id++;

				PARSE_LOG("Getting first command in cond scope.\n");

				B_tree_node *root = get_cmd();

				B_tree_node *cur_node = root;

				while(CUR_TYPE != CCBR)
				{
					PARSE_LOG("Getting command in cond scope.\n");
					cur_node->right = get_cmd();
					CHECK_RET(cur_node->right);
					cur_node = cur_node->right;
				}

				PARSE_LOG("CCBR detected.\n");
				id++;

				return CR_KWD(kwd->value.var_value, br_expr, root);
			}
			else
			{
				PARSE_LOG("Getting command for cond scope.\n");
				B_tree_node *cmd = get_cmd();
				CHECK_RET(cmd);

				return CR_KWD(kwd->value.var_value, br_expr, cmd);
			}
		}
		else
		{
			SYNTAX_ERROR;
		}
	}
	else
	{
		SYNTAX_ERROR;
	}
}

B_tree_node *get_ass()
{
	B_tree_node *var = get_id();
	CHECK_RET(var);

	if(CUR_TYPE == OP && CUR_OP == ASS)
	{
		id++;
	}
	else
	{
		SYNTAX_ERROR;
	}

	B_tree_node *expr = get_add();
	CHECK_RET(expr);

	return CR_ASS(var, expr);
}

B_tree_node *get_num()
{
	btr_elem_t val = CUR_NUM;

	PARSE_LOG("It's num: %lf\n", val);

	id++;

	return CR_NUM(val, NULL, NULL);
}

B_tree_node *get_add()
{
	B_tree_node *val = get_mul();
	CHECK_RET(val);

	while(	CUR_TYPE == OP &&
			(	CUR_OP == ADD ||
				CUR_OP == SUB	)	)
	{
		PARSE_LOG("It's ADD or SUB.\n");
		Ops op = CUR_OP;

		id++;

		B_tree_node *val_2 = get_mul();
		CHECK_RET(val_2);

		val = CR_OP(op, val, val_2);
	}

	return val;
}

B_tree_node *get_mul()
{
	B_tree_node *val = get_pow();
	CHECK_RET(val);

	while(	CUR_TYPE == OP &&
			(	CUR_OP == MUL ||
				CUR_OP == DIV	)	)
	{
		PARSE_LOG("It's MUL or DIV.\n");
		Ops op = CUR_OP;

		id++;

		B_tree_node *val_2 = get_pow();
		CHECK_RET(val_2);

		val = CR_OP(op, val, val_2);
	}

	return val;
}

B_tree_node *get_par()
{
	if(CUR_TYPE == OBR)
	{
		id++;
		B_tree_node *val = get_add();
		CHECK_RET(val);

		SYNTAX_CHECK(CUR_TYPE == CBR);
		id++;

		return val;
	}
	else if(CUR_TYPE == NUM)
	{
		B_tree_node *val = get_num();
		CHECK_RET(val);

		return val;
	}
	else
	{
		B_tree_node *val = get_id();
		CHECK_RET(val);

		return val;
	}
}

B_tree_node *get_id()
{
	PARSE_LOG("%s log:\n", __func__);

	char *var_name = tokens->data[id].value.var_value;

	PARSE_LOG("name: %s\n", var_name);



	if(CUR_TYPE == KWD)
	{
		PARSE_LOG("It's KWD.\n");
		id++;

		if(	!(IS_KWD(var_name, "while") || IS_KWD(var_name, "if")))
		{
			if(CUR_TYPE == OBR)
			{
				PARSE_LOG("OBR ok\n");

				id++;
				B_tree_node *child = get_add();
				CHECK_RET(child);

				if(CUR_TYPE == CBR)
				{
					PARSE_LOG("CBR ok\n");

					id++;
					return CR_KWD(var_name, NULL, child);
				}
				else
				{
					SYNTAX_ERROR;
				}
			}
			else
			{
				SYNTAX_ERROR;
			}
		}
		else
		{
			PARSE_LOG("It's 'while' of 'if'.\n");

			return CR_KWD(var_name, NULL, NULL);
		}
	}
	else
	{
		id++;
		return CR_VAR(var_name, NULL, NULL);
	}


}

B_tree_node *get_pow()
{
	B_tree_node *val = get_par();
	CHECK_RET(val);

	while(	(CUR_TYPE == OP) && (CUR_OP == POW)	)
	{
		id++;

		B_tree_node *val_2 = get_par();
		CHECK_RET(val_2);

		val = CR_OP(POW, val, val_2);
	}

	return val;
}

B_tree_node *get_scope_end(B_tree_node *root)
{
	if(root == NULL)
	{
		return root;
	}
	while(root->right != NULL)
	{
		root = root->right;
	}

	return root;
}


#undef SYM_COND
#undef FIRST_SYM_COND
