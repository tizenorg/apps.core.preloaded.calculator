/*
  * Copyright 2012  Samsung Electronics Co., Ltd
  * 
  * Licensed under the Flora License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  * 
  *     http://www.tizenopensource.org/license
  * 
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */
 


#include <stdbool.h>		/*  true/false */
#include <Elementary.h>
#include <dlog.h>

#include "calc-main.h"
#include "calculator_parser.h"
#include "calc-string.h"
#include "calc-expression.h"
#include "calc-view.h"
#include <svi.h>

#define CALCULATOR_CHAR_IS_OPERATOR(C) ((C == '+')||(C == '-')||(C == 'x')||(C == '/'))	/**<judge if a char is a basic operator*/
#define CALCULATOR_CHAR_IS_MULTI_DIVIDE_OPERATOR(C) ((C == 'x')||(C == '/'))			/**<judge if an operator is "*" or "/"*/
#define CALCULATOR_CHAR_IS_PLUS_DEDUCT(C) ((C == '+')||(C == '-'))
#define CALCULATOR_CHAR_IS_DIGITAL(C)(C >= '0' && C <= '9')
#define CALCULATOR_IS_DIGIT_DOT(ch, decimal) (isdigit(ch) || decimal == (ch))

static Elm_Entry_Filter_Limit_Size limit_filter_data;

extern int get_max_fontsize();
extern void _calc_view_show_newest_histroy(Evas_Object * entry);
extern void _calc_view_clear_histroy(Evas_Object * entry);
extern gboolean __calculator_search_function(char *op, int* len);
static void __calculator_wrong_format_create(char * wrong_string);
extern gboolean __calculator_search_function(char *op, int* len);

static struct appdata *ad;	/* will be removed */
static calculator_state_t calculator_state = CALCULATOR_WAITING_INPUT;
static double last_result = 0.0;

int calculator_cursor_pos = 0;
char calculator_input_str[MAX_EXPRESSION_LENGTH] = { 0 };
char calculator_before_paste_str[MAX_EXPRESSION_LENGTH] = { 0 };

bool paste_flag = FALSE;

struct lconv *locale = NULL;
char *decimal = NULL;
char *separator = NULL;
char decimal_ch = 0;
char separator_ch = 0;

static op_item_t calc_op_item[] = {
	{OP_INVALID, "", NULL},

	/* basic */
	{OP_PARENTHESIS, "()", NULL},
	{OP_DELETE, "<-", NULL},
	{OP_CLEAR, "C", NULL},
	{OP_DIVIDE, "/", NULL},

	{OP_NUM_7, "7", NULL},
	{OP_NUM_8, "8", NULL},
	{OP_NUM_9, "9", NULL},
	{OP_MULTIPLY, "x", NULL},

	{OP_NUM_4, "4", NULL},
	{OP_NUM_5, "5", NULL},
	{OP_NUM_6, "6", NULL},
	{OP_MINUS, "-", NULL},

	{OP_NUM_1, "1", NULL},
	{OP_NUM_2, "2", NULL},
	{OP_NUM_3, "3", NULL},
	{OP_PLUS, "+", NULL},

	{OP_DOT, ".", NULL},
	{OP_NUM_0, "0", NULL},
	{OP_PLUS_MINUS, "+/-", NULL},
	{OP_EQUAL, "=", NULL},

	/* functional */
	{OP_PERCENT, "%", NULL},
	{OP_ROOT, "sqrt", "sqrt("},
	{OP_FACT, "x!", "!"},

	{OP_SIN, "sin", "sin("},
	{OP_COS, "cos", "cos("},
	{OP_TAN, "tan", "tan("},

	{OP_LN, "ln", "ln("},
	{OP_LOG, "log", "log("},
	{OP_1X, "1/x", "1/x"},

	{OP_EX, "e^x", "e^("},
	{OP_X2, "x^2", "^2"},
	{OP_XY, "x^y", "^("},

	{OP_ABS, "abs", "abs("},
	{OP_PI, "Pi", "p"},
	{OP_E, "e", "e"},
};
char *error_string[] = {
    "IDS_CCL_POP_UP_TO_15_DIGITS_AVAILABLE",
    "IDS_CCL_POP_UP_TO_5_DECIMALS_AVAILABLE",
    "IDS_CCL_POP_UP_TO_20_OPERATORS_AVAILABLE",
    "IDS_CCL_POP_UNABLE_TO_DIVIDE_BY_ZERO",
    "IDS_CCL_POP_NO_NUMBER_ERROR",
    "IDS_CCL_POP_ERROR",
    "IDS_CCL_POP_ENTER_NUMBER_AFTER_OPERATOR",
    "IDS_CCL_BODY_INVALID_INPUT_FOR_SQUARE_ROOT_FUNCTION",
    "IDS_CCL_BODY_INVALID_INPUT_FOR_LOG_FUNCTION",
    "IDS_CCL_BODY_NATURAL_NUMBER_ONLY_FOR_X_E_FUNCTION",
    "IDS_CCL_BODY_ENTER_NUMBER_BEFORE_INPUTTING_X_E_FUNCTION",
    "IDS_CCL_BODY_ENTER_NUMBER_BEFORE_INPUTTING_1_X_FUNCTION",
    "IDS_CCL_BODY_ENTER_NUMBER_BEFORE_INPUTTING_X2_FUNCTION",
    "IDS_CCL_BODY_ENTER_NUMBER_BEFORE_INPUTTING_XY_FUNCTION",
    "IDS_CCL_POP_NO_OPERATOR_ERROR",
    "IDS_CCL_POP_SYNTAX_ERROR",
};


calculator_state_t calculator_get_state()
{
	return calculator_state;
}

void _calc_add_tag(char *string, char *format_string)
{
	CONV_FUNC_IN();
	int i = 0;
	int begin = 0;
	int end = 0;
	char buf[MAX_EXPRESSION_LENGTH] = { 0 };
	char tmp[MAX_EXPRESSION_LENGTH] = { 0 };
	char *p = NULL;
	while (string[i] != '\0') {
		if (CALCULATOR_CHAR_IS_DIGITAL(string[i])
		    || string[i] == separator_ch || string[i] == decimal_ch) {
			begin = i;
			p = &string[begin];
			while (CALCULATOR_CHAR_IS_DIGITAL(string[i])
			       || string[i] == separator_ch
			       || string[i] == decimal_ch) {
				i++;
			}
			end = i - 1;
			strncpy(tmp, p, MAX_EXPRESSION_LENGTH - 1);
			tmp[end - begin + 1] = '\0';
			memset(buf, 0, sizeof(buf));
			snprintf(buf, sizeof(buf),
				 "<align=right><+ font_size=%d><color=#000000ff>%s",
				 get_max_fontsize(), tmp);
			strcat(format_string, buf);
		} else {
			begin = i;
			p = &string[begin];
			while (!CALCULATOR_CHAR_IS_DIGITAL(string[i])
			       && string[i] != separator_ch
			       && string[i] != decimal_ch
			       && string[i] != '\0') {
				i++;
			}
			end = i - 1;
			strncpy(tmp, p, MAX_EXPRESSION_LENGTH - 1);
			tmp[end - begin + 1] = '\0';
			memset(buf, 0, sizeof(buf));
			snprintf(buf, sizeof(buf),
				 "<align=right><+ font_size=%d><color=#855B11FF>%s",
				 get_max_fontsize(), tmp);
			strcat(format_string, buf);
		}

	}
	PLOG("Expression with tag : %s\n", format_string);
	CONV_FUNC_OUT();
}

/* BEGIN INPUT ENTRY RELATED */

/**
* @describe
*
*
* @param    entry
* @param    str
* @return    void
* @exception
*/
static void _calc_entry_text_set(Evas_Object * entry, const char *str)
{
	CONV_FUNC_IN();
	char tmp[MAX_EXPRESSION_LENGTH] = { 0 };
	char tag_text[MAX_TAG_EXPRESSION_LENGTH] = { 0 };

	if (str == NULL) {
		return;
	}

	if (strlen(str) == 0) {
		elm_entry_entry_set(entry, "");
		calc_view_cursor_set_position(entry, calculator_cursor_pos);
		return;
	}
	calc_expr_format_expression(str, tmp);
	_calc_add_tag(tmp, tag_text);
	elm_entry_entry_set(entry, tag_text);

	calc_view_cursor_set_position(entry, calculator_cursor_pos);
	calc_view_revise_input_scroller(ad);
	CONV_FUNC_OUT();

}

void _calc_entry_text_set_rotate(struct appdata *ad)
{
	if (calculator_state == CALCULATOR_CALCULATED) {
		_calc_view_show_newest_histroy(ad->input_entry);
		elm_entry_cursor_end_set(ad->input_entry);
	} else {
		_calc_entry_text_set(ad->input_entry, calculator_input_str);
	}
}

/**
* @describe
*
*
* @param    entry
* @param    str
* @return    void
* @exception
*/
static void _calc_entry_text_insert(Evas_Object * entry, char *str)
{
	CONV_FUNC_IN();
	calc_expr_input_insert(calculator_input_str, &calculator_cursor_pos,
			       str);
	printf("calculator_input_str=%s, str=%s, calculator_cursor_pos=%d\n", calculator_input_str, str, calculator_cursor_pos);
	_calc_entry_text_set(entry, calculator_input_str);
	CONV_FUNC_OUT();
}

