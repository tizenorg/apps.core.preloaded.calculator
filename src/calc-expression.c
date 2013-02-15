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

#include <string.h>
#include <stdbool.h>
#include <stdio.h>		//snprintf
#include <ctype.h>		//isdigit
#include <math.h>
#include "calc-main.h"
#include "calc-string.h"
#include "calc-expression.h"

/* special characters */
#define PI_S 					"\xcf\x80"
#define DIVIDE 					"\xc3\xb7"
#define MULTIPLY 				"\xc3\x97"
#define SQRT					"\xe2\x88\x9a"
#define MINUS					"\xe2\x88\x92"
extern char decimal_ch;
extern char separator_ch;
extern int scientific_result_len;
struct calc_func_t calc_func[] = {
	    {OP_PERCENT, "%"},
    {OP_ROOT, "sqrt("},
    {OP_FACT, "!"},
    {OP_SIN, "sin("},
    {OP_COS, "cos("},
    {OP_TAN, "tan("},
    {OP_LN, "ln("},
    {OP_LOG, "log("},
    {OP_1X, "1/x"},
    {OP_10X, "10^("}, {OP_X2, "^2"}, {OP_XY, "^("}, {OP_ABS, "abs("},
};


/*
 * format number with comma
 */
static void _calc_expr_format_number(const char *in, char *out)
{
	char number[NUMBER_LENGTH] = { 0 };
	char *dot = strchr(in, decimal_ch);
	int length = (dot == NULL ? strlen(in) : dot - in);
	bool negative = (in[0] == '-' ? true : false);
	length -= (negative ? 1 : 0);	/* the number length between '-' and '.' */
	strncpy(number, negative ? in + 1 : in, length);
	int comma_count = length / 3;	/* the count of comma that will be add */
	comma_count -= (length % 3 == 0 ? 1 : 0);
	int p = length - 3;	/* insert ',' at number[p] */
	while (comma_count--)
		 {
		int l = length;
		while (l > p)
			 {
			number[l] = number[l - 1];
			l--;
			}
		number[p] = separator_ch;
		length++;
		p -= 3;
		}
	char *s = out;
	if (negative)
		 {
		out[0] = '-';
		s += 1;
		}
	strncpy(s, number, length);
	s[length] = '\0';
	if (dot != NULL)
		 {
		strcpy(s + length, dot);
		}
}


/**
 * @description
 *  Get the fisrt number's postion in the expression string.
 *
 * @param       expr    Expression string
 * @param       begin   The index of fisrt digit(maybe '-')
 * @param       end     The index of last digit
 * @return      bool
 * @exception   none
 */
static bool _calc_expr_get_number_position(const char *expr, int *begin,
					   int *end)
{
	if (expr == NULL) {
		return false;
	}
	bool negative = false;
	char *digit = "0123456789";
	char digit_with_dot[32] = { 0 };
	snprintf(digit_with_dot, sizeof(digit_with_dot), "%s%c", digit,
		  decimal_ch);
	int b = strcspn(expr, digit);
	if (b == strlen(expr)) {
		return false;
	}
	if (b > 0 && expr[b - 1] == '-')
		 {

		    /* if before '-' is ')' or digit, I cosider it as minus. */
		    if (b > 1 && (expr[b - 2] == ')' || isdigit(expr[b - 2])))
			 {
			negative = false;
			}

		else
			 {
			negative = true;
			}
		}
	int length = strspn(expr + b, digit_with_dot);
	*begin = (negative ? b - 1 : b);
	*end = b + length - 1;
	return true;
}

/*
 * replace common char with special locally.
 */
void calc_expr_replace_with_special_char(char *expr)
{
	CALC_FUN_BEG();
	string_replace(expr, "p", PI_S);
	string_replace(expr, "x", MULTIPLY);
	string_replace(expr, "/", DIVIDE);
	//string_replace(expr,"-",MINUS);
	string_replace(expr, "sqrt", SQRT);
	CALC_FUN_END();
}

void calc_expr_replace_from_special_char(char *expr)
{
	CALC_FUN_BEG();
	string_replace(expr, PI_S, "p");
	string_replace(expr, MULTIPLY, "x");
	string_replace(expr, DIVIDE, "/");
	string_replace(expr,MINUS,"-");
	string_replace(expr, SQRT, "sqrt");
	CALC_FUN_END();
}

/*
 * convert string for calculate to string for display
 */
