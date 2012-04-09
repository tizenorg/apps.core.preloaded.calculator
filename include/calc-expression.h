/*
 * Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * This file is part of calculator
 * Written by Zhao Danni <danni.zhao@samsung.com>
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * This software is the confidential and proprietary information of
 * SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered
 * into with SAMSUNG ELECTRONICS.
 *
 * SAMSUNG make no representations or warranties about the suitability
 * of the software, either express or implied, including but not limited
 * to the implied warranties of merchantability, fitness for a particular
 * purpose, or non-infringement. SAMSUNG shall not be liable for any
 * damages suffered by licensee as a result of using, modifying or
 * distributing this software or its derivatives.
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

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* __DEF_CALC_EXPRESSION_H_ */

