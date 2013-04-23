/*
*
* Copyright 2012  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the License);
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org/license
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an AS IS BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#ifndef __DEF_CALC_EXPRESSION_H_
#define __DEF_CALC_EXPRESSION_H_

#ifdef __cplusplus
extern "C" {

#endif	/* __cplusplus */

#include "calculator_parser.h"

#define IS_SIGN(ch)         ((ch) == '+' || (ch) == '-')
#define IS_SCIENCE_E(ch)    ((ch) == 'E' || (ch) == 'e')
#define IS_OPERATOER(ch)    ((ch) == '+' || (ch) == '-' || (ch) == 'x' || (ch) == '/' || (ch) == '^')
#define IS_DIGITAL(ch)		(isdigit((ch)) || (ch) == 'p' || (ch) == 'e')

/* */
#define FLOAT_EQUAL(a, b)       (fabs((a) - (b)) < 0.000000000001)
	  struct calc_func_t  {
		op_id_t id;
		char *symbol;
	};

/**
* @describe
*
*
* @param    expr_in
* @param    expr_out
* @return    void
* @exception
*/
	void calc_expr_format_expression(const char *expr_in, char *expr_out);

/**
* @describe
*
*
* @param    expr
* @return    int
* @exception
*/
	int calc_expr_get_operator_num(const char *expr);

/**
* @describe
*
*
* @param    expr
* @return    int
* @exception
*/
	int calc_expr_get_left_parentheses_num(const char *expr);

/**
* @describe
*
*
* @param    expr
* @return    void
* @exception
*/
	void calc_expr_replace_with_special_char(char *expr);

/**
* @describe
*
*
* @param    expr
* @return    void
* @exception
*/
	void calc_expr_replace_from_special_char(char *expr);

/**
* @describe
*
*
* @param    result
* @param    text
* @return    void
* @exception
*/
	void calc_expr_num_format_result(double result, char *text);

/**
* @describe
*
*
* @param    expr
* @param    cursor
* @return    void
* @exception
*/
	char *calc_expr_get_current_func_at_cursor(char *expr, int cursor);

/**
* @describe
*
*
* @param    expr
* @param    cursor
* @param    begin
* @param    length
* @return    int
* @exception
*/
	int calc_expr_get_current_num_at_cursor(char *expr, int cursor,
						int *begin, int *length);

/**
* @describe
*
*
* @param    expr
* @return    void
* @exception
*/
	void calc_expr_close_parentheses(char *exp);

/**
* @describe
*
*
* @param    expr
* @return    void
* @exception
*/
	void calc_expr_preprocess(char *exp);

/**
* @describe
*
*
* @param    expr
* @param    cursor
* @return    void
* @exception
*/
	void calc_expr_input_backspace(char *exp, int *cursor);

/**
* @describe
*
*
* @param    expr
* @param    cursor
* @param    cursor
* @return    str
* @exception
*/
	void calc_expr_input_insert(char *exp, int *cursor, char *str);

bool calc_select_string_search(char *str);


#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* __DEF_CALC_EXPRESSION_H_ */

