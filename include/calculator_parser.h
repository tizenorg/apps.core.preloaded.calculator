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

#ifndef __CALCULATOR_PARSER_H
#define __CALCULATOR_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

#include <glib.h>

	typedef enum {
		CHAR_IS_NULL = 0,	//
		CHAR_IS_DIGIT,	//0,1,2...
		CHAR_IS_LEFT_PARENTHESE,	//(
		CHAR_IS_RIGHT_PARENTHESE,
		CHAR_IS_PLUS_MINUS,	//+/-
		CHAR_IS_PI,	//Pi
		CHAR_IS_E,	//e
		CHAR_IS_MULTIPLY_DIVIDE,	// */
		CHAR_IS_CHARACTER,	//a,P,C,^
		CHAR_IS_POINT,	//point
	} last_char_t;

/**

*@enum calculator_state_t, define calculator current state

* This enum is used to judge or record which state is current.

*/
	typedef enum {
		CALCULATOR_WAITING_INPUT = 0,		/**<waiting input state*/
		CALCULATOR_OPERAND_INPUT,			/**<operand input state*/
		CALCULATOR_OPERAND_FRACTION_INPUT,	/**<fraction operand input state*/
		CALCULATOR_OPERATOR_INPUT,			/**<[basic] operator input state*/
		CALCULATOR_OPERATOR_OPERAND_INPUT,	/**<operand after [basic] operator input state*/
		CALCULATOR_SPECIAL_SYMBOL_INPUT,	/**<special symbol inputting state*/
		CALCULATOR_SPECIAL_SYMBOL_INPUT_OVER,
						/**<special symbol inputed state*/
		CALCULATOR_SPECIAL_FUNCTION_INPUT,	/**<function[excluding basic operator] input state*/
		CALCULATOR_NAVIGATION,				/**<browser in entry state, depreated*/
		CALCULATOR_CALCULATED,				/**<calculation completed state*/
		CALCULATOR_ERROR_OCCURED,			/**<error occured state*/
	} calculator_state_t;

/**

*@enum calculator_calculate_priority_t, define calculation priorities

* This enum is used to mark operator &functions's priority.

*/
	typedef enum {
		CALCULATOR_CALCULATE_PRIORITY_INVALID = 0,
							/**<invalid priority*/
		CALCULATOR_CALCULATE_PRIORITY_LOW,		/**<lowest priority*/
		CALCULATOR_CALCULATE_PRIORITY_MIDDLE,	/**<middle priority*/
		CALCULATOR_CALCULATE_PRIORITY_HIGH,		/**<high priority*/
		CALCULATOR_CALCULATE_PRIORITY_HIGHER,	/**<higher priority*/
		CALCULATOR_CALCULATE_PRIORITY_HIGHEST,	/**<highest priority*/
	} calculator_calculate_priority_t;

/**

*@enum operator_type_t, define calculator operator type

*/
	typedef enum {
		OPERATOR_TYPE_INVALID = 0,
					/**<operator type invalid*/
		OPERATOR_TYPE_UNARY,		/**<operator unary*/
		OPERATOR_TYPE_BINARY,		/**<operator binary*/
		OPERATOR_TYPE_CNT,		/**<max count*/
	} operator_type_t;

/**

*@enum function_category_t, define calculator function category

*/
	typedef enum {
		FUNCTION_INVALID = 0,
				/**<category invalid*/
		FUNCTION_PREFIX,	/**<prefix category, operand follows functions*/
		FUNCTION_POSTFIX,	/**<postfix category, function follows operand*/
		FUNCTION_CONSTANT,
				/**<constant category, only for PI*/
		FUNCTION_CNT,		/**<max count*/
	} function_category_t;

	typedef enum {
		OP_INVALID = 0,

		/* basic pannel */
		OP_PARENTHESIS,	// 1
		OP_DELETE,
		OP_CLEAR,
		OP_DIVIDE,

		OP_NUM_7,	// 5
		OP_NUM_8,
		OP_NUM_9,
		OP_MULTIPLY,

		OP_NUM_4,	// 9
		OP_NUM_5,
		OP_NUM_6,
		OP_MINUS,

		OP_NUM_1,	// 13
		OP_NUM_2,
		OP_NUM_3,
		OP_PLUS,

		OP_DOT,		// 17
		OP_NUM_0,
		OP_PLUS_MINUS,
		OP_EQUAL,

		/* function pannel */
		OP_PERCENT,	//21
		OP_ROOT,
		OP_FACT,

		OP_SIN,		// 24
		OP_COS,
		OP_TAN,

		OP_LN,		// 27
		OP_LOG,
		OP_1X,

		OP_EX,		// 30
		OP_X2,
		OP_XY,

		OP_ABS,		// 33
		OP_PI,
		OP_E,
	} op_id_t;

	typedef struct {
		op_id_t op_id;
		char *op_sym;
		char *op_name;
	} op_item_t;

/**

* @struct calculator_node_data_t, calculator node data structure

* This structure is used as node data of n-ary tree

*/
	typedef struct {
		double tmp_result;							/**<store result by temoprary*/
		calculator_calculate_priority_t node_calcu_priority;
								/**<node calculation priority*/
		operator_type_t operator_type;					/**<node operator type*/
		int children_num;							/**<node children number*/
		char cur_operator;							/**<node operator char*/
		int negative_flag;							/**<node negative flag*/
	} calculator_node_data_t;

/**

* @struct calculator_parentheses_data_t, calculator parentheses data structure

*/
	typedef struct {
//      GNode* tree;            /**<parentheses tree node*/
		char *start_pos;/**<parentheses start position in original string*/
		char *end_pos;	/**<parentheses end position in original string*/
		bool matched;
			/**<parentheses if is matched*/
	} calculator_parentheses_data_t;

/**

* @struct function_t, calculator function data structure

*/
	typedef struct {
		char *func_name;						/**<function name, use only prompt*/
		char *func_string;						/**<function string, use in text view*/
		gint str_len;							/**<function string length*/
		char function_char;					/**<function char*/
		function_category_t category;			/**<fuction category, input after operand*/
		operator_type_t op_type;				/**<operator type*/
		calculator_calculate_priority_t priority;
							/**<calculation priority*/
		bool has_parentheses;				/**<if function has parentheses*/
	} function_t;

/**
* @describe
*
*
* @param    tmp_result
* @return    bool
* @exception
*/
//bool calculator_calculate_truncate_result(double* tmp_result);

/**
* @describe
*
*
* @param    szInputString
* @return    bool
* @exception
*/
	int calculator_get_open_braket(const char *szInputString);

/**
* @describe
*
*
* @param    szInput
* @param    pDigitCnt
* @param    pPointCnt
* @return    bool
* @exception
*/
	bool calculator_get_digits_number(char *szInput, int *pDigitCnt,
					  int *pPointCnt);

/**
* @describe
*
*
* @param    szInput
* @param    pDigitCnt
* @param    pPointCnt
* @return    bool
* @exception
*/
	bool calculator_calculate(char *string, double *result,
				  char *error_msg);

#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif
