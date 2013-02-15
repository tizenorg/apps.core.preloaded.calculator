/*
*
* Copyright 2012  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the License);
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.tizenopensource.org/license
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an AS IS BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#include <math.h>		/* For math functions, cos(), sin(), etc. */
#include <dlog.h>
#include <string.h>
#include <stdbool.h>

#include "calc-main.h"
#include "calc-string.h"
#include "calc-expression.h"
#include "calculator_parser.h"

#define  DECNUMDIGITS 34
#include "decnumber/decNumber.h"

#define CALCULATOR_MAX_FUNC_NUMBER 				16		/**<maximum num of function*/

extern char decimal_ch;
static char g_stack[MAX_EXPRESSION_LENGTH];

static calculator_parentheses_data_t g_parentheses_data[MAX_PARENTHESES_NUM];

static int oper_num = 0;

static const function_t g_functions[CALCULATOR_MAX_FUNC_NUMBER] = {
	{"x^y", "^", 1, '^', FUNCTION_POSTFIX, OPERATOR_TYPE_BINARY,
	 CALCULATOR_CALCULATE_PRIORITY_HIGH, FALSE},
	{"ln", "ln", 2, 'L', FUNCTION_PREFIX, OPERATOR_TYPE_UNARY,
	 CALCULATOR_CALCULATE_PRIORITY_HIGHER, TRUE},
	{"sqrt", "sqrt", 4, 'q', FUNCTION_PREFIX, OPERATOR_TYPE_UNARY,
	 CALCULATOR_CALCULATE_PRIORITY_HIGHER, TRUE},
	{"10^x", "10^", 3, 'T', FUNCTION_PREFIX, OPERATOR_TYPE_UNARY,
	 CALCULATOR_CALCULATE_PRIORITY_HIGHER, FALSE},
	{"x!", "!", 1, '!', FUNCTION_POSTFIX, OPERATOR_TYPE_UNARY,
	 CALCULATOR_CALCULATE_PRIORITY_HIGHEST, FALSE},
	{"1/x", "/", 1, 'x', FUNCTION_POSTFIX, OPERATOR_TYPE_UNARY,
	 CALCULATOR_CALCULATE_PRIORITY_HIGHEST, FALSE},
	{"sin", "sin", 3, 's', FUNCTION_PREFIX, OPERATOR_TYPE_UNARY,
	 CALCULATOR_CALCULATE_PRIORITY_HIGHER, TRUE},
	{"cos", "cos", 3, 'c', FUNCTION_PREFIX, OPERATOR_TYPE_UNARY,
	 CALCULATOR_CALCULATE_PRIORITY_HIGHER, TRUE},
	{"tan", "tan", 3, 't', FUNCTION_PREFIX, OPERATOR_TYPE_UNARY,
	 CALCULATOR_CALCULATE_PRIORITY_HIGHER, TRUE},
	{"Pi", "p", 1, 'p', FUNCTION_CONSTANT, OPERATOR_TYPE_INVALID,
	 CALCULATOR_CALCULATE_PRIORITY_INVALID, FALSE},
	{"e", "e", 1, 'e', FUNCTION_CONSTANT, OPERATOR_TYPE_INVALID,
	 CALCULATOR_CALCULATE_PRIORITY_INVALID, FALSE},
	{"log", "log", 3, 'l', FUNCTION_PREFIX, OPERATOR_TYPE_UNARY,
	 CALCULATOR_CALCULATE_PRIORITY_HIGHER, TRUE},
	{"abs", "abs", 3, 'b', FUNCTION_PREFIX, OPERATOR_TYPE_UNARY,
	 CALCULATOR_CALCULATE_PRIORITY_HIGHER, TRUE},
	{"2^x", "2^", 2, '2', FUNCTION_PREFIX, OPERATOR_TYPE_UNARY,
	 CALCULATOR_CALCULATE_PRIORITY_HIGHER, FALSE},
	{"xCy", "C", 1, 'C', FUNCTION_POSTFIX, OPERATOR_TYPE_BINARY,
	 CALCULATOR_CALCULATE_PRIORITY_HIGH, FALSE},
	{"xPy", "P", 1, 'P', FUNCTION_POSTFIX, OPERATOR_TYPE_BINARY,
	 CALCULATOR_CALCULATE_PRIORITY_HIGH, FALSE},
};

#define CALCULATOR_GET_NODE_DATA(t)	((calculator_node_data_t*)(((GNode*)(t))->data))/**<to get data from GNode structure*/

/**
* @describe
*
*
* @param    out_tree
* @param    to_insert
* @param    const_node
* @return    gboolean
* @exception
*/
static gboolean
__calculator_calculate_insert_node(GNode ** out_tree, GNode * to_insert,
				   GNode * const_node)
{
	GNode *upper_node = NULL;
	GNode *node = NULL;
	calculator_node_data_t *node_data = NULL;

	if ((!out_tree) || (!*out_tree) || (!to_insert)) {
		return FALSE;
	}

	upper_node = *out_tree;
	node = upper_node;

	while (CALCULATOR_GET_NODE_DATA(upper_node)->node_calcu_priority >
	       CALCULATOR_GET_NODE_DATA(to_insert)->node_calcu_priority) {
		if (G_NODE_IS_ROOT(upper_node)) {
			g_node_insert(to_insert, -1, upper_node);
			CALCULATOR_GET_NODE_DATA(to_insert)->children_num++;

			if (const_node) {
				g_node_insert(*out_tree, -1, const_node);	//value of last node's priority>= upper_node's > to_insert's
				CALCULATOR_GET_NODE_DATA(*out_tree)->
				    children_num++;
			}
			break;
		}
		node = upper_node;
		upper_node = upper_node->parent;
	}

	if (CALCULATOR_GET_NODE_DATA(upper_node)->node_calcu_priority ==
	    CALCULATOR_GET_NODE_DATA(to_insert)->node_calcu_priority) {
		if (CALCULATOR_GET_NODE_DATA(to_insert)->operator_type ==
		    OPERATOR_TYPE_BINARY) {
			GNode *parent_node = NULL;

			if (!G_NODE_IS_ROOT(upper_node)) {
				parent_node = upper_node->parent;
				g_node_unlink(upper_node);
				g_node_insert(parent_node, -1, to_insert);
			}

			g_node_insert(to_insert, -1, upper_node);
			CALCULATOR_GET_NODE_DATA(to_insert)->children_num++;

			if (const_node) {
				g_node_insert(*out_tree, -1, const_node);
				CALCULATOR_GET_NODE_DATA(*out_tree)->
				    children_num++;
			}
		} else if (CALCULATOR_GET_NODE_DATA(to_insert)->operator_type ==
			   OPERATOR_TYPE_UNARY) {
			GNode *tmp_node = NULL;

			if (const_node) {
				node_data =
				    g_malloc0(sizeof(calculator_node_data_t));
				node_data->cur_operator = 'x';
				node_data->negative_flag = 1;
				node_data->operator_type = OPERATOR_TYPE_BINARY;
				node_data->node_calcu_priority =
				    CALCULATOR_CALCULATE_PRIORITY_MIDDLE;
				tmp_node = g_node_new(node_data);

				if (CALCULATOR_GET_NODE_DATA(to_insert)->
				    cur_operator != '!') {
					__calculator_calculate_insert_node
					    (out_tree, tmp_node, const_node);
				} else	//(CALCULATOR_GET_NODE_DATA(to_insert)->cur_operator == '!')
				{
					g_node_insert(to_insert, -1,
						      const_node);
					CALCULATOR_GET_NODE_DATA(to_insert)->
					    children_num++;
				}

				g_node_insert(*out_tree, -1, to_insert);
				CALCULATOR_GET_NODE_DATA(*out_tree)->
				    children_num++;
			} else {
				g_node_insert(upper_node, -1, to_insert);	//combine from right
				CALCULATOR_GET_NODE_DATA(upper_node)->
				    children_num++;
			}
		}
	} else if (CALCULATOR_GET_NODE_DATA(upper_node)->node_calcu_priority <
		   CALCULATOR_GET_NODE_DATA(to_insert)->node_calcu_priority) {
		if (CALCULATOR_GET_NODE_DATA(to_insert)->operator_type ==
		    OPERATOR_TYPE_BINARY) {
			if (node != upper_node) {
				g_node_unlink(node);
				CALCULATOR_GET_NODE_DATA(upper_node)->
				    children_num--;

				g_node_insert(to_insert, -1, node);
				CALCULATOR_GET_NODE_DATA(to_insert)->
				    children_num++;
			}

			if (const_node) {
				if (CALCULATOR_GET_NODE_DATA(*out_tree)->
				    node_calcu_priority >
				    CALCULATOR_GET_NODE_DATA(to_insert)->
				    node_calcu_priority) {
					g_node_insert(*out_tree, -1,
						      const_node);
					CALCULATOR_GET_NODE_DATA(*out_tree)->
					    children_num++;
				} else {
					g_node_insert(to_insert, -1,
						      const_node);
					CALCULATOR_GET_NODE_DATA(to_insert)->
					    children_num++;
				}
			}

			g_node_insert(upper_node, -1, to_insert);
			CALCULATOR_GET_NODE_DATA(upper_node)->children_num++;
		}

		else if (CALCULATOR_GET_NODE_DATA(to_insert)->operator_type == OPERATOR_TYPE_UNARY)	//upper_node must be equal to node equalized to *out_tree, for no functions' prority higher than unary operator
		{
			if (const_node) {

				if (CALCULATOR_GET_NODE_DATA(to_insert)->
				    cur_operator != '!') {
					GNode *tmp_node = NULL;

					node_data =
					    g_malloc0(sizeof
						      (calculator_node_data_t));
					node_data->cur_operator = 'x';
					node_data->negative_flag = 1;
					node_data->operator_type =
					    OPERATOR_TYPE_BINARY;
					node_data->node_calcu_priority =
					    CALCULATOR_CALCULATE_PRIORITY_MIDDLE;
					tmp_node = g_node_new(node_data);

					__calculator_calculate_insert_node
					    (out_tree, tmp_node, const_node);
				} else	//(CALCULATOR_GET_NODE_DATA(to_insert)->cur_operator == '!')
				{
					g_node_insert(to_insert, -1,
						      const_node);
					CALCULATOR_GET_NODE_DATA(to_insert)->
					    children_num++;
				}
			}

			g_node_insert(*out_tree, -1, to_insert);
			CALCULATOR_GET_NODE_DATA(*out_tree)->children_num++;
		}

	}

	*out_tree = to_insert;
	return TRUE;
}

/**
* @describe
*
*
* @param    node
* @param    data
* @return    gboolean
* @exception
*/
static gboolean
__calculator_tree_destroy_traverse_func(GNode * node, gpointer data)
{
	if (CALCULATOR_GET_NODE_DATA(node)) {
		g_free(CALCULATOR_GET_NODE_DATA(node));
	}
	g_node_destroy(node);
	return FALSE;
}

/**
* @describe
*
*
* @param    tree
* @return    gboolean
* @exception
*/
static gboolean __calculator_tree_destroy_all(GNode ** tree)
{
	guint depth = 0;
	GNode *root = NULL;

	if (*tree) {
		root = g_node_get_root(*tree);
		depth = g_node_max_height(root);
		g_node_traverse(root, G_POST_ORDER, G_TRAVERSE_ALL, depth,
				__calculator_tree_destroy_traverse_func, NULL);

		*tree = NULL;
	}
	return TRUE;
}

/**
* @describe
*
*
* @param    result
* @param    error_msg
* @return    gboolean
* @exception
*/
static gboolean __calculator_check_overflow(double result, char *error_msg)
{
	if ((result > CALCULATOR_MAX_RESULT_SUM2)
	    || (result < CALCULATOR_MIN_RESULT_SUM2)) {
		strcat(error_msg, CALC_MSG_OUT_OF_RANGE);
		return FALSE;
	}
	return TRUE;
}

/**
* @describe
*   add parenthesis between function and paremeter.
*
* @param    exp
* @param    func
* @return    void
* @exception
*/
static void __calculator_expression_add_parenthesis(char *exp, const char *func)
{
	char *p = exp;
	char *digit = "0123456789.";

	while ((p = strstr(p, func)) != NULL) {
		p = p + strlen(func);
		if (*p != '(') {
			int l = strspn(p, digit);
			string_insert(exp, p - exp, "(");
			string_insert(exp, p - exp + l + 1, ")");
			p += l + 2;
		}
	}
}