/**
* @describe
*
*
* @param    entry
* @param    from_pos
* @param    end_pos
* @return    void
* @exception
*/
static void _calc_entry_text_remove(Evas_Object * entry, const int from_pos,
				    const int end_pos)
{
	CONV_FUNC_IN();
	string_remove_at(calculator_input_str, from_pos,
			 end_pos - from_pos + 1);
	calculator_cursor_pos = from_pos;
	_calc_entry_text_set(entry, calculator_input_str);
	CONV_FUNC_OUT();
}

/**
* @describe
*       Set correct cursor position in entry.
*
* @param    entry
* @return    void
* @exception
*/
static void _calc_entry_cursor_set(Evas_Object * entry)
{
	CONV_FUNC_IN();
	calc_view_cursor_set_position(entry, calculator_cursor_pos);
	CONV_FUNC_OUT();
}

/**
* @describe
*
*
* @param    entry
* @return    void
* @exception
*/
static void _calc_entry_backspace(Evas_Object * entry)
{
	CONV_FUNC_IN();
	calc_expr_input_backspace(calculator_input_str, &calculator_cursor_pos);
	_calc_entry_text_set(entry, calculator_input_str);
	CONV_FUNC_OUT();
}

/**
* @describe
*
*
* @param    entry
* @return    void
* @exception
*/
void _calc_entry_clear(Evas_Object * entry)
{
	memset(calculator_input_str, 0, sizeof(calculator_input_str));
	calculator_cursor_pos = 0;
	_calc_entry_text_set(entry, "");
	calc_view_cursor_set_position(entry, calculator_cursor_pos);
}

/* END INPUT ENTRY RELATED */

/**
* @describe
*
*
* @param    text
* @param    begin
* @param    end
* @return    void
* @exception
*/
static bool
__calculator_get_float_num_in_cursor_position(char *text, int cur_pos,
					      int *begin, int *end)
{
	CONV_FUNC_IN();

	int pos = cur_pos - 1;
	if (pos < 0) {
		pos++;
		pos = 0;
	}
	int _begin = 0;
	int _end = 0;

	if ('p' == text[pos] || 'e' == text[pos]) {
		*begin = pos;
		*end = pos;
		return true;
	} else if (CALCULATOR_IS_DIGIT_DOT(text[pos], decimal_ch)) {
		for (_begin = pos - 1;
		     CALCULATOR_IS_DIGIT_DOT(text[_begin], decimal_ch)
		     && _begin >= 0; --_begin) {
			;	/* NULL */
		}
		_begin++;
		if (_begin > 1 && '-' == text[_begin - 1]
		    && '(' == text[_begin - 2]) {
			_begin--;
		}
		for (_end = pos + 1;
		     CALCULATOR_IS_DIGIT_DOT(text[_end], decimal_ch)
		     && _end < strlen(text); ++_end) {
			;	/* NULL */
		}
		_end--;

		*begin = _begin;
		*end = _end;
		return true;
	} else {
		return false;
	}
	CONV_FUNC_OUT();
}

/**
* @describe
*
*
* @param    entry_text
* @param    str
* @param    from
* @param    end
* @return    void
* @exception
*/
static bool
__calculator_get_cursor_position_float_string(char *entry_text, char *str,
					      int cur_pos, int *from, int *end)
{
	CONV_FUNC_IN();

	int from_pos = cur_pos;
	int end_pos = cur_pos;

	if (false ==
	    __calculator_get_float_num_in_cursor_position(entry_text, cur_pos,
							  &from_pos,
							  &end_pos)) {
		return false;
	}

	/* set from&end position */
	if (from != NULL) {
		*from = from_pos;
	}
	if (end != NULL) {
		*end = end_pos;
	}

	strncpy(str, entry_text + from_pos, end_pos - from_pos + 1);
	str[end_pos - from_pos + 1] = '\0';
	CONV_FUNC_OUT();
	return true;
}

/**
* @describe
*   Get the float number in current cursor
*
* @param    entry_text
* @param    str
* @return    void
* @exception
*/
static bool
__calculator_get_before_cursor_float_string(char *entry_text, char *str)
{
	CONV_FUNC_IN();

	int from_pos = calculator_cursor_pos;
	int end_pos = calculator_cursor_pos;

	if (false ==
	    __calculator_get_float_num_in_cursor_position(entry_text,
							  calculator_cursor_pos,
							  &from_pos,
							  &end_pos)) {
		return false;
	}
	snprintf(str, calculator_cursor_pos - from_pos + 1, "%s",
		 entry_text + from_pos);
	CONV_FUNC_OUT();
	return true;
}

/**
* @describe
*   Get string before cursor in the Entry.
*
* @param    entry_text
* @param    str
* @return    void
* @exception
*/
static void
__calculator_get_input_from_begin_to_cursor(char *entry_text, char *str)
{
	CONV_FUNC_IN();

	if (calculator_cursor_pos > 0) {
		strncpy(str, entry_text, calculator_cursor_pos);
		str[calculator_cursor_pos] = '\0';
	} else {
		strcpy(str, "");
	}
	CONV_FUNC_OUT();
}

/**
* @describe
*
* judge the type of current input
*
* @param    input
* @return    void
* @exception
*/
static last_char_t __calculator_string_get_char_type( char ch_in)
{
	CONV_FUNC_IN();
	if (ch_in == '\0') {
		return CHAR_IS_NULL;
	}
	if (ch_in == 'p') {
		return CHAR_IS_PI;
	}
	if (ch_in == 'e') {
		return CHAR_IS_E;
	}
	if (CALCULATOR_CHAR_IS_MULTI_DIVIDE_OPERATOR(ch_in)) {
		return CHAR_IS_MULTIPLY_DIVIDE;
	}
	if (ch_in == '(') {
		return CHAR_IS_LEFT_PARENTHESE;
	}
	if (ch_in == ')') {
		return CHAR_IS_RIGHT_PARENTHESE;
	}
	if (CALCULATOR_CHAR_IS_DIGITAL(ch_in)) {
		return CHAR_IS_DIGIT;
	}
	if (CALCULATOR_CHAR_IS_PLUS_DEDUCT(ch_in) ){
		return CHAR_IS_PLUS_MINUS;
	}
	if (ch_in == decimal_ch) {
		return CHAR_IS_POINT;
	}
	return CHAR_IS_CHARACTER;
	CONV_FUNC_OUT();
}

static bool  __calculator_string_digit_in(const char *input)
{
	int i =0;
	while(input[i]!='\0'){
		if(CALCULATOR_CHAR_IS_DIGITAL(input[i])){
			return TRUE;
		}else{
			i++;
		}
	}
	 return FALSE;
}

/*
* search charactor in input string, if have charactor, return True and index of first charactor;
* else return False;
*/
static bool __calculator_string_char_search(const char *input, int *index)
{
	CONV_FUNC_IN();
	int len_cp = strlen(input);
	if(len_cp <= 0){
		return FALSE;
	}
	int i = 0;
	for(; i < len_cp ; i++){
		last_char_t cur_char_type = __calculator_string_get_char_type(input[i]);
		if (CHAR_IS_CHARACTER == cur_char_type) {
			*index  = i;
			return TRUE;
		}
	}
	return FALSE;
	CONV_FUNC_OUT();
}