void calc_expr_format_expression(const char *expr_in, char *expr_out)
{
	CALC_FUN_BEG();
	char number1[NUMBER_LENGTH] = { 0 };
	char number2[NUMBER_LENGTH] = { 0 };
	const char *in = expr_in;
	char *out = expr_out;
	int begin, end;
	while (_calc_expr_get_number_position(in, &begin, &end)) {
		int l = end - begin + 1;
		strncpy(number1, in + begin, l);
		number1[l] = '\0';
		_calc_expr_format_number(number1, number2);
		strncpy(out, in, begin);
		out += begin;
		strcpy(out, number2);
		in = in + end + 1;
		out = out + strlen(out);
	  }
	  strcpy(out, in);
	calc_expr_replace_with_special_char(expr_out);
	CALC_FUN_END();
}

int calc_expr_get_operator_num(const char *expr)
{
	if (expr == NULL) {
		return 0;
	}
	char *operator = "+-x/^";
	int count = 0;
	int i = 0;
	while (expr[i]) {
		int j = 0;
		while (operator[j]) {
			if (operator[j] == expr[i]) {
				count++;
				break;
			}
			++j;
		}
		++i;
	 }
	return count;
}

int calc_expr_get_left_parentheses_num(const char *expr)
{
	if (expr == NULL) {
		return 0;
	}
	int i = 0;
	int count = 0;
	while (expr[i] != '\0') {
		if (expr[i] == '(') {
			count++;
		}
		i++;
	}
	return count;
}

void calc_expr_num_remove_end_zero(char *number)
{
	int len = strlen(number);
	if (strchr(number, 'e') != NULL || strchr(number, 'E') != NULL) {
		return;
	}
	if (strchr(number, decimal_ch) != NULL) {
		while (len > 0 && number[len - 1] == '0') {
			number[len - 1] = '\0';
			len--;
		}
		if (len > 0 && number[len - 1] == decimal_ch) {
			number[len - 1] = '\0';
			len--;
		}
	}
}

/**
* @description
* Format calculate result to show it suitably in the entry
* Mainly, remove end zero, if too long, change it to scientific form
*
* @param[in]	result	The calculate result
* @param[out]	text	The format string of the result
* @return	void
* @exception
*/
void calc_expr_num_format_result(double result, char *text)
{
	CALC_FUN_BEG();
	char buf[MAX_EXPRESSION_LENGTH] = { 0 };
	if ((fabs(result) - 0.00001) < 0) {
		/* Solve the bug of "-0" */
		if (ceil(fabs(result)) - 0 < 0.000000000001) {
			strcpy(text, "0");
			return;
		}
		snprintf(buf, sizeof(buf), "%.*E", scientific_result_len, result);
	} else {
		snprintf(buf, sizeof(buf), "%.*lf", MAX_DECIMAL_NUM, result);
		calc_expr_num_remove_end_zero(buf);
		double temp_result = (floor)(fabs(result));
		char temp_buf[MAX_EXPRESSION_LENGTH] = { 0 };
		snprintf(temp_buf, sizeof(buf), "%lf",temp_result);
		calc_expr_num_remove_end_zero(temp_buf);
		int max_lengh = isdigit(buf[0])? MAX_NUM_LENGTH:(MAX_NUM_LENGTH+1);
		if (strlen(buf) > max_lengh) {
			if (strlen(temp_buf) <= max_lengh) {
				memset(temp_buf, 0, sizeof(temp_buf));
				snprintf(temp_buf, sizeof(temp_buf), "%s", buf);
				memset(buf, 0, sizeof(buf));
				if (isdigit(temp_buf[max_lengh-1])) {
					snprintf(buf, sizeof(buf), "%.*s",max_lengh,temp_buf);
				} else {
					snprintf(buf, sizeof(buf), "%.*s",max_lengh-1,temp_buf);//delete decimal point
				}
			} else {
				double revise_result = result;
				/* 12345678650*100,000 The result should be 1.23456787+E15, not 1.23456786+E15. */
				if (buf[9] == '5') {
					buf[9] = '6';
					sscanf(buf, "%lf", &revise_result);
				}
				snprintf(buf, sizeof(buf), "%.*E", scientific_result_len, revise_result);
			}
		}
	}
	strcpy(text, buf);
	CALC_FUN_END();
}


/**
 * Get fucntion at current cursor position.
 * If get successfully return the function symbol.
 * If at the cursor position is not a function, return NULL.
 */