/**
* @describe
*   If sin/cos/tan function omitted parenthesis, add it
*
* @param    exp
* @return    void
* @exception
*/
static void __calculator_expression_tri_func_parenthesis(char *exp)
{
	__calculator_expression_add_parenthesis(exp, "sin");
	__calculator_expression_add_parenthesis(exp, "cos");
	__calculator_expression_add_parenthesis(exp, "tan");
}

/**
* @describe
*   record every matching parentheses position
*
* @param    string
* @param    end_pos
* @param    error_msg
* @return    bool
* @exception
*/
static bool
__calculator_calculate_formula_scan(char *string, int *end_idx, char *error_msg)
{
	CALC_FUN_BEG();
	gint i = 0;
	gchar *p = NULL;

	p = string;
	memset(g_parentheses_data, 0x0, sizeof(g_parentheses_data));

	__calculator_expression_tri_func_parenthesis(string);

	while (*p != '\0') {
		if (*p == '(') {
			while (g_parentheses_data[i].start_pos) {
				i++;
			}
			g_parentheses_data[i].start_pos = p;
		}
		if (*p == ')') {
			while (g_parentheses_data[i].end_pos) {
				i--;
			}
			if (g_parentheses_data[i].start_pos) {
				g_parentheses_data[i].end_pos = p;
			}
		}
		p++;
	}
	*end_idx = p - 1 - string;
	CALC_FUN_END();
	return TRUE;
}

/**
* @describe
*
*
* @param    n
* @param    err_msg
* @return    double
* @exception
*/
static double __calculator_calculate_factorial(double n, char *err_msg)
{
	if (n < 0) {
		strcat(err_msg, CALC_MSG_INVALID_FAC);
		return -1;
	} else if (n >= 30) {
		strcat(err_msg, CALC_MSG_OUT_OF_RANGE);
		return -1;
	}

	double i = 0;
	double result = 1;
	for (i = 2; i <= n; ++i) {
		result *= i;
	}
	return result;
}

/**
* @describe
*   Judge the op is a operator, if yes, give fucntion type
*
* @param    op
* @param    function
* @return    gboolean
* @exception
*/
static gboolean __calculator_util_get_operator(char *op, function_t * function)
{
	gint i = 0;
	gboolean ret = FALSE;
	if ((*op == decimal_ch) || (*op == '(') || (*op == ')')) {
		ret = FALSE;
	} else if (isdigit(*op)) {
		ret = FALSE;
	} else if ((*op == '+') || (*op == '-') || (*op == 'x') || (*op == '/')) {
		function->category = FUNCTION_POSTFIX;
		function->function_char = *op;
		function->func_name = NULL;
		function->func_string = NULL;
		function->has_parentheses = FALSE;
		function->op_type = OPERATOR_TYPE_BINARY;
		if ((*op == '+') || (*op == '-')) {
			function->priority = CALCULATOR_CALCULATE_PRIORITY_LOW;
		} else {
			function->priority = CALCULATOR_CALCULATE_PRIORITY_MIDDLE;
		}
		function->str_len = 1;
		ret = TRUE;
	} else {
		for (i = 0; i < CALCULATOR_MAX_FUNC_NUMBER; i++) {
			if (!strncmp
			    (op, g_functions[i].func_string,
			     strlen(g_functions[i].func_string))) {
				memcpy(function, &g_functions[i],
				       sizeof(function_t));
				ret = TRUE;
				break;
			}
		}
	}
	return ret;
}

/* search function, if have, return TRUE and get the length of current function;else return FALSE */
gboolean __calculator_search_function(char *op, int* len)
{
	gint i = 0;
	gboolean ret = FALSE;
	for (i = 0; i < CALCULATOR_MAX_FUNC_NUMBER; i++) {
		if (!strncmp(op, g_functions[i].func_string, strlen(g_functions[i].func_string))) {
			ret = TRUE;
			if(len){
				*len = strlen(g_functions[i].func_string);
			}
			break;
		}
	}
	return ret;
}

/**
* @describe
*
*
* @param    pos
* @return    calculator_parentheses_data_t*
* @exception
*/
static calculator_parentheses_data_t
    *__calculator_calculate_get_parentheses_info(gchar * pos)
{
	calculator_parentheses_data_t *found = NULL;
	gint i = 0;

	while ((g_parentheses_data[i].start_pos) ||
	       (g_parentheses_data[i].end_pos)) {
		if (g_parentheses_data[i].start_pos == pos) {
			found = &g_parentheses_data[i];
			break;
		} else if (g_parentheses_data[i].end_pos == pos) {
			found = &g_parentheses_data[i];
			break;
		}
		i++;
	}

	return found;
}

/**
* @describe
*
*
* @param    tmp_result
* @param    out_tree
* @param    start_pos
* @param    end_pos
* @param    error_msg
* @return    gboolean
* @exception
*/
static gboolean
__calculator_calculate_formula_parse(double *tmp_result, GNode ** out_tree,
				     char *start_pos, int end_idx,
				     char *error_msg)
{
	CALC_FUN_BEG();
	char *end_pos = start_pos + end_idx;
	gchar *p = NULL;
	gchar *q = NULL;
	gchar tmp[MAX_RESULT_LENGTH];
	gint i = 0;
	calculator_state_t calculator_state = CALCULATOR_OPERAND_INPUT;
	calculator_node_data_t *node_data = NULL;
	GNode *tree = NULL;
	GNode *new_node = NULL;
	GNode *last_node = NULL;	//to proc parentheses
	calculator_parentheses_data_t *p_data = NULL;
	gint negative_sign = 1;
	function_t function = { 0 };

	memset(tmp, 0x00, sizeof(tmp));

	if (start_pos > end_pos) {
		strcat(error_msg, CALC_MSG_WRONG_FORMAT);
		return FALSE;
	}

	/* Scan from start to end of string */
	for (p = start_pos; p <= end_pos;) {
		q = p;		/* Point to start of string */
		if (q != NULL && __calculator_util_get_operator(q, &function))	//('.'::'('::')') and digits are four exceptions
		{
			oper_num++;
			if (oper_num > MAX_OPERATOR_NUM) {
				strcat(error_msg, CALC_MSG_MAX_OP);
				return false;
			}

			p = q + function.str_len;	/* Shift */
			if ((NULL == p) && (function.category != FUNCTION_CONSTANT)) {
				return false;
			}
			if ((NULL != p) && (*p == decimal_ch)) {
				printf("p:%s\n", p);
				strcat(error_msg, CALC_MSG_WRONG_FORMAT);
				return false;
			}
			calculator_state = CALCULATOR_OPERATOR_INPUT;
			if (i != 0)	//save the before operator,create a node.such as 345sin(),save 345
			{
				i = 0;

				node_data = g_malloc0(sizeof(calculator_node_data_t));
				strcpy(node_data->tmp_result, tmp);
				memset(tmp, 0x00, sizeof(tmp));
				node_data->negative_flag = 1;
				node_data->negative_flag *= negative_sign;
				new_node = g_node_new(node_data);
				negative_sign = 1;
			} else {
				if (!new_node)	//no leaf node before, first factor's "+/-" sign allowed.
				{
					if (((function.op_type ==OPERATOR_TYPE_BINARY)&& (function.function_char != '-'))
					    ||
					    ((function.op_type ==OPERATOR_TYPE_UNARY) && (function.category == FUNCTION_POSTFIX))) {
						strcat(error_msg,CALC_MSG_WRONG_FORMAT);
						return FALSE;
					} else if (function.function_char == '-') {
						negative_sign = -1;
						continue;
					}
				}
			}

			if (function.category == FUNCTION_CONSTANT)	//Pi, e
			{
				if (new_node)	//have the operator like 345,now it will 345*Pi
				{
					node_data = g_malloc0(sizeof(calculator_node_data_t));
					node_data->cur_operator = 'x';
					node_data->negative_flag = 1;
					node_data->operator_type = OPERATOR_TYPE_BINARY;
					node_data->node_calcu_priority = CALCULATOR_CALCULATE_PRIORITY_MIDDLE;
					tree = g_node_new(node_data);

					if (last_node == NULL)	//first node
					{
						g_node_insert(tree, -1, new_node);
						CALCULATOR_GET_NODE_DATA(tree)->children_num++;
					} else {
						if (CALCULATOR_GET_NODE_DATA(last_node)->children_num >
						    CALCULATOR_GET_NODE_DATA(last_node)->operator_type) {
							strcat(error_msg, CALC_MSG_WRONG_FORMAT);
							return FALSE;
						}
						__calculator_calculate_insert_node(&last_node, tree, new_node);
					}
					new_node = NULL;
					last_node = tree;

				}	//Pi will like a leaf
				node_data = g_malloc0(sizeof(calculator_node_data_t));
				if (function.function_char == 'p') {
					strcpy(node_data->tmp_result, PI_STR);
				} else if (function.function_char == 'e') {
					strcpy(node_data->tmp_result, EXPONENT_STR);
				}
				node_data->cur_operator = function.function_char;
				node_data->node_calcu_priority = function.priority;
				node_data->negative_flag = 1;
				node_data->negative_flag *= negative_sign;
				new_node = g_node_new(node_data);
				negative_sign = 1;
				continue;
			} else if (function.category == FUNCTION_PREFIX)	//sin()
			{
				if (!last_node)	//first node
				{
					if (new_node) {
						node_data = g_malloc0(sizeof(calculator_node_data_t));
						node_data->cur_operator = 'x';
						node_data->negative_flag = 1;
						node_data->operator_type = OPERATOR_TYPE_BINARY;
						node_data->node_calcu_priority = CALCULATOR_CALCULATE_PRIORITY_MIDDLE;
						tree = g_node_new(node_data);

						g_node_insert(tree, -1, new_node);
						CALCULATOR_GET_NODE_DATA(tree)->children_num++;

						last_node = tree;
						new_node = NULL;
					}
				}

			}

			node_data = g_malloc0(sizeof(calculator_node_data_t));
			node_data->cur_operator = function.function_char;
			node_data->node_calcu_priority = function.priority;
			node_data->operator_type = function.op_type;
			node_data->negative_flag = 1;
			tree = g_node_new(node_data);

			if (last_node == NULL)	//first node
			{
				if (new_node) {
					g_node_insert(tree, -1, new_node);
					CALCULATOR_GET_NODE_DATA(tree)->children_num++;
				}
			} else {
				if (CALCULATOR_GET_NODE_DATA(last_node)->children_num >
				    CALCULATOR_GET_NODE_DATA(last_node)->operator_type) {
					strcat(error_msg, CALC_MSG_WRONG_FORMAT);
					return FALSE;
				}
				__calculator_calculate_insert_node(&last_node, tree, new_node);
			}
			last_node = tree;
			new_node = NULL;
		} else {
			p++;	/* Shift */
		}

		if (NULL == q) {
			strcat(error_msg, CALC_MSG_WRONG_FORMAT);	//add for "6((" ,then "="
			return FALSE;
		}		//added by zhaodanni for prevent
		else if (*q == decimal_ch) {
			if (!isdigit(*p) || (calculator_state == CALCULATOR_OPERAND_FRACTION_INPUT)) {
			} else {
				calculator_state = CALCULATOR_OPERAND_FRACTION_INPUT;
			}
			tmp[i++] = *q;
		} else if (*q == '(') {
			if (__calculator_util_get_operator(p, &function)) {
				if ((function.category == FUNCTION_POSTFIX) && ((*p != '+') && (*p != '-')))	//"(*","(/", not allowed.
				{
					strcat(error_msg, CALC_MSG_WRONG_FORMAT);
					return FALSE;
				}
			}
			if ((new_node)
			    &&
			    (!strcmp(CALCULATOR_GET_NODE_DATA(new_node)->tmp_result, PI_STR)
			     || !strcmp(CALCULATOR_GET_NODE_DATA(new_node)->tmp_result, EXPONENT_STR))) {
				node_data =g_malloc0(sizeof(calculator_node_data_t));
				node_data->cur_operator = 'x';
				node_data->negative_flag = 1;
				node_data->operator_type = OPERATOR_TYPE_BINARY;
				node_data->node_calcu_priority = CALCULATOR_CALCULATE_PRIORITY_MIDDLE;
				tree = g_node_new(node_data);

				if (last_node) {
					__calculator_calculate_insert_node(&last_node, tree, new_node);
				} else {
					g_node_insert(tree, -1, new_node);
					CALCULATOR_GET_NODE_DATA(tree)->children_num++;
				}
				last_node = tree;
				new_node = NULL;
			}

			if (i != 0) {
				i = 0;

				node_data = g_malloc0(sizeof(calculator_node_data_t));
				strcpy(node_data->tmp_result, tmp);
				memset(tmp, 0x00, sizeof(tmp));
				node_data->negative_flag = 1;
				node_data->negative_flag *= negative_sign;
				negative_sign = 1;
				new_node = g_node_new(node_data);

				node_data = g_malloc0(sizeof(calculator_node_data_t));
				node_data->cur_operator = 'x';
				node_data->negative_flag = 1;
				node_data->operator_type = OPERATOR_TYPE_BINARY;
				node_data->node_calcu_priority =CALCULATOR_CALCULATE_PRIORITY_MIDDLE;
				tree = g_node_new(node_data);

				if (last_node == NULL)	//first node
				{
					g_node_insert(tree, -1, new_node);
					CALCULATOR_GET_NODE_DATA(tree)->children_num++;
				} else {
					if (CALCULATOR_GET_NODE_DATA(last_node)->children_num >
					    CALCULATOR_GET_NODE_DATA(last_node)->operator_type) {
						strcat(error_msg, CALC_MSG_WRONG_FORMAT);
						return FALSE;
					}

					__calculator_calculate_insert_node(&last_node, tree, new_node);
				}
				last_node = tree;
				new_node = NULL;
			}

			p_data = __calculator_calculate_get_parentheses_info(q);
			if (!p_data) {
				strcat(error_msg, CALC_MSG_WRONG_FORMAT);
				return FALSE;
			}
			p = p_data->end_pos;
		} else if (*q == ')') {
			if (*p == decimal_ch)	//").", not allowed.
			{
				strcat(error_msg, CALC_MSG_WRONG_FORMAT);
				return FALSE;
			}

			p_data = __calculator_calculate_get_parentheses_info(q);
			if (!p_data) {
				strcat(error_msg, CALC_MSG_WRONG_FORMAT);
				return FALSE;
			}
			if (!__calculator_calculate_formula_parse(NULL, &new_node, (p_data->start_pos + 1),
			     (p_data->end_pos - 1) - (p_data->start_pos + 1), error_msg)) {
				if (new_node != NULL) {
					__calculator_tree_destroy_all(&new_node);
				} else {
					LOGD("current node is null\n");
				}
				return FALSE;
			}
			if (new_node) {
				CALCULATOR_GET_NODE_DATA(new_node)->negative_flag *= negative_sign;
			}
			negative_sign = 1;

			if ((*p == '(') || (isdigit(*p))) {
				node_data = g_malloc0(sizeof(calculator_node_data_t));
				node_data->cur_operator = 'x';
				node_data->negative_flag = 1;
				node_data->operator_type = OPERATOR_TYPE_BINARY;
				node_data->node_calcu_priority = CALCULATOR_CALCULATE_PRIORITY_MIDDLE;
				tree = g_node_new(node_data);

				if (last_node == NULL)	//first node
				{
					if (new_node) {
						g_node_insert(tree, -1,
							      new_node);
						CALCULATOR_GET_NODE_DATA(tree)->
						    children_num++;
					}
				} else {
					if (CALCULATOR_GET_NODE_DATA
					    (last_node)->children_num >
					    CALCULATOR_GET_NODE_DATA
					    (last_node)->operator_type) {
						strcat(error_msg, CALC_MSG_WRONG_FORMAT);
						return FALSE;
					}

					__calculator_calculate_insert_node
					    (&last_node, tree, new_node);
				}
				last_node = tree;
				new_node = NULL;
			}
		} else if (isdigit(*q)) {
			if (new_node) {
				node_data = g_malloc0(sizeof(calculator_node_data_t));
				node_data->cur_operator = 'x';
				node_data->negative_flag = 1;
				node_data->operator_type = OPERATOR_TYPE_BINARY;
				node_data->node_calcu_priority = CALCULATOR_CALCULATE_PRIORITY_MIDDLE;
				tree = g_node_new(node_data);

				if (last_node) {
					__calculator_calculate_insert_node(&last_node, tree, new_node);
				} else {
					g_node_insert(tree, -1, new_node);
					CALCULATOR_GET_NODE_DATA(tree)->children_num++;
				}
				last_node = tree;
				new_node = NULL;
			}
			calculator_state = CALCULATOR_OPERAND_INPUT;
			tmp[i++] = *q;
		}/*for unvalid input ,such as "clipborad"*/
		else if ((*q != '+') && (*q != '-') && (*q != 'x')
			 && (*q != '/')) {
			if (!__calculator_search_function(q, NULL)) {
				printf("q=%s,line=%d\n", q, __LINE__);
				strcat(error_msg, "invalid input");
				return FALSE;
			}
		}
	}

	if (i != 0)		//last digit number
	{
		//if ((new_node) && (CALCULATOR_GET_NODE_DATA(new_node)->tmp_result == PI || CALCULATOR_GET_NODE_DATA(new_node)->tmp_result == EXPONENT))
		if ((new_node)
		    &&
		    (!strcmp
		     (CALCULATOR_GET_NODE_DATA(new_node)->tmp_result, PI_STR)
		     || !strcmp(CALCULATOR_GET_NODE_DATA(new_node)->tmp_result,
				EXPONENT_STR))) {
			node_data = g_malloc0(sizeof(calculator_node_data_t));
			node_data->cur_operator = 'x';
			node_data->negative_flag = 1;
			node_data->operator_type = OPERATOR_TYPE_BINARY;
			node_data->node_calcu_priority = CALCULATOR_CALCULATE_PRIORITY_MIDDLE;
			tree = g_node_new(node_data);

			if (last_node) {
				__calculator_calculate_insert_node(&last_node, tree, new_node);
			} else {
				g_node_insert(tree, -1, new_node);
				CALCULATOR_GET_NODE_DATA(tree)->children_num++;
			}
			last_node = tree;
			new_node = NULL;
		}
		i = 0;

		node_data = g_malloc0(sizeof(calculator_node_data_t));
		strcpy(node_data->tmp_result, tmp);
		memset(tmp, 0x00, sizeof(tmp));
		node_data->negative_flag = 1;
		node_data->negative_flag *= negative_sign;
		negative_sign = 1;
		new_node = g_node_new(node_data);

		if (last_node != NULL) {
			if (CALCULATOR_GET_NODE_DATA(last_node)->children_num >
			    CALCULATOR_GET_NODE_DATA(last_node)->operator_type) {
				strcat(error_msg, CALC_MSG_WRONG_FORMAT);
				return FALSE;
			} else {
				g_node_insert(last_node, -1, new_node);
				CALCULATOR_GET_NODE_DATA(last_node)->
				    children_num++;
			}
			new_node = NULL;
		}
	}

	if (new_node != NULL) {
		CALCULATOR_GET_NODE_DATA(new_node)->negative_flag *=
		    negative_sign;
		negative_sign = 1;

		if (last_node != NULL) {
			if (CALCULATOR_GET_NODE_DATA(last_node)->children_num >
			    CALCULATOR_GET_NODE_DATA(last_node)->operator_type) {
				strcat(error_msg, CALC_MSG_WRONG_FORMAT);
				return FALSE;
			} else {
				g_node_insert(last_node, -1, new_node);
				CALCULATOR_GET_NODE_DATA(last_node)->
				    children_num++;
			}
		} else {
			last_node = new_node;
		}
		new_node = NULL;
	}
	*out_tree = g_node_get_root(last_node);
	CALC_FUN_END();
	return TRUE;
}