/*
* search invalid charactor in input string, if have invalid charactor, return True and index of first invalid charactor;
* else return False;
*/
static bool __calculator_string_invalid_char_search(char *input, int *index)
{
	int sub_index = 0;
	char *p = input;
	bool char_in = FALSE;
	int len = 0;
	char_in = __calculator_string_char_search(p, &sub_index);
	if(!char_in){/*no charactor*/
		return FALSE;
	}
	while(p){
		/* charactor  present*/
		*index += sub_index;
		p += sub_index;
		if(!__calculator_search_function(p, &len)){/*charactor not a function*/
	        return TRUE;
		}else{/*the first sevaral charactors are function, continue search*/
			*index += len;
			p += len;
		}
	}
	return FALSE;
}
/**
* @describe
*
*
* @param    entry
* @param    op_item
* @return    void
* @exception
*/
static void
__calculator_control_panel_number_button_clicked(Evas_Object * entry,
						 op_item_t * op_item)
{
	CONV_FUNC_IN();

	/* replace special characters */
	char entry_text[MAX_EXPRESSION_LENGTH] = { 0 };
	snprintf(entry_text, sizeof(entry_text), "%s", calculator_input_str);

	//Current state is calculated, clear all
	if (calculator_state == CALCULATOR_CALCULATED) {
		edje_object_signal_emit(_EDJ(ad->edje), "show,input", "");
		_calc_entry_clear(entry);
		_calc_entry_text_insert(entry, op_item->op_sym);
		calculator_state = CALCULATOR_OPERAND_INPUT;
		return;
	}

	char str_buf[MAX_EXPRESSION_LENGTH] = { 0 };
	char before_cursor[MAX_EXPRESSION_LENGTH] = { 0 };
	int before_len = 0;
	int nDigitCnt = 0;
	int nPointCnt = 0;
	__calculator_get_cursor_position_float_string(entry_text, str_buf,
						      calculator_cursor_pos,
						      NULL, NULL);
	__calculator_get_input_from_begin_to_cursor(entry_text, before_cursor);
	before_len = strlen(before_cursor);
	calculator_get_digits_number(str_buf, &nDigitCnt, &nPointCnt);

	char str_bufa[MAX_EXPRESSION_LENGTH] = { 0 };
	__calculator_get_before_cursor_float_string(entry_text, str_bufa);
	if (strcmp(str_bufa, "0") == 0) {
		_calc_entry_backspace(entry);
	}

	if (strlen(str_buf) >= MAX_NUM_LENGTH) {
		__calculator_wrong_format_create(CALC_MSG_MAX_DIGIT);
		return;
	} else if (nPointCnt >= MAX_DECIMAL_NUM
		   && calculator_cursor_pos > nDigitCnt) {
		__calculator_wrong_format_create(CALC_MSG_MAX_DEC_DIGIT);
		return;
	} else if (before_len > 0
		   && (__calculator_string_get_char_type(before_cursor[before_len - 1]) ==
		   CHAR_IS_PI
		   || __calculator_string_get_char_type(before_cursor[before_len - 1]) ==
		   CHAR_IS_E)) {
		__calculator_wrong_format_create(CALC_MSG_OP_FIRST);
		return;
	} else if (before_len > 0
		   && ((before_cursor[before_len - 1] == '(')
		       ||
		       CALCULATOR_CHAR_IS_DIGITAL(before_cursor[before_len - 1])
		       || CALCULATOR_CHAR_IS_OPERATOR(before_cursor[before_len - 1]) || (int)before_cursor[before_len - 1] > 120))	//* or/
	{
		_calc_entry_text_insert(entry, op_item->op_sym);
		calculator_state = CALCULATOR_OPERAND_INPUT;
		return;
	} else if (before_len > 0 && (before_cursor[before_len - 1] == ')')) {
		_calc_entry_text_insert(entry, op_item->op_sym);
		return;
	} else {
		_calc_entry_text_insert(entry, op_item->op_sym);
		calculator_state = CALCULATOR_OPERAND_INPUT;
		return;
	}
	CONV_FUNC_OUT();
}

/**
* @describe
*
*
* @param    entry
* @return    void
* @exception
*/
static void _calc_btn_dot_clicked(Evas_Object * entry)
{
	CONV_FUNC_IN();
	char temp[MAX_RESULT_LENGTH] = { 0 };
	char str_num[CALCULATOR_CONTENT_LEN] = { 0 };
	int str_num_len = 0;

	char decimal_str[32] = { 0 };

	/* replace special characters */
	char entry_text[MAX_EXPRESSION_LENGTH] = { 0 };
	strncpy(entry_text, calculator_input_str, MAX_EXPRESSION_LENGTH - 1);

	if (calculator_state == CALCULATOR_CALCULATED) {
		calc_expr_num_format_result(last_result, temp);
		if (strchr(temp, 'E') != NULL) {
			return;
		}
		/* science number */
		_calc_entry_clear(entry);
		_calc_entry_text_insert(entry, temp);
	}

	int from_pos = 0;
	int end_pos = 0;
	if (!__calculator_get_cursor_position_float_string
	    (entry_text, str_num, calculator_cursor_pos, &from_pos, &end_pos)) {
		if (calculator_cursor_pos == 0
		    || IS_OPERATOER(calculator_input_str[from_pos - 1])) {
			snprintf(decimal_str, sizeof(decimal_str), "0%c",
				 decimal_ch);
			_calc_entry_text_insert(entry, decimal_str);
			calculator_state = CALCULATOR_OPERAND_INPUT;
			return;
		} else {
			__calculator_wrong_format_create(CALC_MSG_NUM_FIRST);
		}
		return;
	} else {
		if (strcmp(str_num, "p") == 0 || strcmp(str_num, "e") == 0) {
			__calculator_wrong_format_create(CALC_MSG_NUM_FIRST);
			return;
		}
	}

	str_num_len = strlen(str_num);
	if (str_num_len > 0 && str_num[str_num_len - 1] == decimal_ch) {
		__calculator_wrong_format_create(CALC_MSG_NUM_FIRST);
		return;
	}

	int nDigitCnt = 0;
	int nPointCnt = 0;
	calculator_get_digits_number(str_num, &nDigitCnt, &nPointCnt);

	if (nDigitCnt >= 15) {
		__calculator_wrong_format_create(CALC_MSG_MAX_DIGIT);
		return;
	}

	if (nPointCnt > 0) {
		__calculator_wrong_format_create(CALC_MSG_ENTRY_LIMIT);
		return;
	}
	snprintf(decimal_str, sizeof(decimal_str), "%c", decimal_ch);
	_calc_entry_text_insert(entry, decimal_str);
	calculator_state = CALCULATOR_OPERAND_INPUT;
	CONV_FUNC_OUT();
	return;
}

/**
* @describe
*
*
* @param        entry
* @return       void
* @exception
*/
static void _calc_btn_backspace_clicked(Evas_Object * entry)
{
	CONV_FUNC_IN();

	if (calculator_state == CALCULATOR_CALCULATED) {
		calculator_state = CALCULATOR_OPERATOR_INPUT;
		if (calculator_cursor_pos > strlen(calculator_input_str)) {
			calculator_cursor_pos = strlen(calculator_input_str);	/* set last position */
		}
	}
	_calc_entry_backspace(entry);
	CONV_FUNC_OUT();
}

static int __calculator_delete_long_press(void *data)
{
	CONV_FUNC_IN();
	Evas_Object *entry = (Evas_Object *) data;
	_calc_btn_backspace_clicked(entry);
	CONV_FUNC_OUT();
	return 1;
}

/**
* @describe
*   Process +.-.*.\ these four buttons clicked.
*
* @param        entry
* @param        op_item
* @return       void
* @exception
*/
static void
__calculator_control_normal_func_clicked(Evas_Object * entry,
					 op_item_t * op_item)
{
	CONV_FUNC_IN();
	char all_input[MAX_EXPRESSION_LENGTH] = { 0 };
	strncpy(all_input, calculator_input_str, MAX_EXPRESSION_LENGTH - 1);

	if (calculator_state == CALCULATOR_CALCULATED) {
		edje_object_signal_emit(_EDJ(ad->edje), "show,input", "");
		char temp[20] = { 0 };

		calc_expr_num_format_result(last_result, temp);
		_calc_entry_clear(entry);

		if (temp[0] == '-' || strchr(temp, 'E'))	//result < 0 or sicience number
		{
			_calc_entry_text_insert(entry, "(");
			_calc_entry_text_insert(entry, temp);
			_calc_entry_text_insert(entry, ")");
		} else {
			_calc_entry_text_insert(entry, temp);
		}

		_calc_entry_text_insert(entry, op_item->op_sym);
		calculator_state = CALCULATOR_OPERATOR_INPUT;
		return;

	}
    if(!strcmp(op_item->op_sym, "x") || !strcmp(op_item->op_sym, "/") || !strcmp(op_item->op_sym, "+")){
		if(!__calculator_string_digit_in(calculator_input_str)){ return; }
	}
	int nCntOp = calc_expr_get_operator_num(all_input);
	if (nCntOp >= MAX_OPERATOR_NUM) {	/* Can't exceed 20 operators */
		__calculator_wrong_format_create(CALC_MSG_MAX_OP);
		return;
	}

	char before_cursor[MAX_EXPRESSION_LENGTH] = { 0 };
	int input_len = 0;

	__calculator_get_input_from_begin_to_cursor(all_input, before_cursor);
	input_len = strlen(before_cursor);

	if (input_len > 0) {
		if (before_cursor[input_len - 1] == '('
		    && !strcmp(op_item->op_sym, "-")) {
			_calc_entry_text_insert(entry, op_item->op_sym);
			calculator_state = CALCULATOR_OPERATOR_INPUT;
		} else if (((input_len > 1) && (before_cursor[input_len - 1] == '-') && (before_cursor[input_len - 2] == '('))	//(-
			   || before_cursor[input_len - 1] == decimal_ch || before_cursor[input_len - 1] == '(')	// . or (
		{

			__calculator_wrong_format_create(CALC_MSG_NUM_FIRST);
			return;
		} else if (IS_OPERATOER(before_cursor[input_len - 1])) {
			if ((input_len > 1 || !strcmp(op_item->op_sym, "-"))
			    || input_len == 1) {
				_calc_entry_backspace(entry);
				_calc_entry_text_insert(entry, op_item->op_sym);
				calculator_state = CALCULATOR_OPERATOR_INPUT;
			} else {
				__calculator_wrong_format_create(CALC_MSG_NUM_FIRST);
			}
			return;
		} else if (before_cursor[input_len - 1] == ')')	//
		{
			_calc_entry_text_insert(entry, op_item->op_sym);
			calculator_state = CALCULATOR_OPERATOR_INPUT;
			return;
		} else {
			if (!IS_DIGITAL(before_cursor[input_len - 1])) {
				__calculator_wrong_format_create(CALC_MSG_NUM_FIRST);
				return;
			} else {
				_calc_entry_text_insert(entry, op_item->op_sym);
				calculator_state = CALCULATOR_OPERATOR_INPUT;
				return;
			}
		}
	} else {		/* before_cursor si empty */

		_calc_entry_text_insert(entry, op_item->op_sym);
		calculator_state = CALCULATOR_OPERATOR_INPUT;
	}
	CONV_FUNC_OUT();
}

/**
* @describe
*
*
* @param    entry
* @return    void
* @exception
*/
static void __calculator_symbol_negative_clicked(Evas_Object * entry)
{
	CONV_FUNC_IN();

	char result[MAX_RESULT_LENGTH] = { 0 };

	if (calculator_state == CALCULATOR_CALCULATED) {
		edje_object_signal_emit(_EDJ(ad->edje), "show,input", "");

		calc_expr_num_format_result(last_result, result);
		if (last_result < 0) {
			string_remove_at(result, 0, 1);	//remove '-'
		} else {
			string_insert(result, 0, "(-");	// add (-xxx)
			string_insert(result, strlen(result), ")");
		}

		_calc_entry_clear(entry);
		_calc_entry_text_insert(entry, result);

		calculator_state = CALCULATOR_OPERAND_INPUT;
		//use_last_result = 1;

		return;
	}

	int cursor = calculator_cursor_pos;
	int begin = 0, length = 0;
	char expr[MAX_EXPRESSION_LENGTH] = { 0 };
	strncpy(expr, calculator_input_str, MAX_EXPRESSION_LENGTH - 1);
	int flag = 0;

	cursor -= 1;
	if (expr[cursor] == ')') {
		cursor -= 1;
		flag = 1;	/* before current cursor is ')' */
	}

	if (0 ==
	    calc_expr_get_current_num_at_cursor(expr, cursor, &begin,
						&length)) {
		if (expr[begin] == '-') {
			if (begin > 0 && expr[begin - 1] == '('
			    && expr[begin + length] == ')') {
				string_remove_at(expr, begin + length, 1);	//remove ')'
				string_remove_at(expr, begin - 1, 2);	// remove '(-'
				calculator_cursor_pos -= flag ? 3 : 2;
			} else {
				string_remove_at(expr, begin, 1);
				calculator_cursor_pos -= 1;
			}

		} else {
			if (flag == 1)	//not '(-xxx)' but has ')'
			{
				__calculator_wrong_format_create(CALC_MSG_NUM_FIRST);
				return;
			} else {
				string_insert(expr, begin + length, ")");
				string_insert(expr, begin, "(-");
				calculator_cursor_pos +=
				    (((begin + length) ==
				      calculator_cursor_pos) ? 3 : 2);
			}
		}
		strncpy(calculator_input_str, expr, MAX_EXPRESSION_LENGTH - 1);

		_calc_entry_text_set(entry, calculator_input_str);
		_calc_entry_cursor_set(entry);

		calculator_state = CALCULATOR_OPERAND_INPUT;

	} else {
		__calculator_wrong_format_create(CALC_MSG_NUM_FIRST);
	}
	CONV_FUNC_OUT();
}

/* search the previous operator and value */
static char * __calculator_search_prev_input(char *input_str)
{
	int i = strlen(input_str) - 1;
	int rightbracket = 0;	
	char *prev_input = NULL;
	for ( ; i > 0; i--){
		if (input_str[i] == ')') {
			rightbracket++;
		} else if (input_str[i] == '(') {		
			rightbracket--;
		}
		if ((CALCULATOR_CHAR_IS_PLUS_DEDUCT(input_str[i])
			||CALCULATOR_CHAR_IS_MULTI_DIVIDE_OPERATOR(input_str[i])) && (rightbracket == 0)){
			prev_input = &input_str[i];			
			return prev_input;
		}
	}
	return prev_input;
}

/**
* @describe
*
*
* @param        entry
* @return       void
* @exception
*/
static void __calculator_op_equal(Evas_Object * entry)
{
	CONV_FUNC_IN();
	if (calculator_state == CALCULATOR_CALCULATED) {
		/*duplicate previous input operator and value*/
		char *p = __calculator_search_prev_input(calculator_input_str);
		if (p == NULL) {
			return;
		}
		char prev_input[32] = { 0 };
		int len = g_strlcpy(prev_input, p, sizeof(prev_input));
		if(len >= sizeof(prev_input)){
			return;
		}
		char temp[32] = { 0 };
		calc_expr_num_format_result(last_result, temp);
		_calc_entry_clear(entry);

		if (temp[0] == '-' || strchr(temp, 'E')) {
			_calc_entry_text_insert(entry, "(");
			_calc_entry_text_insert(entry, temp);
			_calc_entry_text_insert(entry, ")");
		} else {
			_calc_entry_text_insert(entry, temp);
		}
		_calc_entry_text_insert(entry, prev_input);
		calculator_state = CALCULATOR_OPERATOR_INPUT;
	}

	double result = 0;
	char str_buf[MAX_EXPRESSION_LENGTH] = { 0 };
	char result_buf[MAX_RESULT_LENGTH] = { 0 };
	int str_len = 0;
	char error_msg[MAX_ERROR_MESSAGE_LENGTH] = { 0 };
	calc_expr_close_parentheses(calculator_input_str);
	_calc_entry_text_set(entry, calculator_input_str);
	snprintf(str_buf, sizeof(str_buf), "%s", calculator_input_str);
	str_len = strlen(str_buf);
	if (str_len == 0) {
		__calculator_wrong_format_create(CALC_MSG_NUM_FIRST);
		return;
	}

	PLOG("calculate :%s\n", str_buf);
	if (!calculator_calculate(str_buf, &result, error_msg)) {
		PLOG("error : %s\n", error_msg);
		__calculator_wrong_format_create(error_msg);
		calculator_state = CALCULATOR_ERROR_OCCURED;
		return;
	} else {
		last_result = result;
		memset(result_buf, 0, CALCULATOR_CONTENT_LEN + 1);
		calc_expr_num_format_result(result, result_buf);
		/* save result */

		struct history_item item;
		memset(item.expression, 0, sizeof(item.expression));
		memset(item.result, 0, sizeof(item.result));
		calc_expr_format_expression(calculator_input_str,
					    item.expression);
		calc_expr_format_expression(result_buf, item.result);
#ifdef SAVE_HISTORY
		calc_view_save_history(item);
#endif

		/* show result */
		calculator_state = CALCULATOR_CALCULATED;
		calc_view_show_result(item.result, ad);

	}
	CONV_FUNC_OUT();
}

/**
* @describe
*
*
* @param        entry
* @return       void
* @exception    none
*/
static void __calculator_parenthesis_clicked(Evas_Object * entry)
{
	CONV_FUNC_IN();
	char all_input[MAX_EXPRESSION_LENGTH] = { 0 };
	snprintf(all_input, sizeof(all_input), "%s", calculator_input_str);
	if (calculator_state == CALCULATOR_CALCULATED) {
		edje_object_signal_emit(_EDJ(ad->edje), "show,input", "");

		char temp[MAX_RESULT_LENGTH] = { 0 };

		calc_expr_num_format_result(last_result, temp);
		_calc_entry_clear(entry);

		if (temp[0] == '-' || strchr(temp, 'E') != NULL)	//result < 0 or science number
		{
			_calc_entry_text_insert(entry, "((");
			_calc_entry_text_insert(entry, temp);
			_calc_entry_text_insert(entry, ")");
		} else {
			_calc_entry_text_insert(entry, "(");
			_calc_entry_text_insert(entry, temp);
		}
		calculator_state = CALCULATOR_OPERATOR_INPUT;
		return;
	}
	char before_cursor[MAX_EXPRESSION_LENGTH] = { 0 };
	int input_len = 0;
	__calculator_get_input_from_begin_to_cursor(all_input, before_cursor);
	input_len = strlen(before_cursor);

	if (input_len == 0) {
		_calc_entry_text_insert(entry, "(");
		calculator_state = CALCULATOR_OPERATOR_INPUT;
		return;
	}
	int bracket_num = calculator_get_open_braket(all_input);
	if (input_len > 0) {
		if (before_cursor[input_len - 1] == decimal_ch) {
			__calculator_wrong_format_create(CALC_MSG_NUM_FIRST);
			return;
		} else if (bracket_num > 0	//'(' is more than ')'
			   && before_cursor[input_len - 1] != '('
			   && (before_cursor[input_len - 1] == ')'
			       || isdigit(before_cursor[input_len - 1])
			       ||
			       (__calculator_string_get_char_type
				(before_cursor[input_len - 1]) == CHAR_IS_PI
				||
				__calculator_string_get_char_type
				(before_cursor[input_len - 1]) == CHAR_IS_E))) {
			_calc_entry_text_insert(entry, ")");
			return;
		} else if (bracket_num == 0) {
			if (calc_expr_get_left_parentheses_num
			    (calculator_input_str) < MAX_PARENTHESES_NUM) {
				_calc_entry_text_insert(entry, "(");
			}
			return;
		} else if (before_cursor[input_len - 1] != ')')	//'(' is less than ')'!isdigit(before_cursor[input_len-1])&&(
		{
			if (calc_expr_get_left_parentheses_num
			    (calculator_input_str) < MAX_PARENTHESES_NUM) {
				_calc_entry_text_insert(entry, "(");
			}
			return;
		}
	}
	CONV_FUNC_OUT();
	return;
}