char *calc_expr_get_current_func_at_cursor(char *expr, int cursor)
{
	if (cursor == 0) {
		return NULL;
	}
	int pos = cursor - 1;	//previous cursor position
	int func_num = sizeof(calc_func) / sizeof(calc_func[0]);	//the number of fucntions
	int i = 0;
	char *ch = NULL;
	for (i = 0; i < func_num; ++i) {
		if ((ch = strchr(calc_func[i].symbol, expr[pos])) == NULL){
			continue;
		}
		int t = pos - (ch - calc_func[i].symbol);
		if (t < 0) {
			continue;
		}
		if (!strncmp(calc_func[i].symbol, expr + t, strlen(calc_func[i].symbol)))	//equal
		{
			return calc_func[i].symbol;
		}
	}
	return NULL;
}


/**
 * Parse the first number and give the number length in the expression.
 * @param   exp     the expression string
 * @param   number  the first number in the expression
 * @param   length  the number's length in the expression string
 * @return  int     0 if parse succefully, else return -1
 */
int calc_expr_parse_number(const char *exp, double *number, int *length)
{
	const char *ptr = IS_SIGN(exp[0]) ? exp + 1 : exp;
	char *digit = "0123456789";
	int t = strspn(ptr, digit);
	if (t == 0) {
		return -1;
	}			/* has no digit */
	ptr += t;
	if (ptr[0] == decimal_ch) {
		ptr += 1;
		t = strspn(ptr, digit);
		ptr += t;
	}
	if (IS_SCIENCE_E(ptr[0]))	/* science number */ {
		ptr += 1;
		ptr += IS_SIGN(ptr[0]) ? 1 : 0;
		t = strspn(ptr, digit);
		ptr += (t == 0 && IS_SIGN(ptr[-1]) ? -1 : t);	/* "1.0E-" are not allowed */
	}
	int len = ptr - exp;
	double num = 0.0;
	char *buf = (char *)malloc(len + 1);
	if (buf == NULL) {
		return -1;
	}
	strncpy(buf, exp, len);
	buf[len] = '\0';
	sscanf(buf, "%lf", &num);
	free(buf);
	if (number != NULL) {
		*number = num;
	}
	if (length != NULL) {
		*length = len;
	}
	return 0;		/* successfully */
}


/*
 * Return the current number position at the cursor in expr
 * @expr, the expression string
 * @cursor, the cursor postion
 * @[out]begin, the current number begin position
 * @[out]length, the current number's length
 * @return, return 0 if get succefully, else return -1
 */
int calc_expr_get_current_num_at_cursor(char *expr, int cursor, int *begin,
					int *length)
{
	int index = cursor;
	int _begin = 0;
	int _length = 0;
	int counter = 0;
	if (expr[index] == 'p' || expr[index] == 'e') {
		if (index > 0 && expr[index - 1] == '-') {	//-e  OR -p
			_begin = index - 1;
			_length = 2;
		 }
		  else {
			_begin = index;
			_length = 1;
		  }
	 }
	 else {
		if (expr[index] == 'E' || expr[index] == decimal_ch) {
			index--;
		 }
		while (calc_expr_parse_number(expr + index, NULL, &_length) ==
			0){
			counter++;	/* flag */
			if (index + _length <= cursor) {
				index += 1;
				break;
			 }
			if (index == 0)
				break;
			index--;
			if (expr[index] == 'E' || expr[index] == decimal_ch)
				index--;
			}
			if (counter == 0) {
				return -1;
			 }		/* not a number */
			if (0 != calc_expr_parse_number(expr + index, NULL, NULL)) {
				index += 1;
			}
		    calc_expr_parse_number(expr + index, NULL, &_length);
		   if (IS_SIGN(expr[index])) {
				if (index > 0
				     && (isdigit(expr[index - 1])
					 || expr[index - 1] == ')')) {
					index += 1;
					_length -= 1;
				 }
			}
		   _begin = index;
	  }
	if (begin != NULL) {
		  *begin = _begin;
	  }
	 if (length != NULL) {
		  *length = _length;
	  }
	return 0;		/* successfully */
}


/*
 * Auto close parentheses at the end of exp.
 */
void calc_expr_close_parentheses(char *exp)
{
	int cnt_l = 0, cnt_r = 0;
	int i = 0;
	for (i = 0; exp[i]; ++i) {
		if ('(' == exp[i]) {
			cnt_l++;
		 }
		if (')' == exp[i]) {
		 	cnt_r++;
		 }
	 }
	int n = cnt_l - cnt_r;
	while (n > 0) {
		strcat(exp, ")");
		n--;
	 }
}


/**
 * According "[Fraser] UI_Calculator_v0.5_20101125.pdf",
 *  *123+2=125
 *  /123+2=125
 * Enter operator at first are allowed.
 * So it should remove 'x' or '/' at the first position.
 */