/**
* @describe
*
*
* @param        node
* @param        data
* @return       gboolean
* @exception
*/
static gboolean
__calculator_calculate_exec_traverse_func(GNode * node, gpointer data)
{
	gdouble operand0, operand1 = 0.0;
	double tmp_ret = 0.0;
	decNumber doperand0, doperand1, dresult;
	decContext set;
	char dec_result[DECNUMDIGITS + 14];
	char dec_result_tmp[MAX_RESULT_LENGTH] = { 0 };
	char *error_msg = (char *)data;
	char op0buf[MAX_RESULT_LENGTH] = { 0 };
	char op1buf[MAX_RESULT_LENGTH] = { 0 };
	if (CALCULATOR_GET_NODE_DATA(node)->children_num !=
	    CALCULATOR_GET_NODE_DATA(node)->operator_type) {
		strcat(error_msg, CALC_MSG_NUM_AFTER_OP);
		return TRUE;	//break the recursion
	}

	decContextTestEndian(0);
	decContextDefault(&set, DEC_INIT_BASE);
	set.digits = DECNUMDIGITS;

	if (CALCULATOR_GET_NODE_DATA(node->children)->negative_flag == -1) {
		strcat(op0buf, "-");
		CALCULATOR_GET_NODE_DATA(node->children)->negative_flag = 1;
	}
	strcat(op0buf, CALCULATOR_GET_NODE_DATA(node->children)->tmp_result);
	operand0 = atof(op0buf);
	decNumberFromString(&doperand0, op0buf, &set);

	if (CALCULATOR_GET_NODE_DATA(node)->operator_type == OPERATOR_TYPE_BINARY) {
		if (CALCULATOR_GET_NODE_DATA
		    (node->children->next)->negative_flag == -1) {
			strcat(op1buf, "-");
			CALCULATOR_GET_NODE_DATA(node->children->next)->negative_flag = 1;
		}
		strcat(op1buf, CALCULATOR_GET_NODE_DATA(node->children->next)->tmp_result);
		operand1 = atof(op1buf);
		decNumberFromString(&doperand1, op1buf, &set);
	}
	switch (CALCULATOR_GET_NODE_DATA(node)->cur_operator) {
	case '+':
		decNumberAdd(&dresult, &doperand0, &doperand1, &set);
		decNumberToString(&dresult, dec_result);
		strcpy(CALCULATOR_GET_NODE_DATA(node)->tmp_result, dec_result);
		break;
	case '-':
		decNumberSubtract(&dresult, &doperand0, &doperand1, &set);
		decNumberToString(&dresult, dec_result);
		strcpy(CALCULATOR_GET_NODE_DATA(node)->tmp_result, dec_result);
		break;
	case 'x':
		decNumberMultiply(&dresult, &doperand0, &doperand1, &set);
		decNumberToString(&dresult, dec_result);
		strcpy(CALCULATOR_GET_NODE_DATA(node)->tmp_result, dec_result);
		break;
	case '/':
		if (FLOAT_EQUAL(operand1, 0)) {
			strcat(error_msg, CALC_MSG_DIVIDE_BY_ZERO);
			return TRUE;	//break the recursion
		} else {
			decNumberDivide(&dresult, &doperand0, &doperand1, &set);
			decNumberToString(&dresult, dec_result);
			strcpy(CALCULATOR_GET_NODE_DATA(node)->tmp_result, dec_result);
		}
		break;
	case '^':
		if (operand0 < 0) {
			gdouble power = 0.0;
			power = floor(operand1);
			if (!FLOAT_EQUAL(power, operand1))	//operand1 is not an integer
			{
				return TRUE;
			}
		}
		tmp_ret =pow(operand0, operand1);
		if (fabs(tmp_ret - 0) < 0.000001) {
			snprintf(dec_result_tmp, sizeof(dec_result_tmp), "%.99f", tmp_ret);
		} else {
			snprintf(dec_result_tmp, sizeof(dec_result_tmp), "%lf", tmp_ret);
		}
		strcpy(CALCULATOR_GET_NODE_DATA(node)->tmp_result, dec_result_tmp);
		if (!__calculator_check_overflow(atof(CALCULATOR_GET_NODE_DATA(node)->tmp_result), error_msg)) {
			return TRUE;
		}
		break;
	case '!':
		if (operand0 < 0) {
			gdouble power = 0.0;
			power = floor(operand1);
			if (!FLOAT_EQUAL(power, operand1))	//operand1 is not an integer
			{
				strcat(error_msg, CALC_MSG_INVALID_FAC);
				return TRUE;
			}
		}
		if (strlen(error_msg) != 0) {
			return TRUE;
		}
		tmp_ret = __calculator_calculate_factorial(operand0, error_msg);
		snprintf(dec_result, sizeof(dec_result), "%lf", tmp_ret);
		strcpy(CALCULATOR_GET_NODE_DATA(node)->tmp_result, dec_result);
		if (strlen(error_msg) != 0) {
			return TRUE;
		}
		break;

	case 'L':
		if (operand0 < 0) {
			return TRUE;
		} else if (FLOAT_EQUAL(operand0, 0)) {
			return TRUE;
		}
		set.emax = 999999;
		set.emin = -999999;
		decNumberLn(&dresult, &doperand0, &set);
		decNumberToString(&dresult, dec_result);
		strcpy(CALCULATOR_GET_NODE_DATA(node)->tmp_result, dec_result);
		if (!__calculator_check_overflow
		    (atof(CALCULATOR_GET_NODE_DATA(node)->tmp_result),
		     error_msg)) {
			return TRUE;
		}
		break;
	case 'q':
		if (operand0 < 0) {
			strcat(error_msg, CALC_MSG_INVALID_SQUARE);
			return TRUE;
		}
		decNumberSquareRoot(&dresult, &doperand0, &set);
		decNumberToString(&dresult, dec_result);
		strcpy(CALCULATOR_GET_NODE_DATA(node)->tmp_result, dec_result);

		if (!__calculator_check_overflow
		    (atof(CALCULATOR_GET_NODE_DATA(node)->tmp_result),
		     error_msg)) {
			return TRUE;
		}
		break;
	case 's':
		tmp_ret = sin(operand0 * RADIAN_FACTOR);
		snprintf(dec_result, sizeof(dec_result), "%lf", tmp_ret);
		strcpy(CALCULATOR_GET_NODE_DATA(node)->tmp_result, dec_result);
		if (!__calculator_check_overflow
		    (atof(CALCULATOR_GET_NODE_DATA(node)->tmp_result),
		     error_msg)) {
			return TRUE;
		}
		break;
	case 'c':
		tmp_ret = cos(operand0 * RADIAN_FACTOR);
		snprintf(dec_result, sizeof(dec_result), "%lf", tmp_ret);
		strcpy(CALCULATOR_GET_NODE_DATA(node)->tmp_result, dec_result);

		if (!__calculator_check_overflow
		    (atof(CALCULATOR_GET_NODE_DATA(node)->tmp_result),
		     error_msg)) {
			return TRUE;
		}
		break;
	case 't':
		if (FLOAT_EQUAL(fmod(operand0, 180), 90) || FLOAT_EQUAL(fmod(operand0, 180), -90))	//revise by bfl
		{
			return TRUE;
		}
		tmp_ret = tan(operand0 * RADIAN_FACTOR);
		snprintf(dec_result, sizeof(dec_result), "%lf", tmp_ret);
		strcpy(CALCULATOR_GET_NODE_DATA(node)->tmp_result, dec_result);
		if (!__calculator_check_overflow
		    (atof(CALCULATOR_GET_NODE_DATA(node)->tmp_result),
		     error_msg)) {
			return TRUE;
		}
		break;
	case 'l':
		if (operand0 < 0) {
			strcat(error_msg, CALC_MSG_INVALID_LOG);
			return TRUE;
		} else if (FLOAT_EQUAL(operand0, 0)) {
			strcat(error_msg, CALC_MSG_INVALID_LOG);
			return TRUE;
		}
		set.emax = 999999;
		set.emin = -999999;
		decNumberLog10(&dresult, &doperand0, &set);
		decNumberToString(&dresult, dec_result);
		strcpy(CALCULATOR_GET_NODE_DATA(node)->tmp_result, dec_result);
		if (!__calculator_check_overflow
		    (atof(CALCULATOR_GET_NODE_DATA(node)->tmp_result),
		     error_msg)) {
			return TRUE;
		}
		break;
	case 'b':
		tmp_ret = fabs(operand0);
		snprintf(dec_result, sizeof(dec_result), "%lf", tmp_ret);
		strcpy(CALCULATOR_GET_NODE_DATA(node)->tmp_result, dec_result);
		if (!__calculator_check_overflow
		    (atof(CALCULATOR_GET_NODE_DATA(node)->tmp_result),
		     error_msg)) {
			return TRUE;
		}
		break;

	case '2':
		decNumberFromString(&doperand1, "2", &set);
		decNumberPower(&dresult, &doperand1, &doperand0, &set);
		decNumberToString(&dresult, dec_result);
		strcpy(CALCULATOR_GET_NODE_DATA(node)->tmp_result, dec_result);
		if (!__calculator_check_overflow
		    (atof(CALCULATOR_GET_NODE_DATA(node)->tmp_result),
		     error_msg)) {
			return TRUE;
		}
		break;

	default:
		break;
	}
	/*CALCULATOR_GET_NODE_DATA(node)->tmp_result *=
	    CALCULATOR_GET_NODE_DATA(node)->negative_flag;
	CALCULATOR_GET_NODE_DATA(node)->negative_flag = 1;*/

	return FALSE;
}

/**
* @describe
*
*
* @param    tree
* @param    result
* @param    error_msg
* @return    gboolean
* @exception
*/
static gboolean
__calculator_calculate_exec(GNode * tree, double **result, char *error_msg)
{
	guint depth = 0;
	depth = g_node_max_height(tree);

	if (depth < 1) {
		strcat(error_msg, CALC_MSG_WRONG_FORMAT);
		return FALSE;	//break the recursion
	} else if (depth == 1) {
		**result = atof(CALCULATOR_GET_NODE_DATA(tree)->tmp_result);
		**result *= CALCULATOR_GET_NODE_DATA(tree)->negative_flag;
		return TRUE;
	}
	g_node_traverse(tree,
			G_POST_ORDER,
			G_TRAVERSE_NON_LEAFS,
			depth,
			__calculator_calculate_exec_traverse_func,
			(gpointer) error_msg);

	if (strlen(error_msg) > 0) {
		return FALSE;
	} else {
		**result = atof(CALCULATOR_GET_NODE_DATA(tree)->tmp_result);
	}
	return TRUE;
}

/**
* @describe
*
*
* @param    tmp_result
* @return    gboolean
* @exception
*/

#if 0

/*
bool calculator_calculate_truncate_result(double* tmp_result)
{

	return TRUE;
}
*/
#endif