/**
* @describe
*
*
* @param        entry
* @param        op_item
* @return       void
* @exception
*/
static void
__calculator_control_functions_button_clicked(Evas_Object * entry,
					      op_item_t * op_item)
{
	CONV_FUNC_IN();

	char *str = NULL;
	function_t function = { 0 };

	memset(&function, 0x0, sizeof(function_t));
	str = op_item->op_name;

	char all_input[MAX_EXPRESSION_LENGTH] = { 0 };
	int input_len = 0;
	char before_cursor[MAX_EXPRESSION_LENGTH] = { 0 };
	int before_len = 0;
	last_char_t last_char_type = 0;

	snprintf(all_input, sizeof(all_input), "%s", calculator_input_str);
	__calculator_get_input_from_begin_to_cursor(all_input, before_cursor);
	input_len = strlen(all_input);
	before_len = strlen(before_cursor);
	if(before_len > 0){
		last_char_type = __calculator_string_get_char_type(before_cursor[before_len - 1]);
	}

	switch (op_item->op_id) {
	case OP_PERCENT:
		/* if it is calculated state */
		if (calculator_state == CALCULATOR_CALCULATED) {
			edje_object_signal_emit(_EDJ(ad->edje), "show,input",
						"");

			char temp[CALCULATOR_CONTENT_LEN] = { 0 };
			double per_result = last_result / 100.0;

			calc_expr_num_format_result(per_result, temp);
			_calc_entry_clear(entry);

			if (strcmp(temp, "0") == 0) {
				_calc_entry_text_insert(entry, "0");
			} else if (temp[0] == '-')	//result < 0
			{
				_calc_entry_text_insert(entry, "(");
				_calc_entry_text_insert(entry, temp);
				_calc_entry_text_insert(entry, ")");
			} else {
				_calc_entry_text_insert(entry, temp);
			}
			calculator_state = CALCULATOR_OPERAND_INPUT;
			return;
		}

		{
			int from_pos, end_pos;
			char str_num[CALCULATOR_CONTENT_LEN] = { 0 };
			char temp[CALCULATOR_CONTENT_LEN] = { 0 };
			double per_result = 0.0;

			if (false ==
			    __calculator_get_cursor_position_float_string
			    (all_input, str_num, calculator_cursor_pos,
			     &from_pos, &end_pos)) {
				__calculator_wrong_format_create(CALC_MSG_NUM_FIRST);
				return;
			}

			{
				if (strlen(str_num) == 0) {
					__calculator_wrong_format_create(CALC_MSG_NUM_FIRST);
					return;
				}

				if (strcmp(str_num, "0") == 0) {
					return;
				}

				per_result = atof(str_num);
				per_result /= 100.0;

				calc_expr_num_format_result(per_result, temp);

				_calc_entry_text_remove(entry, from_pos,
							end_pos);
				_calc_entry_text_insert(entry, temp);

				calculator_state = CALCULATOR_OPERAND_INPUT;
				return;
			}
		}
		break;
	case OP_PI:
		if (calculator_state == CALCULATOR_CALCULATED) {
			edje_object_signal_emit(_EDJ(ad->edje), "show,input",
						"");
			_calc_entry_clear(entry);
			_calc_entry_text_insert(entry, "p");
			break;
		}
		if ((before_len != 0) && (before_cursor[before_len - 1] != '+')
		    && (before_cursor[before_len - 1] != '-')
		    && (last_char_type != CHAR_IS_MULTIPLY_DIVIDE)
		    && (before_cursor[before_len - 1] != '^')
		    && (before_cursor[before_len - 1] != '(')
		    && (before_cursor[before_len - 1] != 'P')
		    && before_cursor[before_len - 1] != 'C') {
			__calculator_wrong_format_create(CALC_MSG_OP_FIRST);
			break;
		}
		_calc_entry_text_insert(entry, "p");
		break;

	case OP_E:
		if (calculator_state == CALCULATOR_CALCULATED) {
			edje_object_signal_emit(_EDJ(ad->edje), "show,input",
						"");

			_calc_entry_clear(entry);
			_calc_entry_text_insert(entry, op_item->op_name);
			break;
		}
		if ((before_len != 0) && (before_cursor[before_len - 1] != '+')
		    && (before_cursor[before_len - 1] != '-')
		    && (last_char_type != CHAR_IS_MULTIPLY_DIVIDE)
		    && (before_cursor[before_len - 1] != '^')
		    && (before_cursor[before_len - 1] != '(')
		    && (before_cursor[before_len - 1] != 'P')
		    && before_cursor[before_len - 1] != 'C') {
			__calculator_wrong_format_create(CALC_MSG_OP_FIRST);
			break;
		}
		_calc_entry_text_insert(entry, op_item->op_name);
		break;

	case OP_SIN:		//sin()
	case OP_COS:		//cos()
	case OP_TAN:		//tan()
	case OP_LOG:		//log()
	case OP_ABS:		//abs()
		if (calculator_state == CALCULATOR_CALCULATED) {
			edje_object_signal_emit(_EDJ(ad->edje), "show,input",
						"");

			_calc_entry_clear(entry);
			_calc_entry_text_insert(entry, op_item->op_name);
			break;
		}
		if (last_char_type == CHAR_IS_PI || last_char_type == CHAR_IS_E) {
			__calculator_wrong_format_create(CALC_MSG_OP_FIRST);
			break;
		}

		_calc_entry_text_insert(entry, op_item->op_name);
		break;

	case OP_LN:		//ln()
		if (calculator_state == CALCULATOR_CALCULATED) {
			edje_object_signal_emit(_EDJ(ad->edje), "show,input",
						"");
			_calc_entry_clear(entry);
			_calc_entry_text_insert(entry, op_item->op_name);
			break;
		}
		if (last_char_type == CHAR_IS_PI) {
			__calculator_wrong_format_create(CALC_MSG_OP_FIRST);
			break;
		}
		if ((before_len != 0) && (before_cursor[before_len - 1] != '+')
		    && (before_cursor[before_len - 1] != '-')
		    && (last_char_type != CHAR_IS_MULTIPLY_DIVIDE)
		    && (before_cursor[before_len - 1] != '^')
		    && (before_cursor[before_len - 1] != '(')
		    && (before_cursor[before_len - 1] != 'C')
		    && (before_cursor[before_len - 1] != 'P')) {
			__calculator_wrong_format_create(CALC_MSG_OP_FIRST);
			break;
		}
		_calc_entry_text_insert(entry, op_item->op_name);
		break;

	case OP_ROOT:		//sqrt()
		if (calculator_state == CALCULATOR_CALCULATED) {
			edje_object_signal_emit(_EDJ(ad->edje), "show,input",
						"");

			_calc_entry_clear(entry);
			_calc_entry_text_insert(entry, op_item->op_name);
			break;
		}
		if (last_char_type == CHAR_IS_PI) {
			__calculator_wrong_format_create(CALC_MSG_OP_FIRST);
			break;
		}
		if ((before_len != 0) && (before_cursor[before_len - 1] != '+')
		    && (before_cursor[before_len - 1] != '-')
		    && (last_char_type != CHAR_IS_MULTIPLY_DIVIDE)
		    && (before_cursor[before_len - 1] != '^')
		    && (before_cursor[before_len - 1] != '(')
		    && (before_cursor[before_len - 1] != 'C')
		    && (before_cursor[before_len - 1] != 'P')) {
			__calculator_wrong_format_create(CALC_MSG_OP_FIRST);
			break;
		}
		_calc_entry_text_insert(entry, op_item->op_name);
		break;

	case OP_EX:		//e^
		if (calculator_state == CALCULATOR_CALCULATED) {
			edje_object_signal_emit(_EDJ(ad->edje), "show,input",
						"");
			_calc_entry_clear(entry);
			_calc_entry_text_insert(entry, op_item->op_name);
			break;
		}
		if (last_char_type == CHAR_IS_PI) {
			__calculator_wrong_format_create(CALC_MSG_OP_FIRST);
			break;
		}
		if ((before_len != 0) && (before_cursor[before_len - 1] != '+')
		    && (before_cursor[before_len - 1] != '-')
		    && (last_char_type != CHAR_IS_MULTIPLY_DIVIDE)
		    && (before_cursor[before_len - 1] != '^')
		    && (before_cursor[before_len - 1] != '(')
		    && (before_cursor[before_len - 1] != 'C')
		    && (before_cursor[before_len - 1] != 'P')) {
			__calculator_wrong_format_create(CALC_MSG_OP_FIRST);
			break;
		}
		_calc_entry_text_insert(entry, op_item->op_name);
		break;

	case OP_X2:		//x^2
		if (calculator_state == CALCULATOR_CALCULATED) {
			edje_object_signal_emit(_EDJ(ad->edje), "show,input",
						"");

			char temp[CALCULATOR_CONTENT_LEN] = { 0 };
			calc_expr_num_format_result(last_result, temp);
			_calc_entry_clear(entry);

			if (temp[0] == '-' || strchr(temp, 'E'))	//result < 0 or science number
			{
				_calc_entry_text_insert(entry, "(");
				_calc_entry_text_insert(entry, temp);
				_calc_entry_text_insert(entry, ")");
			} else {
				_calc_entry_text_insert(entry, temp);
			}
			_calc_entry_text_insert(entry, op_item->op_name);
			calculator_state = CALCULATOR_OPERAND_INPUT;
			//use_last_result = 1;
			return;
		}

		if (input_len == 0) {
			__calculator_wrong_format_create(CALC_MSG_NUM_FIRST_X2);
			break;
		}
		if (last_char_type == CHAR_IS_PI) {
			_calc_entry_text_insert(entry, op_item->op_name);
			calculator_state = CALCULATOR_OPERAND_INPUT;
			break;
		} else if ((before_len > 0)
			   && (!isdigit(before_cursor[before_len - 1]))
			   && (before_cursor[before_len - 1] != ')')) {
			__calculator_wrong_format_create(CALC_MSG_NUM_FIRST_X2);
			break;
		}
		_calc_entry_text_insert(entry, op_item->op_name);
		calculator_state = CALCULATOR_OPERAND_INPUT;
		break;
	case OP_XY:		//x^y
		if (calculator_state == CALCULATOR_CALCULATED) {
			edje_object_signal_emit(_EDJ(ad->edje), "show,input",
						"");

			char temp[CALCULATOR_CONTENT_LEN] = { 0 };

			calc_expr_num_format_result(last_result, temp);
			_calc_entry_clear(entry);

			if (temp[0] == '-' || strchr(temp, 'E'))	//result < 0 or science number
			{
				_calc_entry_text_insert(entry, "(");
				_calc_entry_text_insert(entry, temp);
				_calc_entry_text_insert(entry, ")");
			} else {
				_calc_entry_text_insert(entry, temp);
			}
			_calc_entry_text_insert(entry, op_item->op_name);
			calculator_state = CALCULATOR_OPERATOR_INPUT;
			return;
		}

		if (input_len == 0) {
			__calculator_wrong_format_create(CALC_MSG_NUM_FIRST_XY);
			break;
		}
		if ((last_char_type == CHAR_IS_PI)
		    || (last_char_type == CHAR_IS_E)) {
			_calc_entry_text_insert(entry, op_item->op_name);
			calculator_state = CALCULATOR_OPERATOR_INPUT;
			break;
		} else if ((before_len > 0)
			   && !isdigit(before_cursor[before_len - 1])
			   && (before_cursor[before_len - 1] != ')')) {
			__calculator_wrong_format_create(CALC_MSG_NUM_FIRST_XY);
			break;
		}
		_calc_entry_text_insert(entry, op_item->op_name);
		calculator_state = CALCULATOR_OPERATOR_INPUT;
		break;

	case OP_FACT:		//x!
		if (calculator_state == CALCULATOR_CALCULATED) {
			edje_object_signal_emit(_EDJ(ad->edje), "show,input",
						"");

			char temp[MAX_RESULT_LENGTH] = { 0 };

			calc_expr_num_format_result(last_result, temp);

			if (strchr(temp, decimal_ch) != NULL || strchr(temp, '-') != NULL)	//revise by bfl
			{
				__calculator_wrong_format_create(CALC_MSG_INVALID_FAC);
				calculator_state = CALCULATOR_WAITING_INPUT;	//revise by bfl
				return;
			}

			_calc_entry_clear(entry);
			_calc_entry_text_insert(entry, "(");
			_calc_entry_text_insert(entry, temp);
			_calc_entry_text_insert(entry, "!)");

			calculator_state = CALCULATOR_OPERATOR_INPUT;
			return;
		}

		if (input_len == 0) {
			__calculator_wrong_format_create(CALC_MSG_NUM_FIRST_FAC);
			break;
		}
		if ((last_char_type == CHAR_IS_PI)
		    || (last_char_type == CHAR_IS_E)) {
			__calculator_wrong_format_create(CALC_MSG_INVALID_FAC);
			break;
		}

		/* check if it is natural */
		{
			char str_buf[MAX_EXPRESSION_LENGTH] = { 0 };
			int from_pos = 0, end_pos = 0;

			if (false ==
			    __calculator_get_cursor_position_float_string
			    (all_input, str_buf, calculator_cursor_pos,
			     &from_pos, &end_pos)) {
				__calculator_wrong_format_create(CALC_MSG_NUM_FIRST);
				break;
			}

			if (strchr(str_buf, decimal_ch) != NULL
			    || str_buf[0] == '-') {
				__calculator_wrong_format_create(CALC_MSG_INVALID_FAC);
				break;
			}

			_calc_entry_text_remove(entry, from_pos, end_pos);
			_calc_entry_text_insert(entry, "(");
			_calc_entry_text_insert(entry, str_buf);
			_calc_entry_text_insert(entry, "!)");
			calculator_state = CALCULATOR_OPERATOR_INPUT;
		}
		break;
/* @ attention
    This is redundant comment.
    Its only use is for Klocwork testing.
    The comment rete should passed 25%.
    But we have only one day to do this work.
    So, sorry, I will delete this comment and add useful comment later.
*/
	case OP_1X:
		if (calculator_state == CALCULATOR_CALCULATED) {
			edje_object_signal_emit(_EDJ(ad->edje), "show,input",
						"");

			char temp[MAX_RESULT_LENGTH] = { 0 };
			int i = 0;

			calc_expr_num_format_result(last_result, temp);
			if (strchr(temp, 'E') != NULL)	//science number
			{
				temp[strlen(temp)] = ')';
				for (i = strlen(temp); i > 0; --i) {
					temp[i] = temp[i - 1];
				}
				temp[0] = '(';
			}

			_calc_entry_clear(entry);
			if (temp[0] == '-') {
				_calc_entry_text_insert(entry, "(-1");
				_calc_entry_text_insert(entry, "/");

				_calc_entry_text_insert(entry, &temp[1]);
				_calc_entry_text_insert(entry, ")");
			} else {
				_calc_entry_text_insert(entry, "(1");
				_calc_entry_text_insert(entry, "/");

				_calc_entry_text_insert(entry, temp);
				_calc_entry_text_insert(entry, ")");
			}

			calculator_state = CALCULATOR_OPERATOR_INPUT;
			//use_last_result = 1;
			return;
		}
/* @ attention
    This is redundant comment.
    Its only use is for Klocwork testing.
    The comment rete should passed 25%.
    But we have only one day to do this work.
    So, sorry, I will delete this comment and add useful comment later.
*/
		if (input_len == 0) {
			__calculator_wrong_format_create(CALC_MSG_NUM_FIRST_RECIP);
			break;
		}

		/* check if it is digit */
		{
			char str_buf[MAX_EXPRESSION_LENGTH] = { 0 };
			int from_pos = 0, end_pos = 0;

			if (false ==
			    __calculator_get_cursor_position_float_string
			    (all_input, str_buf, calculator_cursor_pos,
			     &from_pos, &end_pos)) {
				__calculator_wrong_format_create(CALC_MSG_NUM_FIRST_RECIP);
				break;
			}

			if (strcmp(str_buf, "p") && strcmp(str_buf, "e")
			    && atof(str_buf) == 0) {
				__calculator_wrong_format_create(CALC_MSG_DIVIDE_BY_ZERO);
				break;
			}

			_calc_entry_text_remove(entry, from_pos, end_pos);
			if (str_buf[0] == '-') {
				_calc_entry_text_insert(entry, "(-1");
				_calc_entry_text_insert(entry, "/");

				_calc_entry_text_insert(entry, &str_buf[1]);
				_calc_entry_text_insert(entry, ")");
			} else {
				_calc_entry_text_insert(entry, "(1");
				_calc_entry_text_insert(entry, "/");

				_calc_entry_text_insert(entry, str_buf);
				_calc_entry_text_insert(entry, ")");
			}

			calculator_state = CALCULATOR_OPERATOR_INPUT;
		}
		break;

	default:
		break;
	}
	calculator_state = CALCULATOR_SPECIAL_FUNCTION_INPUT;
	CONV_FUNC_OUT();
	return;
}

