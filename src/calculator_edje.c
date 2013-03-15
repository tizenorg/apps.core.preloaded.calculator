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

#include <stdbool.h>		/*  true/false */
#include <Elementary.h>
#include <dlog.h>

#include "calc-main.h"
#include "calculator_parser.h"
#include "calc-string.h"
#include "calc-expression.h"
#include "calc-view.h"
#include <feedback.h>
#include <Ecore_X.h>
#include <vconf.h>

#define CALCULATOR_CHAR_IS_OPERATOR(C) ((C == '+')||(C == '-')||(C == 'x')||(C == '/'))	/**<judge if a char is a basic operator*/
#define CALCULATOR_CHAR_IS_MULTI_DIVIDE_OPERATOR(C) ((C == 'x')||(C == '/'))			/**<judge if an operator is "*" or "/"*/
#define CALCULATOR_CHAR_IS_PLUS_DEDUCT(C) ((C == '+')||(C == '-'))
#define CALCULATOR_CHAR_IS_DIGITAL(C)(C >= '0' && C <= '9')
#define CALCULATOR_IS_DIGIT_DOT(ch, decimal) (isdigit(ch) || decimal == (ch))

static Elm_Entry_Filter_Limit_Size limit_filter_data;
extern void _calc_view_show_newest_histroy(Evas_Object * entry);
extern void _calc_view_clear_histroy(Evas_Object * entry);
extern gboolean __calculator_search_function(char *op, int* len);
static void __calculator_wrong_format_create(char * wrong_string);
void calc_view_load(struct appdata *ad);

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

int cur_fontsize = 70;
int small_fontsize = 58;
int default_fontsize = 0;
int min_len = 13;
int max_len = 0;
int scientific_result_len = 8;

bool select_mode = FALSE;

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

	{OP_10X, "10^x", "10^("},
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
    "IDS_CCL_POP_INPUT_ERROR",
};

calculator_state_t calculator_get_state()
{
	return calculator_state;
}

void _calc_add_tag(char *string, char *format_string)
{
	CALC_FUN_BEG();
	int i = 0;
	int begin = 0;
	int end = 0;
	char buf[MAX_EXPRESSION_LENGTH] = { 0 };
	char tmp[MAX_EXPRESSION_LENGTH] = { 0 };
	char *p = NULL;
	while (string[i] != '\0') {
		if (CALCULATOR_CHAR_IS_DIGITAL(string[i])
		    || string[i] == separator_ch || string[i] == decimal_ch
		    ||( string[i]=='(' && string[i+1]=='\xe2' &&string[i+2]=='\x88' && string[i+3]=='\x92')) {
			if ( string[i]=='(' && string[i+1]=='\xe2' &&string[i+2]=='\x88' && string[i+3]=='\x92') {
				memset(buf, 0, sizeof(buf));
				snprintf(buf, sizeof(buf),
				 "<align=right><font_size=%d><color=#505050FF>(<align=right><font_size=%d><color=#505050FF>-",
				 cur_fontsize, cur_fontsize);
				strcat(format_string, buf);
				i=i+4;
			}
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
				 "<align=right><font_size=%d><color=#505050FF>%s",
				 cur_fontsize, tmp);
			strcat(format_string, buf);
		} else {
			begin = i;
			p = &string[begin];
			while (!CALCULATOR_CHAR_IS_DIGITAL(string[i])
			       && string[i] != separator_ch
			       && string[i] != decimal_ch
			       && string[i] != '\0') {
			       if ( string[i]=='(' && string[i+1]=='\xe2' &&string[i+2]=='\x88' && string[i+3]=='\x92') {
					break;
			       } else {
					i++;
			       }
			}
			end = i - 1;
			strncpy(tmp, p, MAX_EXPRESSION_LENGTH - 1);
			tmp[end - begin + 1] = '\0';
			memset(buf, 0, sizeof(buf));
			snprintf(buf, sizeof(buf),
				 "<align=right><font_size=%d><color=#505050FF>%s",
				 cur_fontsize, tmp);
			strcat(format_string, buf);
		}

	}
	CALC_FUN_END();
}

/**
* @describe
* Change line and font size when it is needed.
* Refer to the auto font resizing rules.
* @param	tag_text		The text with tags.
* @return	void
* @exception
*/
void _calc_add_br(char *tag_text)
{
	CALC_FUN_BEG();
	if (tag_text  == NULL) {
		return;
	}
	app_device_orientation_e curr = app_get_device_orientation();
	if (curr  == APP_DEVICE_ORIENTATION_270 || curr == APP_DEVICE_ORIENTATION_90) {
		return;
	}
	int line_valid_num = 0;
	int whole_valid_num = 0;
	int operator_tag = -1;
	int operator_location = -1;
	/* record is there an operator in one line? */
	bool operator_flag = FALSE;
	/* when change to small font, we should rescan the tag_text */
	bool rescan_flag = FALSE;
	int cur_len = 0;
	char buf[MAX_TAG_EXPRESSION_LENGTH] = { 0 };
	strcpy(buf, tag_text);
	int br_number = 0;
	int i = 0;
	int j = 0;
	for ( ; tag_text[i] != '\0' && calculator_input_str[j] != '\0'; ) {
		while (tag_text[i] == '<') {
			while (tag_text[i++] != '>') {
				/* NULL */
				;
			}
		}
		/* if calculator_input_str[j] is "*" or "/", tag_text[i] would be
			"\xc3\x97" or "\xc3\xb7" */
		if (tag_text[i] == calculator_input_str[j] || (tag_text[i] == '\xc3') ||
			((tag_text[i] == '\xe2') && (tag_text[i+1] == '\x88') && (tag_text[i+2] == '\x92'))) {

			if ((CALCULATOR_CHAR_IS_OPERATOR(calculator_input_str[j])) &&
				(!(calculator_input_str[j] == '-' && calculator_input_str[j-1] == '('))) {
				if (!operator_flag || rescan_flag) {
					operator_location = j;
					operator_tag = i;
					operator_flag = TRUE;
				}
			}

			whole_valid_num++;
			line_valid_num++;
			if (br_number < 2) {
				cur_fontsize = default_fontsize;
				cur_len = min_len;
			} else {
				if ((line_valid_num >= min_len) && (!rescan_flag)) {
					rescan_flag = TRUE;
					cur_fontsize = small_fontsize;
					strcpy(tag_text, buf);
					/* restore to the original state, then scan from the begin */
					i = 0;
					j = 0;
					whole_valid_num = 1;
					line_valid_num = 1;
					operator_flag = FALSE;
					operator_tag = -1;
					operator_location = -1;
					cur_len = max_len + 1;
				}
			}

			if (line_valid_num >= cur_len) {
				if (operator_flag && operator_tag > 0) {
					string_insert(tag_text, operator_tag, "<br>");
					line_valid_num = whole_valid_num-operator_location;
					operator_flag = FALSE;
					operator_tag = -1;
					operator_location = -1;
				} else {
					string_insert(tag_text, i, "<br>");
					line_valid_num = 0;

				}
				i = i + 4;
				if (br_number < 2) {
					br_number++;
				}
			}
			i++;
			j++;
		}else {
			i++;
		}
    }
    CALC_FUN_END();
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
static void _calc_entry_text_set(Evas_Object * entry, const char *str)
{
	CALC_FUN_BEG();
	if (select_mode) {
		select_mode = FALSE;
	}
	char tmp[MAX_EXPRESSION_LENGTH] = { 0 };
	char tag_text[MAX_TAG_EXPRESSION_LENGTH] = { 0 };
	char new_font_str[MAX_EXPRESSION_LENGTH] = { 0 };
	char old_font_str[MAX_EXPRESSION_LENGTH] = { 0 };

	Eina_Bool status = elm_entry_editable_get(entry);
	if (EINA_FALSE == status) {
		elm_entry_editable_set(entry, EINA_TRUE);//chx add recover the old style
		elm_entry_line_wrap_set(entry,ELM_WRAP_WORD);
		elm_entry_input_panel_enabled_set(entry, EINA_FALSE);
		elm_object_style_set(entry, "black");
		elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &limit_filter_data);
		elm_entry_text_style_user_push(entry, "DEFAULT='right_margin=32'");
	}

	if (str == NULL) {
		return;
	}

	if (strlen(str) == 0) {
		elm_entry_entry_set(entry, "");
		elm_entry_cursor_end_set(entry);
		elm_object_focus_set(entry, EINA_TRUE);
		CALC_FUN_END();
		return;
	}
	calc_expr_format_expression(str, tmp);
	CALC_INFO("ASCII :: expr_out = %s", tmp);
	CALC_INFO("ASCII :: expr_out = %d", tmp);
	_calc_add_tag(tmp, tag_text);
	int pre_fontsize = cur_fontsize;
	_calc_add_br(tag_text);
	snprintf(old_font_str, sizeof(old_font_str), "=%d", pre_fontsize);
	snprintf(new_font_str, sizeof(new_font_str), "=%d", cur_fontsize);
	string_replace(tag_text, old_font_str, new_font_str);
	CALC_INFO("ASCII :: tag_text = %s", tag_text);
	CALC_INFO("ASCII :: tag_text = %d", tag_text);
	elm_entry_entry_set(entry, tag_text);
	if(calculator_cursor_pos == strlen(calculator_input_str)){
		elm_entry_cursor_end_set(entry);
		elm_object_focus_set(entry, EINA_TRUE);
	}else {
		calc_view_cursor_set_position(entry, calculator_cursor_pos);
	}
	CALC_FUN_END();
}