/**
* @describe
*
*
* @param    szInputString
* @return    int
* @exception
*/
int calculator_get_open_braket(const char *szInputString)
{
	int nReversIndex = strlen(szInputString) - 1;
	int nOpenCnt = 0;
	int nCloseCnt = 0;

	while (nReversIndex >= 0) {
		if (szInputString[nReversIndex] == '(') {
			nOpenCnt++;
		} else if (szInputString[nReversIndex] == ')') {
			nCloseCnt++;
		}
		nReversIndex--;
	}
	return (nOpenCnt - nCloseCnt);
}

/**
* @describe
*
*
* @param    szInput
* @param    [out]pDigitCnt : count of digit before point(.)
* @param    [out]pPointCnt: count of digit after point(.)
* @return    int
* @exception
*/
bool calculator_get_digits_number(char *szInput, int *pDigitCnt, int *pPointCnt)
{
	int nLen = strlen(szInput);
	int nIndex;

	int nTempCnt = 0;
	int nTempCntb = 0;

	*pDigitCnt = 0;
	*pPointCnt = 0;

	if (nLen > 0) {
		for (nIndex = nLen - 1; nIndex >= 0; nIndex--) {
			if (isdigit(szInput[nIndex])) {
				nTempCnt++;
			} else if (szInput[nIndex] == decimal_ch) {
				*pPointCnt = nTempCnt;
				nTempCnt = 0;
				break;
			}
		}
		for (nIndex = 0; nIndex < nLen; nIndex++) {
			if (isdigit(szInput[nIndex])) {
				nTempCntb++;
			} else if (szInput[nIndex] == decimal_ch) {
				*pDigitCnt = nTempCntb;
				break;
			}
		}
		//*pDigitCnt = nTempCntb;
	}
	return true;
}

/**
* @describe
*
*
* @param    str
* @param    error_msg
* @return    bool
* @exception
*/
bool calculator_expression_length_check(char *str, char *error_msg)
{
	int is_digit = 0, has_dot = 0;
	int idx = 0, nCount = 0, nDigitCnt = 0, nPointCnt = 0;
	char *p = NULL;
	char c;

#if 0
/*

	if(strlen(str) > CALCULATOR_MAX_INPUT_DIGIT_NUMBER_POR)
	{
		strcat(error_msg, CALC_MSG_MAX_INPUT);
		return FALSE;
	}
*/
#endif

	p = str;
	while (idx < strlen(str)) {
		c = p[idx];

		if (isdigit(c)) {
			if (is_digit == 0) {
				is_digit = 1;

				nDigitCnt = 1;
				nCount = 1;
			} else {
				nCount++;
				if (nCount > MAX_NUM_LENGTH) {
					strcat(error_msg, CALC_MSG_MAX_DIGIT);
					return FALSE;
				}

				if (has_dot == 0) {
					nDigitCnt++;
				} else {
					nPointCnt++;

#if 0				//YangQ
					/*
					   if(nPointCnt > CALCULATORUI_MAX_INPUT_DECIMALS)
					   {
					   strcat(error_msg, CALC_MSG_MAX_DEC_DIGIT);
					   PTAG;
					   return FALSE;
					   }
					 */
#endif
				}
			}
		}
		else if (c == decimal_ch) {
			if (has_dot == 1) {
				return FALSE;
			} else {
				has_dot = 1;
			}
		} else {
			if (is_digit == 1) {
				is_digit = 0;
				has_dot = 0;
				nCount = 0;
				nDigitCnt = 0;
				nPointCnt = 0;
			}
		}

		idx++;
	}

	return TRUE;

}

/**
* @describe
*
*
* @param    string
* @param    result
* @param    error_msg
* @return    bool
* @exception
*/
bool calculator_calculate(gchar * string, gdouble * result, char *error_msg)
{
#if 1				//YangQ add.
	string_replace(string, "E+", "x10^");
	string_replace(string, "E-", "x0.1^");
#endif
	GNode *tree = NULL;
	int end_idx = 0;

	memset(error_msg, 0, MAX_ERROR_MESSAGE_LENGTH);
	memset(g_stack, 0, sizeof(g_stack));
	oper_num = 0;

	strncpy(g_stack, string, MAX_EXPRESSION_LENGTH - 1);
	g_stack[MAX_EXPRESSION_LENGTH - 1] = '\0';

	char *digit = "0123456789pe";
	if (strcspn(g_stack, digit) == strlen(g_stack)) {	/* no digit in expression */
		strcat(error_msg, CALC_MSG_WRONG_FORMAT);
		return FALSE;
	}

	if (!calculator_expression_length_check(g_stack, error_msg)) {
		return FALSE;
	}
	if (!__calculator_calculate_formula_scan(g_stack, &end_idx, error_msg)) {
		return FALSE;
	}

	if (!__calculator_calculate_formula_parse
	    (result, &tree, g_stack, end_idx, error_msg)) {
		if (tree != NULL) {
			__calculator_tree_destroy_all(&tree);
		} else {
			LOGD("current tree is null\n");
		}
		return FALSE;
	}
	if (!__calculator_calculate_exec(tree, &result, error_msg)) {
		if (tree != NULL) {
			__calculator_tree_destroy_all(&tree);
		}
		return FALSE;
	}
	if (!__calculator_check_overflow(*result, error_msg)) {
		if (tree != NULL) {
			__calculator_tree_destroy_all(&tree);
		}
		return FALSE;
	}

	if (tree != NULL) {
		__calculator_tree_destroy_all(&tree);
	}
	oper_num = 0;
	return TRUE;
}