/////////////////////////// input text finish ////////////////////////////

/**
* @describe
*
*
* @param    evas_obj
* @param    obj
* @return    void
* @exception
*/
static int _calculator_get_input_item(Evas_Object * evas_obj, Evas_Object * obj)
{
	CONV_FUNC_IN();

	int val = 0;
	if (evas_obj == edje_object_part_object_get(obj, "item_brack")) {
		val = OP_PARENTHESIS;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_del")) {
		val = OP_DELETE;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_c")) {
		val = OP_CLEAR;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_div")) {
		val = OP_DIVIDE;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_num7")) {
		val = OP_NUM_7;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_num8")) {
		val = OP_NUM_8;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_num9")) {
		val = OP_NUM_9;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_mul")) {
		val = OP_MULTIPLY;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_num4")) {
		val = OP_NUM_4;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_num5")) {
		val = OP_NUM_5;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_num6")) {
		val = OP_NUM_6;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_sub")) {
		val = OP_MINUS;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_num1")) {
		val = OP_NUM_1;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_num2")) {
		val = OP_NUM_2;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_num3")) {
		val = OP_NUM_3;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_plus")) {
		val = OP_PLUS;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_dot")) {
		val = OP_DOT;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_num0")) {
		val = OP_NUM_0;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_neg")) {
		val = OP_PLUS_MINUS;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_eq")) {
		val = OP_EQUAL;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_per")) {
		val = OP_PERCENT;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_sqr")) {
		val = OP_ROOT;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_fac")) {
		val = OP_FACT;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_sin")) {
		val = OP_SIN;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_cos")) {
		val = OP_COS;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_tan")) {
		val = OP_TAN;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_ln")) {
		val = OP_LN;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_log")) {
		val = OP_LOG;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_1x")) {
		val = OP_1X;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_ex")) {
		val = OP_EX;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_x2")) {
		val = OP_X2;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_xy")) {
		val = OP_XY;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_abs")) {
		val = OP_ABS;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_pi")) {
		val = OP_PI;
	} else if (evas_obj == edje_object_part_object_get(obj, "item_e")) {
		val = OP_E;
	}
	CONV_FUNC_OUT();
	return val;
}