void calc_expr_remove_first_operator(char *exp)
{
	if (exp != NULL && strlen(exp) > 0) {
		if (exp[0] == 'x' || exp[0] == '/' || exp[0] == '+') {
			string_remove_at(exp, 0, 1);
		 }
	 }
}


/**
 * @description
 * According "[Fraser] UI_Calculator_v0.5_20101125.pdf",
 *  123+= : 123+123=246
 *  123-= : 123-123=0
 *  123*= : 123*123=15129
 *  123/= : 123/123=1
 * The operator at last are allowed.
 * The function process these conditions.
 *
 * @param       exp
 * @return      void
 * @exception   none
 */
void calc_expr_process_last_operator(char *exp)
{
	int len = strlen(exp);
	if (len > 1 && IS_OPERATOER(exp[len - 1])) {
		int b, l;
		if (0 ==
		     calc_expr_get_current_num_at_cursor(exp, len - 2, &b, &l)) {
			strncat(exp, exp + b, l);
			exp[len + l] = '\0';
		 }
	 }
}


/*
 * Preprocess befeore calculating.
 */
void calc_expr_preprocess(char *exp)
{
	calc_expr_close_parentheses(exp);
	calc_expr_remove_first_operator(exp);
	calc_expr_process_last_operator(exp);
}

/**
 * @description
 * Search the selection string is a legal string
 *
 * @param       str
 * @return      bool
 * @exception  none
 */
bool calc_select_string_search(char *str)
{
	int i = 0;
	calc_expr_replace_from_special_char(str);
	int length = strlen(str);
	while (i  <  length) {
		if (IS_DIGITAL(str[i]) || IS_OPERATOER(str[i])
		|| (str[i] == decimal_ch) || (str[i] == separator_ch)
		|| (str[i] == 'i')  || (str[i] == '(') || (str[i] == ')')) {
			i++;
		} else {
			if (str[i] == 's') {
				if (str[i+1] == 'i' && str[i+2] == 'n') {
					i = i + 3;
				} else if (str[i+1] == 'q' && str[i+2] == 'r' && str[i+3] == 't') {
					i = i + 4;
				} else {
					return FALSE;
				}
			} else if (str[i] == 'c') {
				if (str[i+1] == 'o' && str[i+2] == 's') {
					i = i + 3;
				} else {
					return FALSE;
				}
			} else if (str[i] == 't') {
				if (str[i+1] == 'a' && str[i+2] == 'n') {
					i = i + 3;
				} else {
					return FALSE;
				}
			} else if (str[i] == 'a') {
				if (str[i+1] == 'b' && str[i+2] == 's') {
					i = i + 3;
				} else {
					return FALSE;
				}
			} else if (str[i] == 'l') {
				if (str[i+1] == 'o' && str[i+2] == 'g') {
					i = i + 3;
				} else if (str[i+1] == 'n') {
					i = i + 2;
				} else {
					return FALSE;
				}
			} else {
				CALC_INFO_PURPLE("str[i]=%c, i=%d", str[i], i);
				return FALSE;
			}
		}
	}
	return TRUE;
}

/*
 * process expression input backspace
 * @[in/out]expr, the expression string
 * @[in/out]cursor, the cursor postion
 * When the fuction return, the exp and cursor will be updated.
 */
void calc_expr_input_backspace(char *exp, int *cursor)
{
	int _cursor = *cursor;
	int count = 0;
	if (_cursor <= 0) {
		return;
	}
	if (_cursor > strlen(exp)) {
		_cursor = strlen(exp);
	}			/* set to the end */
	char *func = calc_expr_get_current_func_at_cursor(exp, _cursor);
	if (NULL == func) {
		count = 1;
		_cursor -= 1;
	} else {
		int p = _cursor;
		while (p >= 0 && (strncmp(func, exp + p, strlen(func)) != 0)) {
			p--;
		}
		count = strlen(func);
		_cursor = p;
	}
	string_remove_at(exp, _cursor, count);
	*cursor = _cursor;
}


/*
 * insert str in exp at sursor position
 * @[in/out]expr, the expression string
 * @[in/out]cursor, the cursor postion
 * @[in]str, the string insert into
 * When the fuction return, the exp and cursor will be updated.
 */
void calc_expr_input_insert(char *exp, int *cursor, char *str)
{
	int _cursor = *cursor;
	if (_cursor < 0) {
		return;
	}
	if (_cursor > strlen(exp)) {
		_cursor = strlen(exp) + 1;
	}			/* set to the end */
	string_insert(exp, _cursor, str);
	*cursor = _cursor + strlen(str);
}