void _calc_entry_text_set_rotate(struct appdata *ad)
{
    CALC_FUN_BEG();
	if (calculator_state == CALCULATOR_CALCULATED) {
		_calc_view_show_newest_histroy(ad->input_entry);
		elm_entry_cursor_end_set(ad->input_entry);
		elm_object_focus_set(ad->input_entry, EINA_TRUE);
	} else {
		_calc_entry_text_set(ad->input_entry, calculator_input_str);
	}
	CALC_FUN_END();
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
	CALC_FUN_BEG();
	CALC_INFO("ASCII :: str = %s", str);
	CALC_INFO("ASCII :: str = %d", str);
	calc_expr_input_insert(calculator_input_str, &calculator_cursor_pos, str);
	_calc_entry_text_set(entry, calculator_input_str);
	CALC_FUN_END();
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
	CALC_FUN_BEG();
	string_remove_at(calculator_input_str, from_pos,
			 end_pos - from_pos + 1);
	calculator_cursor_pos = from_pos;
	_calc_entry_text_set(entry, calculator_input_str);
	CALC_FUN_END();
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
	CALC_FUN_BEG();
	calc_view_cursor_set_position(entry, calculator_cursor_pos);
	CALC_FUN_END();
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
	CALC_FUN_BEG();
	calc_expr_input_backspace(calculator_input_str, &calculator_cursor_pos);
	_calc_entry_text_set(entry, calculator_input_str);
	CALC_FUN_END();
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
	CALC_FUN_BEG();
	memset(calculator_input_str, 0, sizeof(calculator_input_str));
	calculator_cursor_pos = 0;
	_calc_entry_text_set(entry, "");
	calc_view_cursor_set_position(entry, calculator_cursor_pos);
	CALC_FUN_END();
}

/* END INPUT ENTRY RELATED */

/**
* @describe
* Get the operand start and end location where the cursor in.
* 1.2+6.|43 (cursor is before 4), then return begin=4 end=7
* Espcially, cursor is after +, it means the operand which follows
* the operator, so it return the same result as before.
* This is add by on 2012/5/29
*
* @param[in]	text	The current calculator input string
* @param[out]	begin	The start location of an operand
* @param[out]	end	The end location of an operand
* @return		Is cursor in an operand
* @retval	true	The cursor is in the operand
* @retval	false	The cursor is not in the operand
* @exception
*/
static bool
__calculator_get_float_num_in_cursor_position(char *text, int cur_pos,
					      int *begin, int *end)
{
	CALC_FUN_BEG();
	if (text == NULL) {
		return false;
	}
	int pos = cur_pos - 1;
	if (pos < 0) {
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
	} else if (CALCULATOR_CHAR_IS_OPERATOR(text[pos]) || text[pos] == '(') {
		for (_end = pos + 1;
		     CALCULATOR_IS_DIGIT_DOT(text[_end], decimal_ch)
		     && _end < strlen(text); ++_end) {
			;	/* NULL */
		}
		_end--;
		*begin = pos+1;
		*end = _end;
		return true;
	} else {
		return false;
	}
	CALC_FUN_END();
}

/**
* @describe
* Get the operand start and end location where the cursor in the operand
* Actually, it would call "__calculator_get_float_num_in_cursor_position"
* function to recognize whether the cursor is in the operand.
*
* @param[in]	entry_text	The current calcualtor input string
* @param[out]	str	The operand which the cursor in
* @param[out]	from	The operand start location which contain the cursor
* @param[out]	end	The operand end location which contain the cursor
* @return		Is cursor in an operand
* @retval	true	The cursor is in the operand
* @retval	false	The cursor is not in the operand
* @exception
*/
static bool
__calculator_get_cursor_position_float_string(char *entry_text, char *str,
					      int cur_pos, int *from, int *end)
{
	CALC_FUN_BEG();
	if (entry_text == NULL) {
		return false;
	}
	int from_pos = cur_pos;
	int end_pos = cur_pos;

	if (false ==
	    __calculator_get_float_num_in_cursor_position(entry_text, cur_pos, &from_pos, &end_pos)) {
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
	CALC_FUN_END();
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
	CALC_FUN_BEG();

	int from_pos = calculator_cursor_pos;
	int end_pos = calculator_cursor_pos;

	if (false ==  __calculator_get_float_num_in_cursor_position(entry_text, calculator_cursor_pos, &from_pos, &end_pos)) {
		return false;
	}
	snprintf(str, calculator_cursor_pos - from_pos + 1, "%s", entry_text + from_pos);
	CALC_FUN_END();
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
	CALC_FUN_BEG();

	if (calculator_cursor_pos > 0) {
		strncpy(str, entry_text, calculator_cursor_pos);
		str[calculator_cursor_pos] = '\0';
	} else {
		strcpy(str, "");
	}
	CALC_FUN_END();
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
	CALC_FUN_BEG();
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
	CALC_FUN_END();
}

static bool  __calculator_string_digit_in(const char *input)
{
	int i =0;
	while(input[i]!='\0'){
		if(IS_DIGITAL(input[i])){/*here ,digit include "p" and "e"*/
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
	CALC_FUN_BEG();
	if (input == NULL) {
		return FALSE;
	}
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
	CALC_FUN_END();
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
	if (!char_in) {/*no charactor*/
		return FALSE;
	}
		*index += sub_index;
		p += sub_index;
	while (p) {
		if (!__calculator_search_function(p, &len)) {/*charactor not a function*/
			return TRUE;
		} else {/*the first sevaral charactors are function, continue search*/
			*index += len;
			p += len;
		}
		/*Fix bug that paste did not filter invaild word, beacuse here it jump to
		*  the sub_index directly, but the sub_index would change with the string
		*  So, it should recalculate every time*/
		char_in = __calculator_string_char_search(p, &sub_index);
		if (char_in) {
			*index += sub_index;
			p += sub_index;
		} else {
			return FALSE;
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
__calculator_control_panel_number_button_clicked(Evas *e, Evas_Object *entry, op_item_t *op_item)
{
	CALC_FUN_BEG();
	if (select_mode) {
		char *selected_str = elm_entry_markup_to_utf8(elm_entry_selection_get(entry));
		if (selected_str) {
			if (calc_select_string_search(selected_str)) {
				evas_event_feed_key_down(e, op_item->op_sym, op_item->op_sym,
					op_item->op_sym, op_item->op_sym, 0, NULL);
			} else {
				__calculator_wrong_format_create(CALC_MSG_WRONG_FORMAT);
				calculator_cursor_pos = strlen(calculator_input_str);
			}
			SFREE(selected_str);
		}
		return;
	}
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
	__calculator_get_cursor_position_float_string(entry_text, str_buf, calculator_cursor_pos, NULL, NULL);
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
	} else if (nPointCnt >= MAX_DECIMAL_NUM && calculator_cursor_pos > nDigitCnt) {
		__calculator_wrong_format_create(CALC_MSG_MAX_DEC_DIGIT);
		return;
	} else if (before_len > 0
		   && (__calculator_string_get_char_type(before_cursor[before_len - 1]) ==
		   CHAR_IS_PI
		   || __calculator_string_get_char_type(before_cursor[before_len - 1]) ==
		   CHAR_IS_E)) {
		/* input digital after "e" or "p", the "x" opeartor will be added automatically */
		_calc_entry_text_insert(entry, "x");
		_calc_entry_text_insert(entry, op_item->op_sym);
		calculator_state = CALCULATOR_OPERAND_INPUT;
		return;
	} else if (before_len > 0 && ((before_cursor[before_len - 1] == '(') ||CALCULATOR_CHAR_IS_DIGITAL(before_cursor[before_len - 1])
		       || CALCULATOR_CHAR_IS_OPERATOR(before_cursor[before_len - 1]) || (int)before_cursor[before_len - 1] > 120))	//* or/
	{
		_calc_entry_text_insert(entry, op_item->op_sym);
		calculator_state = CALCULATOR_OPERAND_INPUT;
		return;
	} else if (before_len > 0 && (before_cursor[before_len - 1] == ')')) {
		__calculator_wrong_format_create(CALC_MSG_OP_FIRST);
		return;
	} else {
		_calc_entry_text_insert(entry, op_item->op_sym);
		calculator_state = CALCULATOR_OPERAND_INPUT;
		return;
	}
	CALC_FUN_END();
}

/**
* @describe
* Deal the event when dot clicked on the keyboard
*
* @param[in]	entry	The input entry
* @return		void
* @exception
*/
static void _calc_btn_dot_clicked(Evas_Object *entry)
{
	CALC_FUN_BEG();
	if (entry == NULL) {
		return;
	}
	char str_num[CALCULATOR_CONTENT_LEN] = { 0 };
	int str_num_len = 0;

	char decimal_str[32] = { 0 };

	/* replace special characters */
	char entry_text[MAX_EXPRESSION_LENGTH] = { 0 };
	strncpy(entry_text, calculator_input_str, MAX_EXPRESSION_LENGTH - 1);

	if (calculator_state == CALCULATOR_CALCULATED) {
		_calc_entry_clear(entry);
		memset(entry_text,0x00,sizeof(entry_text));
	}

	int from_pos = 0;
	int end_pos = 0;
	if (!__calculator_get_cursor_position_float_string
	    (entry_text, str_num, calculator_cursor_pos, &from_pos, &end_pos)) {
		if (calculator_cursor_pos == 0
		    || IS_OPERATOER(calculator_input_str[from_pos - 1])
			|| calculator_input_str[from_pos - 1] == '(') {
			snprintf(decimal_str, sizeof(decimal_str), "0%c",
				 decimal_ch);
			_calc_entry_text_insert(entry, decimal_str);
			calculator_state = CALCULATOR_OPERAND_INPUT;
			return;
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
		return;
	}
	if (IS_OPERATOER(calculator_input_str[calculator_cursor_pos-1])
		|| calculator_input_str[calculator_cursor_pos-1] == '(') {
		snprintf(decimal_str, sizeof(decimal_str), "0%c", decimal_ch);
	} else {
		snprintf(decimal_str, sizeof(decimal_str), "%c", decimal_ch);
	}
	_calc_entry_text_insert(entry, decimal_str);
	calculator_state = CALCULATOR_OPERAND_INPUT;
	CALC_FUN_END();
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
static void _calc_btn_backspace_clicked(Evas *e, Evas_Object *entry)
{
	CALC_FUN_BEG();
	if (select_mode) {
		char *selected_str = elm_entry_markup_to_utf8(elm_entry_selection_get(entry));
		if (selected_str) {
			if (calc_select_string_search(selected_str)) {
				evas_event_feed_key_down(e, "KP_BACKSPACE", "", "", "", 0, NULL);
			} else {
				__calculator_wrong_format_create(CALC_MSG_WRONG_FORMAT);
				calculator_cursor_pos = strlen(calculator_input_str);
			}
			SFREE(selected_str);
		}
		return;
	}
	if (calculator_state == CALCULATOR_CALCULATED) {
		calculator_state = CALCULATOR_OPERATOR_INPUT;
		if (calculator_cursor_pos > strlen(calculator_input_str)) {
			calculator_cursor_pos = strlen(calculator_input_str);	/* set last position */
		}
	}
	_calc_entry_backspace(entry);
	CALC_FUN_END();
}

static int __calculator_delete_long_press(void *data)
{
	CALC_FUN_BEG();
	Evas_Object *entry = (Evas_Object *) data;

	if (calculator_state == CALCULATOR_CALCULATED) {
		calculator_state = CALCULATOR_OPERATOR_INPUT;
		if (calculator_cursor_pos > strlen(calculator_input_str)) {
			calculator_cursor_pos = strlen(calculator_input_str);	/* set last position */
		}
	}
	_calc_entry_backspace(entry);
	CALC_FUN_END();
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
__calculator_control_normal_func_clicked(Evas *e,Evas_Object *entry,
					 op_item_t *op_item)
{
	CALC_FUN_BEG();
	if (select_mode) {
		char *selected_str = elm_entry_markup_to_utf8(elm_entry_selection_get(entry));
		if (selected_str) {
			if (calc_select_string_search(selected_str)) {
				evas_event_feed_key_down(e, op_item->op_sym, op_item->op_sym, op_item->op_sym, op_item->op_sym, 0, NULL);
			} else {
				__calculator_wrong_format_create(CALC_MSG_WRONG_FORMAT);
				calculator_cursor_pos = strlen(calculator_input_str);
			}
			SFREE(selected_str);
		}
		return;
	}
	char all_input[MAX_EXPRESSION_LENGTH] = { 0 };
	strncpy(all_input, calculator_input_str, MAX_EXPRESSION_LENGTH - 1);

	if (calculator_state == CALCULATOR_CALCULATED) {
		edje_object_signal_emit(_EDJ(ad->edje), "show,input", "");
		char temp[20] = { 0 };

		calc_expr_num_format_result(last_result, temp);
		_calc_entry_clear(entry);
		//result < 0 or sicience number
		if (temp[0] == '-' || strchr(temp, 'E')) {
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
	if (!strcmp(op_item->op_sym, "x") ||!strcmp(op_item->op_sym, "/") ||!strcmp(op_item->op_sym, "+")) {
		if (!__calculator_string_digit_in(calculator_input_str)) {
			return;
		}
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
		if (before_cursor[input_len - 1] == '(' && !strcmp(op_item->op_sym, "-")) {
			_calc_entry_text_insert(entry, op_item->op_sym);
			calculator_state = CALCULATOR_OPERATOR_INPUT;
		} else if (((input_len > 1) && (before_cursor[input_len - 1] == '-') && (before_cursor[input_len - 2] == '('))	//(-
			   || /*before_cursor[input_len - 1] == decimal_ch ||*/ before_cursor[input_len - 1] == '(')	// . or (
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
			if (!IS_DIGITAL(before_cursor[input_len - 1])
					&& (before_cursor[input_len - 1] != decimal_ch)) {
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
	CALC_FUN_END();
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
	CALC_FUN_BEG();

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
	CALC_FUN_END();
}

/* search the previous operator and value */
static char * __calculator_search_prev_input(char *input_str)
{
	CALC_FUN_BEG();
	int i = 0;
	int bracket_number = 0;
	char *prev_input = NULL;
	for(; i< strlen(input_str); i++) {
		if ('(' ==(input_str[i]) ){
			bracket_number++;
		}
		if( ')' == input_str[i]){
			bracket_number --;
		}
		if (CALCULATOR_CHAR_IS_PLUS_DEDUCT(input_str[i])
			|| CALCULATOR_CHAR_IS_MULTI_DIVIDE_OPERATOR(input_str[i])) {
			if ( !bracket_number){
				prev_input = &input_str[i];
				return prev_input;
			}
		}
	}
	CALC_FUN_END();
	return prev_input;
}

/**
* @describe
* Deal the event when "=" clicked on the keyboard
*
* @param[in]	entry	The input entry
* @return		void
* @exception
*/
static void __calculator_op_equal(Evas_Object *entry)
{
	CALC_FUN_BEG();
	if (entry == NULL) {
		return;
	}
	if (calculator_state == CALCULATOR_CALCULATED) {
		/*duplicate previous input operator and value*/
		char *p = __calculator_search_prev_input(calculator_input_str);
		if (p == NULL) {
			return;
		}
		char prev_input[32] = { 0 };
		int len = g_strlcpy(prev_input, p, sizeof(prev_input));
		if (len >= sizeof(prev_input)) {
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
	char result_format[MAX_RESULT_LENGTH] = { 0 };
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

	if (!calculator_calculate(str_buf, &result, error_msg)) {
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
		memset(result_format, 0, sizeof(result_format));
		calc_expr_format_expression(calculator_input_str,
					    item.expression);
		item.result = result;
		calc_expr_format_expression(result_buf, result_format);
		if (result_buf[0] == '-') {
			string_replace(result_format, "\xe2\x88\x92", "-");
		}
#ifdef SAVE_HISTORY
		calc_view_save_history(&item);
#endif
		/* show result */
		calculator_state = CALCULATOR_CALCULATED;
		calc_view_show_result(result_format, ad);

	}
	CALC_FUN_END();
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
	CALC_FUN_BEG();
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
	CALC_FUN_END();
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
__calculator_control_functions_button_clicked(Evas *e, Evas_Object *entry, op_item_t *op_item)
{
	CALC_FUN_BEG();

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
		if (select_mode) {
			__calculator_wrong_format_create(CALC_MSG_WRONG_FORMAT);
			calculator_cursor_pos = strlen(calculator_input_str);
			return;
		}
		/* if it is calculated state */
		if (calculator_state == CALCULATOR_CALCULATED) {
			edje_object_signal_emit(_EDJ(ad->edje), "show,input","");

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

			if (false ==  __calculator_get_cursor_position_float_string(all_input, str_num, calculator_cursor_pos, &from_pos, &end_pos)) {
				__calculator_wrong_format_create(CALC_MSG_NUM_FIRST);
				calculator_cursor_pos = strlen(calculator_input_str);
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

				_calc_entry_text_remove(entry, from_pos, end_pos);
				_calc_entry_text_insert(entry, temp);

				calculator_state = CALCULATOR_OPERAND_INPUT;
				return;
			}
		}
		break;
	case OP_PI:
		if (select_mode) {
			__calculator_wrong_format_create(CALC_MSG_WRONG_FORMAT);
			calculator_cursor_pos = strlen(calculator_input_str);
			return;
		}
		if (calculator_state == CALCULATOR_CALCULATED) {
			edje_object_signal_emit(_EDJ(ad->edje), "show,input",
						"");
			_calc_entry_clear(entry);
			_calc_entry_text_insert(entry, op_item->op_name);
			break;
		}
		if ((before_len != 0) && (!IS_DIGITAL(before_cursor[before_len - 1]))
		    && (!IS_OPERATOER(before_cursor[before_len - 1]))
		    && (before_cursor[before_len - 1] != '(')
		    && (before_cursor[before_len - 1] != 'P')
		    && before_cursor[before_len - 1] != 'C') {
			__calculator_wrong_format_create(CALC_MSG_OP_FIRST);
			break;
		}
		/* input digital after "p", the "x" opeartor will be added automatically */
		if(IS_DIGITAL(before_cursor[before_len - 1])){
			_calc_entry_text_insert(entry, "x");
		}
		_calc_entry_text_insert(entry, op_item->op_name);
		break;

	case OP_E:
		if (select_mode) {
			__calculator_wrong_format_create(CALC_MSG_WRONG_FORMAT);
			calculator_cursor_pos = strlen(calculator_input_str);
			return;
		}
		if (calculator_state == CALCULATOR_CALCULATED) {
			edje_object_signal_emit(_EDJ(ad->edje), "show,input","");
			_calc_entry_clear(entry);
			_calc_entry_text_insert(entry, op_item->op_name);
			break;
		}
		if ((before_len != 0) && (!IS_DIGITAL(before_cursor[before_len - 1]))
		    && (!IS_OPERATOER(before_cursor[before_len - 1]))
		    && (before_cursor[before_len - 1] != '(')
		    && (before_cursor[before_len - 1] != 'P')
		    && before_cursor[before_len - 1] != 'C') {
			__calculator_wrong_format_create(CALC_MSG_OP_FIRST);
			break;
		}
		/* input digital after "e", the "x" opeartor will be added automatically */
		if(IS_DIGITAL(before_cursor[before_len - 1])){
			_calc_entry_text_insert(entry, "x");
		}
		_calc_entry_text_insert(entry, op_item->op_name);
		break;

	case OP_SIN:		//sin()
	case OP_COS:		//cos()
	case OP_TAN:		//tan()
	case OP_LOG:		//log()
	case OP_ABS:		//abs()
		if (select_mode) {
			char *selected_str = elm_entry_markup_to_utf8(elm_entry_selection_get(entry));
			if (selected_str) {
				if (calc_select_string_search(selected_str)) {
					evas_event_feed_key_down(e, op_item->op_sym, op_item->op_sym,
						op_item->op_name, op_item->op_sym, 0, NULL);
				} else {
					__calculator_wrong_format_create(CALC_MSG_WRONG_FORMAT);
					calculator_cursor_pos = strlen(calculator_input_str);
				}
				SFREE(selected_str);
			}
			return;
		}
		if (calculator_state == CALCULATOR_CALCULATED) {
			edje_object_signal_emit(_EDJ(ad->edje), "show,input", "");

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
		if (select_mode) {
			char *selected_str = elm_entry_markup_to_utf8(elm_entry_selection_get(entry));
			if (selected_str) {
				if (calc_select_string_search(selected_str)) {
					evas_event_feed_key_down(e, op_item->op_sym, op_item->op_sym, op_item->op_name,
						op_item->op_sym, 0, NULL);
				} else {
					__calculator_wrong_format_create(CALC_MSG_WRONG_FORMAT);
					calculator_cursor_pos = strlen(calculator_input_str);
				}
				SFREE(selected_str);
			}
			return;
		}
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
		if (select_mode) {
			char *selected_str = elm_entry_markup_to_utf8(elm_entry_selection_get(entry));
			if (selected_str) {
				if (calc_select_string_search(selected_str)) {
					evas_event_feed_key_down(e, op_item->op_sym, op_item->op_sym, op_item->op_name,
						op_item->op_sym, 0, NULL);
				} else {
					__calculator_wrong_format_create(CALC_MSG_WRONG_FORMAT);
					calculator_cursor_pos = strlen(calculator_input_str);
				}
				SFREE(selected_str);
			}
			return;
		}
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

	case OP_10X:		//10^
		if (calculator_state == CALCULATOR_CALCULATED) {
			edje_object_signal_emit(_EDJ(ad->edje), "show,input", "");
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
			edje_object_signal_emit(_EDJ(ad->edje), "show,input", "");

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
		if (select_mode) {
			__calculator_wrong_format_create(CALC_MSG_WRONG_FORMAT);
			calculator_cursor_pos = strlen(calculator_input_str);
			return;
		}
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
	CALC_FUN_END();
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
	CALC_FUN_BEG();

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
	} else if (evas_obj == edje_object_part_object_get(obj, "item_10x")) {
		val = OP_10X;
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
	CALC_FUN_END();
	return val;
}

/////////////////////////// input text finish ////////////////////////////

#ifdef __i386__
/**
* @describe
*
*
* @param    evas_obj
* @param    obj
* @return    void
* @exception
*/
static int _calculator_get_input_item_hd(const char *keyname, Evas_Object * obj)
{
	CALC_FUN_BEG();
	int val = 0;
	if (0 == strcmp(keyname, "KP_7")) {
		val = OP_NUM_7;
	} else if (0 == strcmp(keyname, "KP_8")) {
		val = OP_NUM_8;
	} else if (0 == strcmp(keyname, "KP_9")) {
		val = OP_NUM_9;
	} else if (0 == strcmp(keyname, "KP_4")) {
		val = OP_NUM_4;
	} else if (0 == strcmp(keyname, "KP_5")) {
		val = OP_NUM_5;
	} else if (0 == strcmp(keyname, "KP_6")) {
		val = OP_NUM_6;
	} else if (0 == strcmp(keyname, "KP_1")) {
		val = OP_NUM_1;
	} else if (0 == strcmp(keyname, "KP_2")) {
		val = OP_NUM_2;
	} else if (0 == strcmp(keyname, "KP_3")) {
		val = OP_NUM_3;
	} else if (0 == strcmp(keyname, "KP_0")) {
		val = OP_NUM_0;
	}else if (0 == strcmp(keyname, "KP_Decimal")) {
		val = OP_DOT;
	} else if (0 == strcmp(keyname, "Return")) {
		val = OP_EQUAL;
	} else if (0 == strcmp(keyname, "KP_Add")) {
		val = OP_PLUS;
	} else if (0 == strcmp(keyname, "KP_Subtract")) {
		val = OP_MINUS;
	} else if (0 == strcmp(keyname, "minus")) {
		val = OP_MINUS;
	} else if (0 == strcmp(keyname, "KP_Multiply")) {
		val = OP_MULTIPLY;
	} else if (0 == strcmp(keyname, "slash")) {
		val = OP_DIVIDE;
	} else if (0 == strcmp(keyname, "KP_Divide")) {
		val = OP_DIVIDE;
	}else if (0 == strcmp(keyname, "BackSpace")) {
		val = OP_DELETE;
	}
	CALC_FUN_END();
	return val;
}
#endif

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
_calculator_interp(void *data, Evas * e, Evas_Object * evas_obj, void *event_info)
{

	CALC_FUN_BEG();
	int val = 0;
	if (data) {

		Evas_Object *obj = (Evas_Object *) data;
		val = _calculator_get_input_item(evas_obj, obj);
		if (ad->wrong_timer) {
			ecore_timer_del(ad->wrong_timer);
			ad->wrong_timer = NULL;
		}
		switch (val) {
		case OP_DELETE:
			_calc_btn_backspace_clicked(e, ad->input_entry);
			if (ad->calc_timer) {
				ecore_timer_del(ad->calc_timer);
				ad->calc_timer = NULL;
			}
			ad->calc_timer =
			    ecore_timer_add(0.1, (Ecore_Task_Cb)__calculator_delete_long_press, ad->input_entry);
			break;
		case OP_CLEAR:
			edje_object_signal_emit(_EDJ(ad->edje), "show,input", "");
			_calc_entry_clear(ad->input_entry);
			calculator_state = CALCULATOR_WAITING_INPUT;
			break;
		case OP_PLUS:
		case OP_MINUS:
		case OP_MULTIPLY:
		case OP_DIVIDE:
			__calculator_control_normal_func_clicked(e, ad->input_entry, &calc_op_item[val]);
			break;
		case OP_DOT:
			_calc_btn_dot_clicked(ad->input_entry);
			break;
		case OP_PARENTHESIS:
			__calculator_parenthesis_clicked(ad->input_entry);
			break;
		case OP_EQUAL:
			__calculator_op_equal(ad->input_entry);
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
			__calculator_control_panel_number_button_clicked(e, ad->input_entry, &calc_op_item[val]);
			break;
		case OP_PLUS_MINUS:
			__calculator_symbol_negative_clicked(ad->input_entry);
			break;
		case OP_PERCENT...OP_E:
			__calculator_control_functions_button_clicked(e, ad->input_entry, &calc_op_item[val]);
			break;
		default:
			break;
		}
		if (val == OP_DELETE) {
			feedback_play(FEEDBACK_PATTERN_SIP_BACKSPACE);
		} else {
			feedback_play(FEEDBACK_PATTERN_SIP);
		}
	}

	CALC_FUN_END();
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
	int fontsize = 50;
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf),
		 "<align=right><+ font_size=%d><color=#855B11FF>%s",
		 fontsize, wrong_string);
	elm_entry_editable_set(ad->input_entry, EINA_FALSE);//chx add avoid the word splited
	elm_entry_line_wrap_set(ad->input_entry,ELM_WRAP_MIXED);
	elm_entry_input_panel_enabled_set(ad->input_entry, EINA_FALSE);
	elm_entry_entry_set(ad->input_entry, buf);
	elm_entry_cursor_end_set(ad->input_entry);
	elm_object_focus_set(ad->input_entry, EINA_TRUE);
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
	ad->wrong_timer = ecore_timer_add(3, (Ecore_Task_Cb)__calculator_wrong_format_delete, ad->input_entry);
}

/* mouse up for delete button. */
static void
__calculator_delelte_up(void *data, Evas * e, Evas_Object * evas_obj,
			void *event_info)
{
	CALC_FUN_BEG();
	if (ad->calc_timer) {
		ecore_timer_del(ad->calc_timer);
		ad->calc_timer = NULL;
	}
	CALC_FUN_END();
}

/**
* @describe
*   Register clicked callback of the keys on the keypad.
*
* @param    keypad      the keypad
* @return   void
*/
void _calc_view_keypad_cb_register(Evas_Object * keypad)
{
	CALC_FUN_BEG();
	char *key_name[] = {
		"item_per", "item_sqr", "item_fac", "item_c", "item_div",
		    "item_mul", "item_del",
		"item_sin", "item_cos", "item_tan", "item_num7", "item_num8",
		    "item_num9", "item_sub",
		"item_ln", "item_log", "item_1x", "item_num4", "item_num5",
		    "item_num6", "item_plus",
		"item_10x", "item_x2", "item_xy", "item_num1", "item_num2",
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
			evas_object_event_callback_add(item, EVAS_CALLBACK_MOUSE_DOWN, _calculator_interp, (void *)keypad);
			if (0 == strcmp(key_name[i], "item_del")) {
				evas_object_event_callback_add(item,
							       EVAS_CALLBACK_MOUSE_UP,
							       __calculator_delelte_up,
							       (void *)keypad);
			}

		}
	}
	CALC_FUN_END();
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
{	if (data == NULL) {
		return;
	}
	/* if entry is showing warning message, then input operation is unavailable, just return*/
	struct appdata *ad = (struct appdata *)data;
	if (ad->wrong_timer) {
		return;
	}
	calculator_cursor_pos = calc_view_cursor_get_position(entry);	//for  softkey input and mouse move
}

static void
_calc_view_input_entry_to_str(char *entry_str, char *internal_str, int buf_size)
{
	CALC_FUN_BEG();
	if (entry_str == NULL) {
		return;
	}
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
	CALC_FUN_END();
}

#ifdef __i386__
static void
_calc_view_input_entry_keyup_cb(void *data, Evas * e, Evas_Object * entry,
				void *event_info)
{
	CALC_FUN_BEG();
	calculator_cursor_pos = calc_view_cursor_get_position(entry);	//for hardkey input
	int val = 0;
	if (data) {
		Evas_Object *obj = (Evas_Object *) data;
		Evas_Event_Key_Up *evt = (Evas_Event_Key_Up *) event_info;
		if (0 == strcmp(evt->key, "Return")) {
			//calc_expr_input_backspace(calculator_input_str, &calculator_cursor_pos);
			__calculator_op_equal(entry);

		}
		val = _calculator_get_input_item_hd(evt->key, obj);
		switch (val) {
		case OP_PLUS:
		case OP_MINUS:
		case OP_MULTIPLY:
		case OP_DIVIDE:
			__calculator_control_normal_func_clicked(e, ad->input_entry, &calc_op_item[val]);
			break;
		case OP_DOT:
			_calc_btn_dot_clicked(ad->input_entry);
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
			__calculator_control_panel_number_button_clicked(e, ad-> input_entry, &calc_op_item[val]);
			break;
		case OP_DELETE:
			calc_expr_input_backspace(calculator_input_str, &calculator_cursor_pos);
			break;
		default:
			break;
		}
	}
	CALC_FUN_END();
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
* The callback of input entry when text changed
*
* @param[in]	data	the appdata
* @param[in]	obj	the input entry
* @param[in]	event_info	unused
* @return	void
*/
static void
_calc_view_input_entry_changed_cb(void *data, Evas_Object *obj,
				  void *event_info)
{
	CALC_FUN_BEG();
	if (data == NULL || obj == NULL) {
		return;
	}
	struct appdata *ad = (struct appdata *)data;
	const char *str = (char *)elm_entry_entry_get(obj);

	if (paste_flag) {
		char *entry_tmp = elm_entry_markup_to_utf8(str);
		char *entry_expr= elm_entry_markup_to_utf8(entry_tmp);/*because the string format from clipboard is not correct*/
		char internal_expr[MAX_EXPRESSION_LENGTH] = { 0 };
		char f_number_buf[NUMBER_LENGTH] = { 0 };
		char s_number_buf[NUMBER_LENGTH] = { 0 };
		paste_flag = FALSE;
		_calc_view_input_entry_to_str(entry_expr, internal_expr, MAX_EXPRESSION_LENGTH);
		int index = 0;
		bool char_in =__calculator_string_invalid_char_search(internal_expr, &index);
		__calculator_get_cursor_position_float_string(internal_expr, f_number_buf, calculator_cursor_pos, NULL, NULL);
		int cur_pos = calc_view_cursor_get_position(ad->input_entry);
		__calculator_get_cursor_position_float_string(internal_expr, s_number_buf, cur_pos, NULL, NULL);
		int nCntOp = calc_expr_get_operator_num(internal_expr);

		if ((strlen(f_number_buf) > MAX_NUM_LENGTH) || (strlen(s_number_buf) > MAX_NUM_LENGTH)) {
			__calculator_wrong_format_create(CALC_MSG_MAX_DIGIT);
		} else if (nCntOp >= MAX_OPERATOR_NUM) {
			__calculator_wrong_format_create(CALC_MSG_MAX_OP);
		} else {
			if (char_in) {
				strncpy(calculator_input_str, internal_expr, index);
				calculator_cursor_pos = index;
			} else {
				strncpy(calculator_input_str, internal_expr, sizeof(calculator_input_str) - 1);
				calculator_cursor_pos = cur_pos;
			}
			_calc_entry_text_set(ad->input_entry, calculator_input_str);
		}

		if (entry_tmp) {
			free(entry_tmp);
			entry_tmp = NULL;
		}
		if (entry_expr) {
			free(entry_expr);
			entry_expr = NULL;
		}
	} else {
		if (select_mode) {
			char *entry_expr_s = elm_entry_markup_to_utf8(str);
			char internal_expr_s[MAX_EXPRESSION_LENGTH] = { 0 };
			_calc_view_input_entry_to_str(entry_expr_s, internal_expr_s, MAX_EXPRESSION_LENGTH);
			if(!_calc_view_input_entry_error_include(internal_expr_s)){
				/*change calculator_input_str, after cut operation*/
				if(calculator_state == CALCULATOR_CALCULATED){
					calculator_state = CALCULATOR_OPERAND_INPUT;
				}
				strncpy(calculator_input_str, internal_expr_s, MAX_EXPRESSION_LENGTH - 1);
				calculator_cursor_pos = calc_view_cursor_get_position(obj);
			}
			_calc_entry_text_set(obj, calculator_input_str);
			if (entry_expr_s) {
				free(entry_expr_s);
				entry_expr_s = NULL;
			}
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
		if (begin != NULL) {
			char *end = strchr(begin, '>');
			string_remove_at(buf, begin - buf, end - begin + 1);	/* remove "<item...>" */
			elm_entry_entry_set(obj, buf);
		}

		while (pos--) {
			elm_entry_cursor_next(obj);
		}		/* retrieve cursor position */

		calc_view_revise_input_scroller(ad);
	}
	CALC_FUN_END();
}

static void
_calc_view_input_entry_paste_cb(void *data, Evas_Object * obj, void *event_info)
{
	CALC_FUN_BEG();
	paste_flag = TRUE;
	strncpy(calculator_before_paste_str, calculator_input_str,
		MAX_EXPRESSION_LENGTH - 1);
	CALC_FUN_END();
}

static void
_calc_view_input_entry_select_start_cb(void *data, Evas_Object * obj, void *event_info)
{
	CALC_FUN_BEG();
	select_mode = TRUE;
	CALC_FUN_END();
}

static void
_calc_view_input_entry_select_changed_cb(void *data, Evas_Object * obj, void *event_info)
{
	CALC_FUN_BEG();
	CALC_FUN_END();
}

static void
_calc_view_input_entry_select_cleared_cb(void *data, Evas_Object * obj, void *event_info)
{
	CALC_FUN_BEG();
	select_mode = FALSE;
	CALC_FUN_END();
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
	CALC_FUN_BEG();
	Evas_Object *entry = elm_entry_add(ad->edje);
	elm_entry_single_line_set(entry, EINA_FALSE);
	elm_entry_editable_set(entry, EINA_TRUE);
	elm_entry_input_panel_enabled_set(entry, EINA_FALSE);
	if (strlen(calculator_input_str) > 0) {
		_calc_entry_text_set(entry, calculator_input_str);
	} else {
		elm_entry_entry_set(entry, "");
	}
	elm_entry_cnp_mode_set(entry, ELM_CNP_MODE_NO_IMAGE);
	elm_entry_magnifier_disabled_set(entry, EINA_TRUE);
	elm_entry_cursor_end_set(entry);
	elm_object_focus_set(entry, EINA_TRUE);
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
	evas_object_smart_callback_add(entry, "selection,start",
				       _calc_view_input_entry_select_start_cb, ad);
	evas_object_smart_callback_add(entry, "selection,changed",
				       _calc_view_input_entry_select_changed_cb, ad);
	evas_object_smart_callback_add(entry, "selection,cleared",
				       _calc_view_input_entry_select_cleared_cb, ad);
#ifdef __i386__
	evas_object_event_callback_add(entry, EVAS_CALLBACK_KEY_UP,
				       _calc_view_input_entry_keyup_cb, ad);
	evas_object_smart_callback_del (entry, "changed",  _calc_view_input_entry_changed_cb);
#endif
	evas_object_show(entry);
	limit_filter_data.max_char_count = 0;
	limit_filter_data.max_byte_count = 419 + 20;	//19*21+20//result:20
	elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size,
				     &limit_filter_data);
	elm_entry_text_style_user_push(entry, "DEFAULT='right_margin=32'");
	CALC_FUN_END();
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
	CALC_FUN_BEG();
	Evas_Object *scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL,
					EVAS_HINT_FILL);

	evas_object_show(scroller);
	CALC_FUN_END();
	return scroller;
}

/**
* @description
* Create the background
*
* @param[in]	parent	background's parent
* @return		when success return background, return NULL oppositely
* @retval	layout	if success, return the background
* @retval	NULL	if create failed or parent is null, return null
* @exception
*/
static Evas_Object *__create_bg(Evas_Object *parent)
{
	CALC_FUN_BEG();
	if (parent == NULL) {
		return NULL;
	}
	Evas_Object *bg = elm_bg_add(parent);
	if (bg == NULL) {
		return NULL;
	}
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, bg);
	evas_object_show(bg);
	CALC_FUN_END();
	return bg;
}

/**
* @description
* Create the main layout
*
* @param[in]	parent	main layout's parent
* @return		when success return a layout, return NULL oppositely
* @retval	layout	if success, return the main layout
* @retval	NULL	if create failed or parent is null, return null
* @exception
*/
static Evas_Object *__calc_view_create_layout_main(Evas_Object *parent)
{
	CALC_FUN_BEG();

	if (parent == NULL) {
		return NULL;
	}

	Evas_Object *layout = elm_layout_add(parent);
	if (layout == NULL) {
		return NULL;
	}

	elm_layout_theme_set(layout, "layout", "application", "default");

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_object_content_set(parent, layout);

	edje_object_signal_emit(_EDJ(layout), "elm,state,show,indicator",
				"elm");
	evas_object_show(layout);

	CALC_FUN_END();

	return layout;
}

void _btn_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
	CALC_FUN_BEG();
	Evas_Object *win_main = (Evas_Object *) data;
	elm_win_lower(win_main);
	CALC_FUN_END();
}

/**
* @description
*   Callback function of "Clear" Button on naviframe
*
* @param    data      the appdata
* @param    obj
* @param    event_info
* @return   void

*/
static void
__calc_view_clear_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	CALC_FUN_BEG();
	struct appdata *ad = (struct appdata *)data;
	if (ad->more_btn_popup) {
		evas_object_hide(ad->more_btn_popup);
		//ad->more_btn_popup= NULL;
	}
	elm_entry_entry_set(ad->hist_area, "");
	elm_entry_entry_set(ad->input_entry, "");
	_calc_view_clear_histroy(ad->hist_area);
	/* Set input entry as initial state */
	_calc_entry_clear(ad->input_entry);
	calculator_state = CALCULATOR_WAITING_INPUT;
	CALC_FUN_END();
}

Evas_Object *create_toolbar_more_btn(Evas_Object *parent, Evas_Smart_Cb func, void *data)
{
	Evas_Object *btn = elm_button_add(parent);
	if (!btn) {
		return NULL;
	}
	elm_object_style_set(btn, "naviframe/more/default");
	if (func) {
		evas_object_smart_callback_add(btn, "clicked", func, data);
	}
	return btn;
}

static void __gl_ctxpopup_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(obj);
}

static void __calc_view_create_popup(void *data, Evas_Object *parent)
{
	if (data == NULL) {
		return;
	}
	struct appdata *ad = (struct appdata *)data;
	ad->more_btn_popup = elm_ctxpopup_add(parent);
	const char *profile = elm_config_profile_get();
	if (!strcmp(profile, "mobile")) {
		ad->clear_item = elm_ctxpopup_item_append(ad->more_btn_popup,
			CALC_MSG_CLEAR_HISTTORY, NULL, __calc_view_clear_clicked_cb, ad);
	} else if (!strcmp(profile, "desktop")) {
		ad->clear_item = elm_ctxpopup_item_append(ad->more_btn_popup,
			CALC_MSG_CLEAR_HISTTORY,  NULL, __calc_view_clear_clicked_cb, ad);
	}

	Evas_Object *more_btn = NULL;
	more_btn = elm_object_item_part_content_get(ad->navi_it, "toolbar_more_btn");
	if (more_btn) {
		int x_position = 0;
		int y_position = 0;
		int w = 0;
		int h = 0;
		evas_object_geometry_get(more_btn, &x_position, &y_position, &w, &h);
		evas_object_move(ad->more_btn_popup, x_position + w/4, y_position+h/2);
	}

	evas_object_show(ad->more_btn_popup);
	evas_object_smart_callback_add(ad->more_btn_popup, "dismissed",  __gl_ctxpopup_hide_cb, data);

}

void __calc_view_create_nf_more_btn_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	__calc_view_create_popup(data, ad->nf);
}


/**
* @description
*   Load the Naviframe.
*
* @param    ad      the appdata
* @return   naviframe object

*/
static Eina_Bool  __calc_view_create_navigation_layout(struct appdata *ad)
{
	CALC_FUN_BEG();
	Evas_Object *nf = elm_naviframe_add(ad->layout);
	if (nf == NULL) {
		return EINA_FALSE;
	}
	ad->nf = nf;
	if (!ad->back_btn) {
		Evas_Object *back_btn = elm_button_add(nf);
		elm_object_style_set(back_btn, "naviframe/end_btn/default");
		evas_object_smart_callback_add(back_btn, "clicked", _btn_clicked_cb, ad->win);
		ad->back_btn = back_btn;
	}
	ad->navi_it = elm_naviframe_item_push(nf, NULL, ad->back_btn, NULL, ad->edje, NULL);
	elm_naviframe_item_title_visible_set(ad->navi_it, EINA_FALSE);
	ad->more_btn = create_toolbar_more_btn(nf, __calc_view_create_nf_more_btn_cb, ad);
	elm_object_item_part_content_set(ad->navi_it, "toolbar_more_btn", ad->more_btn);
	evas_object_show(nf);
	CALC_FUN_END();
	return EINA_TRUE;
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
	CALC_FUN_BEG();
	if (ad->bg == NULL) {
		ad->bg = __create_bg(ad->win);
	}
	ad->conform = elm_conformant_add(ad->win);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	elm_object_style_set(ad->conform, "nokeypad");
	evas_object_show(ad->conform);
	ad->layout = __calc_view_create_layout_main(ad->conform);
	ad->edje = load_edj(ad->layout, LAYOUT_EDJ_NAME, GRP_MAIN);
	evas_object_show(ad->edje);
	evas_object_name_set(ad->edje, "edje");

	if (__calc_view_create_navigation_layout(ad)) {
		elm_object_part_content_set(ad->layout, "elm.swallow.content", ad->nf);
	}

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
	feedback_initialize();

	/* input area */
	ad->input_entry = _calc_view_create_input_entry(ad->edje, ad);
	ad->input_scroller = _calc_view_create_input_scroller(ad->edje);
	elm_object_content_set(ad->input_scroller, ad->input_entry);
	edje_object_part_swallow(_EDJ(ad->edje), "input/entry",
				 ad->input_scroller);
	CALC_FUN_END();

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

static Evas_Object* __set_win_icon(struct appdata *ad)
{
	/* set window icon*/
	Evas_Object *icon = evas_object_image_add(evas_object_evas_get(ad->win));
	evas_object_image_file_set(icon, DESKTOP_MODE_ICON, NULL);
	elm_win_icon_object_set(ad->win, icon);
	return icon;
}

void win_profile_changed_cb(void *data, Evas_Object * obj, void *event_info)
{
	CALC_FUN_BEG();
	if (data == NULL) {
		return;
	}
	struct appdata *ad = (struct appdata *)data;
	const char *profile = elm_config_profile_get();
	if (!strcmp(profile, "desktop")) {
		/*In destktop mode, Hide indicator & hide back button on the naviframe */
		elm_object_item_part_content_set(ad->navi_it, "prev_btn", NULL);
		elm_object_item_part_content_set(ad->navi_it , "toolbar_button1", NULL);
		elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_HIDE);

		/* Show window Icon */
		if (!ad->window_icon) {
			ad->window_icon = __set_win_icon(ad);
		}

	} else if (!strcmp(profile, "mobile")) {
		/* In mobile phone mode, show indicator and show back button*/
		elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
		Evas_Object *back_btn = elm_button_add(ad->nf);
		elm_object_style_set(back_btn, "naviframe/end_btn/default");
		evas_object_smart_callback_add(back_btn, "clicked", _btn_clicked_cb, ad->win);
		ad->back_btn = back_btn;
		elm_object_item_part_content_set(ad->navi_it, "prev_btn", ad->back_btn);
	}
	CALC_FUN_END();
}