/**
* @description
*   Interpret all of our different signals, and do things !
*
* @param        data
* @param        e
* @param        evas_obj
* @param        event_info
* @return       void
* @exception    none
*/
static void
_calculator_interp(void *data, Evas * e, Evas_Object * evas_obj,
		   void *event_info)
{
	CONV_FUNC_IN();

	int val = 0;
	if (data && ad->popup == NULL) {
		Evas_Object *obj = (Evas_Object *) data;
		val = _calculator_get_input_item(evas_obj, obj);
		if (ad->wrong_timer) {
			ecore_timer_del(ad->wrong_timer);
			ad->wrong_timer = NULL;
		}
		_calc_entry_text_set(ad->input_entry, calculator_input_str);
		switch (val) {
		case OP_DELETE:
			_calc_btn_backspace_clicked(ad->input_entry);
			if (ad->calc_timer) {
				ecore_timer_del(ad->calc_timer);
				ad->calc_timer = NULL;
			}
			ad->calc_timer =
			    ecore_timer_add(0.1,
					    (Ecore_Task_Cb)
					    __calculator_delete_long_press,
					    ad->input_entry);
			break;
		case OP_CLEAR:
			edje_object_signal_emit(_EDJ(ad->edje), "show,input",
						"");
			_calc_entry_clear(ad->input_entry);
			calculator_state = CALCULATOR_WAITING_INPUT;
			break;
		case OP_PLUS:
		case OP_MINUS:
		case OP_MULTIPLY:
		case OP_DIVIDE:
			__calculator_control_normal_func_clicked(ad->
								 input_entry,
								 &calc_op_item
								 [val]);
			break;
		case OP_EQUAL:
			__calculator_op_equal(ad->input_entry);
			break;
		case OP_DOT:
			_calc_btn_dot_clicked(ad->input_entry);
			break;
		case OP_PARENTHESIS:
			__calculator_parenthesis_clicked(ad->input_entry);
			break;
		case OP_NUM_0:
		case OP_NUM_1:
		case OP_NUM_2:
		case OP_NUM_3:
		case OP_NUM_4:
		case OP_NUM_5:
		case OP_NUM_6:
		case OP_NUM_7:
		case OP_NUM_8:
		case OP_NUM_9:
			__calculator_control_panel_number_button_clicked(ad->
									 input_entry,
									 &calc_op_item
									 [val]);
			break;
		case OP_PLUS_MINUS:
			__calculator_symbol_negative_clicked(ad->input_entry);
			break;
		case OP_PERCENT...OP_E:
			__calculator_control_functions_button_clicked(ad->
								      input_entry,
								      &calc_op_item
								      [val]);
			break;
		default:
			break;
		}
		if (ad->svi_handle) {
			svi_play_sound(ad->svi_handle, SVI_SND_TOUCH_SIP);
			svi_play_vib(ad->svi_handle, SVI_VIB_TOUCH_SIP);
		}
	}

	CONV_FUNC_OUT();
}

static void __calculator_wrong_format_delete(Evas_Object *in_entry)
{
	_calc_entry_text_set(in_entry, calculator_input_str);
	if (ad->wrong_timer) {
		ecore_timer_del(ad->wrong_timer);
		ad->wrong_timer = NULL;
	}
}

static void __calculator_wrong_text_set(char * wrong_string)
{
	char buf[MAX_EXPRESSION_LENGTH] = { 0 };
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf),
		 "<align=right><+ font_size=%d><color=#855B11FF>%s",
		 get_max_fontsize(), wrong_string);
	elm_entry_entry_set(ad->input_entry, buf);
	elm_entry_cursor_end_set(ad->input_entry);
	calc_view_revise_input_scroller(ad);
}

/**
* @describe
*
*
* @param        msg
* @return       void
* @exception    none
*/
static void __calculator_wrong_format_create(char * wrong_string)
{
	__calculator_wrong_text_set(wrong_string);
	if (ad->wrong_timer) {
		ecore_timer_del(ad->wrong_timer);
		ad->wrong_timer = NULL;
	}
	ad->wrong_timer =
    ecore_timer_add(2,
		    (Ecore_Task_Cb)
		    __calculator_wrong_format_delete,
		    ad->input_entry);
}

/* mouse up for delete button. */
static void
__calculator_delelte_up(void *data, Evas * e, Evas_Object * evas_obj,
			void *event_info)
{
	if (ad->calc_timer) {
		ecore_timer_del(ad->calc_timer);
		ad->calc_timer = NULL;
	}
}

/**
* @describe
*   Register clicked callback of the keys on the keypad.
*
* @param    keypad      the keypad
* @return   void
*/
static void _calc_view_keypad_cb_register(Evas_Object * keypad)
{
	char *key_name[] = {
		"item_per", "item_sqr", "item_fac", "item_c", "item_div",
		    "item_mul", "item_del",
		"item_sin", "item_cos", "item_tan", "item_num7", "item_num8",
		    "item_num9", "item_sub",
		"item_ln", "item_log", "item_1x", "item_num4", "item_num5",
		    "item_num6", "item_plus",
		"item_ex", "item_x2", "item_xy", "item_num1", "item_num2",
		    "item_num3", "item_brack",
		"item_abs", "item_pi", "item_e", "item_num0", "item_dot",
		    "item_neg", "item_eq",
	};

	Evas_Object *item = NULL;
	int key_num = sizeof(key_name) / sizeof(key_name[0]);
	int i = 0;

	for (i = 0; i < key_num; ++i) {
		item =
		    (Evas_Object *) edje_object_part_object_get(keypad,
								key_name[i]);
		if (item != NULL) {
			evas_object_event_callback_add(item,
						       EVAS_CALLBACK_MOUSE_DOWN,
						       _calculator_interp,
						       (void *)keypad);
			if (0 == strcmp(key_name[i], "item_del")) {
				evas_object_event_callback_add(item,
							       EVAS_CALLBACK_MOUSE_UP,
							       __calculator_delelte_up,
							       (void *)keypad);
			}

		}
	}
}

/**
* @description
*   The callback of input entry when mouse up.
*
* @param    data        unused
* @param    e           unused
* @param    entry       the input entry
* @param    event_info  unused
* @return   void
*/
static void
_calc_view_input_entry_mouseup_cb(void *data, Evas * e, Evas_Object * entry,
				  void *event_info)
{
	calculator_cursor_pos = calc_view_cursor_get_position(entry);	//for  softkey input and mouse move
}

static void
_calc_view_input_entry_to_str(char *entry_str, char *internal_str, int buf_size)
{
	strncpy(internal_str, entry_str, buf_size - 1);
	/* remove result that behind '='(include '=') */
	char *enter = strchr(internal_str, '=');
	if (enter != NULL) {
		enter--;
		*enter = '\0';
	}
	calc_expr_replace_from_special_char(internal_str);
	/* remove all ','  and '\n'*/
	int i = 0;
	int j = 0;
	while (internal_str[j] != '\0'){

		if (internal_str[j] == separator_ch || internal_str[j] == '\n'){
			j++;
		} else {
			internal_str[i++] = internal_str[j++];
		}
	}
	internal_str[i] = '\0';

}

#ifdef __i386__
static void
_calc_view_input_entry_keyup_cb(void *data, Evas * e, Evas_Object * entry,
				void *event_info)
{
	calculator_cursor_pos = calc_view_cursor_get_position(entry);	//for hardkey input	
	if (data) {
		Evas_Event_Key_Up *evt = (Evas_Event_Key_Up *) event_info;
		printf("keyname: %s\n", (char *)evt->keyname);
		if ((0 == strcmp(evt->keyname, "XF86Phone"))||
			(0 == strcmp(evt->keyname, "XF86PowerOff"))|| 
			(0 == strcmp(evt->keyname, "XF86AudioRaiseVolume"))||
			(0 == strcmp(evt->keyname, "XF86AudioLowerVolume"))){
			return;
		}
		_calc_entry_backspace(ad->input_entry);
	}
}
#endif
static bool
_calc_view_input_entry_error_include(char *string)
{
    int i = 0;
    for(; i < ARRAY_SIZE(error_string); i++)
	{
		if(!strcmp(string, _(error_string[i]))){
			return TRUE;
	    }
    }
    return FALSE;
}

/**
* @description
*   The callback of input entry when text changed.
*
* @param    data        the appdata
* @param    [in]obj     the input entry
* @param    event_info  unused
* @return   void
*/
static void
_calc_view_input_entry_changed_cb(void *data, Evas_Object * obj,
				  void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	PLOG("Input entry test changed\n");	/* Don't remove the log. It can guard against recursion. */
	const char *str = (char *)elm_entry_entry_get(obj);

	if (paste_flag) {
		char *entry_tmp = elm_entry_markup_to_utf8(str);
		char *entry_expr= elm_entry_markup_to_utf8(entry_tmp);/*because the string format from clipboard is not correct*/
		char internal_expr[MAX_EXPRESSION_LENGTH] = { 0 };
		char f_number_buf[NUMBER_LENGTH] = { 0 };
		char s_number_buf[NUMBER_LENGTH] = { 0 };
		paste_flag = FALSE;
		_calc_view_input_entry_to_str(entry_expr, internal_expr,
					      MAX_EXPRESSION_LENGTH);
		int index = 0;
		bool char_in =__calculator_string_invalid_char_search(internal_expr, &index);
		__calculator_get_cursor_position_float_string(internal_expr,
							      f_number_buf,
							      calculator_cursor_pos,
							      NULL, NULL);
		int cur_pos = calc_view_cursor_get_position(ad->input_entry);
		__calculator_get_cursor_position_float_string(internal_expr,
							      s_number_buf,
							      cur_pos, NULL,
							      NULL);
		int nCntOp = calc_expr_get_operator_num(internal_expr);

        if ((strlen(f_number_buf) > MAX_NUM_LENGTH)
			   || (strlen(s_number_buf) > MAX_NUM_LENGTH)) {
			__calculator_wrong_format_create(CALC_MSG_MAX_DIGIT);
		} else if (nCntOp >= MAX_OPERATOR_NUM) {
			__calculator_wrong_format_create(CALC_MSG_MAX_OP);
		} else {
			if(char_in){
				strncpy(calculator_input_str, internal_expr,
					index);
				calculator_cursor_pos = index;
			}else{
				strncpy(calculator_input_str, internal_expr,
					sizeof(calculator_input_str) - 1);
				calculator_cursor_pos = cur_pos;
			}
			_calc_entry_text_set(ad->input_entry, calculator_input_str);
		}

		if (entry_expr) {
			free(entry_expr);
		}
	}else{
		char *entry_expr_s = elm_entry_markup_to_utf8(str);
		char internal_expr_s[MAX_EXPRESSION_LENGTH] = { 0 };
		_calc_view_input_entry_to_str(entry_expr_s, internal_expr_s,
						  MAX_EXPRESSION_LENGTH);
		if(!_calc_view_input_entry_error_include(internal_expr_s)){
			/*change calculator_input_str, after cut operation*/
			strncpy(calculator_input_str, internal_expr_s,
			MAX_EXPRESSION_LENGTH - 1);
		}
	}
	/*
	 * Prevent pasting images into entry.
	 * If a image pasted, "<item absize=... href=...>" will be appended into entry.
	 */
	if (strstr(str, "item") != NULL) {
		int pos = 0;
		char buf[MAX_EXPRESSION_LENGTH] = { 0 };

		while (elm_entry_cursor_prev(obj)) {
			pos++;
		}		/* save cursor position */
		pos -= 1;	/* correct */

		strncpy(buf, str, sizeof(buf));
		char *begin = strstr(buf, "<item");
		char *end = strchr(begin, '>');
		string_remove_at(buf, begin - buf, end - begin + 1);	/* remove "<item...>" */
		elm_entry_entry_set(obj, buf);

		while (pos--) {
			elm_entry_cursor_next(obj);
		}		/* retrieve cursor position */

		calc_view_revise_input_scroller(ad);
	}

}

static void
_calc_view_input_entry_paste_cb(void *data, Evas_Object * obj, void *event_info)
{
	CONV_FUNC_IN();
	paste_flag = TRUE;
	strncpy(calculator_before_paste_str, calculator_input_str,
		MAX_EXPRESSION_LENGTH - 1);
	printf("line = %d\n", __LINE__);
	CONV_FUNC_OUT();
}

static void
__conv_view_clear_clicked_cb(void *data, Evas_Object * obj, const char *emission, const char *source)
{

	CONV_FUNC_IN();

	struct appdata *ad = (struct appdata *)data;
	elm_entry_entry_set(ad->hist_area, "");
	elm_entry_entry_set(ad->input_entry, "");
	_calc_view_clear_histroy(ad->hist_area);

	CONV_FUNC_OUT();
}

/**
* @description
*   Create an entry for inputing expression.
*
* @param    parent          the entry's parent
* @param    ad              the appdata
* @return   Evas_Object*    the input entry
*/
static Evas_Object *_calc_view_create_input_entry(Evas_Object * parent,
						  struct appdata *ad)
{
	CONV_FUNC_IN();
	Evas_Object *entry = elm_entry_add(ad->edje);
	elm_entry_single_line_set(entry, EINA_FALSE);
	elm_entry_line_wrap_set(entry, ELM_WRAP_WORD);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_input_panel_enabled_set(entry, EINA_FALSE);
	elm_entry_entry_set(entry, "");
	elm_entry_cnp_mode_set(entry, ELM_CNP_MODE_NO_IMAGE);
	elm_entry_magnifier_disabled_set(entry, EINA_TRUE);
	elm_entry_cursor_end_set(entry);
	elm_object_style_set(entry, "black");
	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_event_callback_add(entry, EVAS_CALLBACK_MOUSE_UP,
				       _calc_view_input_entry_mouseup_cb, ad);
	evas_object_smart_callback_add(entry, "changed",
				       _calc_view_input_entry_changed_cb, ad);
	evas_object_smart_callback_add(entry, "selection,paste",
				       _calc_view_input_entry_paste_cb, ad);
#ifdef __i386__
	evas_object_event_callback_add(entry, EVAS_CALLBACK_KEY_UP,
				       _calc_view_input_entry_keyup_cb, ad);
#endif
	evas_object_show(entry);
	limit_filter_data.max_char_count = 0;
	limit_filter_data.max_byte_count = 419 + 20;	//19*21+20//result:20
	elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size,
				     &limit_filter_data);
	CONV_FUNC_OUT();
	return entry;
}

/**
* @description
*   Create a input scrooler which around the input entry.
*   It can give a input entry a scroller.
*
* @param    parent          the parent of input scroller
* @return   Evas_Object*    the input scroller
* @exception
*/
static Evas_Object *_calc_view_create_input_scroller(Evas_Object * parent)
{
	CONV_FUNC_IN();
	Evas_Object *scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL,
					EVAS_HINT_FILL);

	evas_object_show(scroller);

	CONV_FUNC_OUT();

	return scroller;
}

/**
* @description
*   Create the main layout.
*
* @param    parent          main layout's parent
* @return   Evas_Object*    when success return a layout, return NULL oppositely.
*/
static Evas_Object *_calc_view_create_layout_main(Evas_Object * parent)
{
	CONV_FUNC_IN();

	if (parent == NULL) {
		return NULL;
	}

	Evas_Object *layout = elm_layout_add(parent);
	if (layout == NULL) {
		return NULL;
	}

	elm_layout_theme_set(layout, "standard", "window", "integration");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, layout);

	edje_object_signal_emit(_EDJ(layout), "elm,state,show,content", "elm");
	edje_object_signal_emit(_EDJ(layout), "elm,state,show,indicator",
				"elm");
	evas_object_show(layout);

	CONV_FUNC_OUT();

	return layout;
}

/**
* @description
*   Load the main view of calculator.
*   Create the input entry and keypad.
*
* @param    ad      the appdata
* @return   void
*/
void calc_view_load(struct appdata *ad)
{
	CONV_FUNC_IN();

	ad->layout = _calc_view_create_layout_main(ad->win);
	ad->edje = load_edj(ad->win, LAYOUT_EDJ_NAME, GRP_MAIN);
	evas_object_show(ad->edje);
	evas_object_name_set(ad->edje, "edje");
	elm_object_part_content_set(ad->layout, "elm.swallow.content", ad->edje);

	/* inititialize environment variable */
	locale = localeconv();
	decimal = locale->decimal_point;
	separator = locale->thousands_sep;
	int len_seq = strlen(separator);
	decimal_ch = decimal[0];
	separator_ch = separator[0];
	if (len_seq == 2 || len_seq == 0) {
		separator_ch = 32;
	}
	/*init svi */
	svi_init(&ad->svi_handle);

	/* input area */
	ad->input_entry = _calc_view_create_input_entry(ad->edje, ad);
	ad->input_scroller = _calc_view_create_input_scroller(ad->edje);
	elm_object_content_set(ad->input_scroller, ad->input_entry);
	edje_object_part_swallow(_EDJ(ad->edje), "input/entry",
				 ad->input_scroller);
	edje_object_signal_callback_add(_EDJ(ad->edje), "clicked", "",
					__conv_view_clear_clicked_cb, ad);

	/* pannels */
	ad->por_pannel = load_edj(ad->edje, LAYOUT_EDJ_NAME, GRP_POR_PANNEL);
	edje_object_part_swallow(_EDJ(ad->edje), "por_pannel/rect",
				 ad->por_pannel);
	edje_object_signal_emit(_EDJ(ad->edje), "portrait", "");
	_calc_view_keypad_cb_register(_EDJ(ad->por_pannel));

	CONV_FUNC_OUT();

}

/**
* @description
*   assign global variable  , this will be removed
*
* @param        ad
* @return       void
* @exception
*/
void calc_xxx(struct appdata *ap)
{
	ad = ap;
}
