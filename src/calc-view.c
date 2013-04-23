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
#include <app.h>
#include <Elementary.h>
#include <stdbool.h>
#include "calc-main.h"
#include "calc-string.h"
#include "calc-expression.h"
#include "calc-view.h"
#include <Ecore_X.h>

extern char decimal_ch;
extern char separator_ch;
extern char calculator_input_str[];
extern int calculator_cursor_pos;

extern void _calc_add_tag(char *string, char *format_string);
extern int cur_fontsize;
extern void _calc_view_keypad_cb_register(Evas_Object * keypad);
extern calculator_state_t calculator_get_state();
static bool calculator_pannel_show = TRUE;

/*
 * BEGIN INPUT SCROLLER REVISE
 *
 * Accturally it needn't these fuction.
 * But scroller lack fuction and has many bugs.
 * So we need to revise it.
 */

/**
* @describe
*   The entry of the program
*
* @param    data
* @return   Eina_Bool
*/
static Eina_Bool  _calc_view_revise_input_scroller_timer_cb(void *data)
{
	CALC_FUN_BEG();
	struct appdata *ad = (struct appdata *)data;
	int child_w, child_h;
	elm_scroller_child_size_get(ad->input_scroller, &child_w, &child_h);
	printf("|| Child size : w=%d h=%d\n", child_w, child_h);
	int scroller_w, scroller_h;
	elm_scroller_region_get(ad->input_scroller, NULL, NULL, &scroller_w,
				 &scroller_h);
	printf("|| Region size : w=%d h=%d\n", scroller_w, scroller_h);
	int region_y = child_h - scroller_h;
	elm_scroller_region_bring_in(ad->input_scroller, 0, region_y,
				       scroller_w, scroller_h);
	CALC_FUN_END();
	return ECORE_CALLBACK_CANCEL;	//0
}


/**
* @describe
*   The entry of the program
*
* @param    ad      the appdata
* @return   void
*/
void  calc_view_revise_input_scroller(struct appdata *ad)
{

	CALC_FUN_BEG();
    /* Because return ECORE_CALLBACK_CANCEL, the timer will be deleted automatically */
    ecore_timer_add(0.05, _calc_view_revise_input_scroller_timer_cb,
		    ad);
	CALC_FUN_END();
}

static Eina_Bool  _calc_view_revise_history_scroller_timer_cb(void *data)
{
	CALC_FUN_BEG();
	struct appdata *ad = (struct appdata *)data;
	int child_w, child_h;
	elm_scroller_child_size_get(ad->hist_scroll, &child_w, &child_h);
	int scroller_w, scroller_h;
	elm_scroller_region_get(ad->hist_scroll, NULL, NULL, &scroller_w, &scroller_h);
	int region_y = child_h - scroller_h;
	elm_scroller_region_bring_in(ad->hist_scroll, 0, region_y, scroller_w, scroller_h);
	CALC_FUN_END();
	return ECORE_CALLBACK_CANCEL;	//0
}


/**
* @describe
*   The entry of the program
*
* @param    ad      the appdata
* @return   void
*/
void  calc_view_revise_history_scroller(struct appdata *ad)
{

	CALC_FUN_BEG();
	/* Because return ECORE_CALLBACK_CANCEL, the timer will be deleted automatically */
	ecore_timer_add(0.05, _calc_view_revise_history_scroller_timer_cb, ad);
	CALC_FUN_END();
}


/* END INPUT SCROLLER REVISE */
void  calc_view_show_result(const char *result, struct appdata *ad)
{
	char buf[MAX_RESULT_LENGTH] = { 0 };
	char temp[MAX_RESULT_LENGTH] = { 0 };
	snprintf(buf, MAX_RESULT_LENGTH,
		  "<br><+ font_size=%d><color=#FF6101FF>=",
		  cur_fontsize);
	snprintf(temp, MAX_RESULT_LENGTH,
		  "<+ font_size=%d><color=#FF6101FF>%s",
		  cur_fontsize, result);
	strncat(buf, temp, strlen(temp));

#if 0
	    elm_entry_entry_set(entry, buf);

#else	/*  */
	    elm_entry_cursor_end_set(ad->input_entry);
	elm_object_focus_set(ad->input_entry, EINA_TRUE);
	elm_entry_entry_insert(ad->input_entry, buf);
	calc_view_revise_input_scroller(ad);

#endif	/*  */
	    evas_object_show(ad->input_entry);
}

/* BEGIN ABOUT CURSOR */

/**
* @description
*   Because display expression and calculate expression are not the same.
*   The display expression has comma or some special character.
*   So we should correct the cursor position for input expression
*
* @param    entry       input entry
* @param    cur_pos     the cursor position in display expression
* @return   int         the cursor position in input expression
*/
static int
_calc_view_cursor_correct_position(Evas_Object * entry, int cur_pos)
{
	char buf[MAX_EXPRESSION_LENGTH] = { 0 };
	char *ss = elm_entry_markup_to_utf8(elm_entry_entry_get(entry));
	strncpy(buf, ss, MAX_EXPRESSION_LENGTH - 1);
	int i = 0, len = cur_pos;
	int count_sqrt = 0;	/* the number of sqrt before cursor */
	int count_comma = 0;	/* the number of comma before cursor */
	int count_br = 0;
	for (i = 0; i < len; ++i) {
		if (buf[i] == separator_ch){
			count_comma++;
		} else if (buf[i] == '\xcf' || buf[i] == '\xc3') {
			len += 1;
		} else if (buf[i] == '\xe2') {
			if (buf[i+2]=='\x92') {
				len += 2;
			} else if (buf[i+2]=='\x9a') {
				len += 2;
				count_sqrt++;
			}
		}else if (buf[i]==10) {
			count_br++;
		}
	}
	SFREE(ss);
	return cur_pos + count_sqrt * 3 - count_comma-count_br;
}

int calc_view_cursor_get_position(Evas_Object * entry)
{
	int pos = elm_entry_cursor_pos_get(entry);
	return _calc_view_cursor_correct_position(entry, pos);
}

void calc_view_cursor_set_position(Evas_Object * entry, int pos)
{
	CALC_FUN_BEG();
	const char *expr =
	    elm_entry_markup_to_utf8(elm_entry_entry_get(entry));
	const char *input = calculator_input_str;
	int i = 0, j = 0;
	int cur_pos = 0;
	while (j < pos){
		if (expr[i] == input[j]){
			cur_pos++;
			i++;
			j++;
		     }

		else if (expr[i] == separator_ch)
			 {
			cur_pos++;
			i++;
			}

		else if (expr[i] == '\xc3' || expr[i] == '\xcf')	/* mutiply/devide or PI */
			 {
			cur_pos++;
			i += 2;
			j++;
			}

		else if (expr[i] == '\xe2' && input[j] == 's')	/* sqrt */
			 {
			cur_pos++;
			i += 3;
			j += 4;
			}

		else if (expr[i] == '\xe2' && input[j] == '-')	/* sqrt */
			 {
			cur_pos++;
			i += 3;
			j ++;
			}

		else if (expr[i] == '*' && input[j] == 'x')
			 {
			cur_pos++;
			i++;
			j++;
			}
		else if(expr[i]==10) {
			cur_pos++;
			i++;
			}

		else
			 {
			break;
			}
		}
	elm_entry_cursor_pos_set(entry, cur_pos);
	elm_object_focus_set(entry, EINA_TRUE);
	SFREE(expr);
	CALC_FUN_END();
}

#ifdef SAVE_HISTORY

/* BEGIN ABOUT HISTORY */
static struct history_item history[MAX_HISTORY_NUM];
static int history_index = 0;

/**
* @descrption
* @Save the current calculate result to history
* @Called by "__calculator_op_equal" function in calculator_edje.c
* @param[in]	item	The history item will be stored
* @return	void
* @exception
*/
void calc_view_save_history(struct history_item *item)
{
	CALC_FUN_BEG();
	history[history_index] = *item;
	history_index = (1 + history_index) % MAX_HISTORY_NUM;
	CALC_FUN_END();
}

/**
* @descrption
* @Show calculate result when switch between protrait/landscape view
* @Called by "_calc_entry_text_set_rotate" function in calculator_edje.c
* @param[in]	entry	The input entry
* @return	void
* @exception
*/
void _calc_view_show_newest_histroy(Evas_Object *entry)
{
	CALC_FUN_BEG();
	if (entry == NULL) {
		return;
	}
	char content[MAX_TAG_EXPRESSION_LENGTH + MAX_RESULT_LENGTH] = { 0 };
	char tag_text[MAX_TAG_EXPRESSION_LENGTH] = { 0 };
	char buf[MAX_RESULT_LENGTH] = { 0 };
	char result_buf[MAX_RESULT_LENGTH] = { 0 };
	char result_format[MAX_RESULT_LENGTH] = { 0 };
	int new_index = (history_index - 1) % MAX_HISTORY_NUM;
	if (strlen(history[new_index].expression) == 0) {
		return;
	}
	_calc_add_tag(history[new_index].expression, tag_text);
	strncat(content, tag_text, strlen(tag_text));
	snprintf(buf, MAX_RESULT_LENGTH,
		  "<br><align=right><+ font_size=%d><color=#FF6101FF>=",
		  cur_fontsize);
	strncat(content, buf, strlen(buf));
	calc_expr_num_format_result(history[new_index].result, result_buf);
	calc_expr_format_expression(result_buf, result_format);
	if (result_buf[0] == '-') {
		string_replace(result_format, "\xe2\x88\x92", "-");
	}
	snprintf(buf, MAX_RESULT_LENGTH,
		  "<align=right><color=#FF6101FF><+ font_size=%d>%s",
		  cur_fontsize, result_format);
	strncat(content, buf, strlen(buf));
	elm_entry_entry_set(entry, content);
	CALC_FUN_END();
}

/**
* @descrption
* @Show calculate history when history view is load
* @Called by "_calc_view_show_history_cb" function
* @param[in]	entry	The history view entry
* @return	void
* @exception
*/
void calc_view_show_histroy(Evas_Object *entry)
{
	CALC_FUN_BEG();
	if (entry == NULL) {
		return;
	}
	char content[(MAX_TAG_EXPRESSION_LENGTH + MAX_TAG_EXPRESSION_LENGTH) *
		      MAX_HISTORY_NUM] = { 0 };

	char tag_text[MAX_TAG_EXPRESSION_LENGTH] = { 0 };
	char buf[MAX_RESULT_LENGTH] = { 0 };
	char result_buf[MAX_RESULT_LENGTH] = { 0 };
	char result_format[MAX_RESULT_LENGTH] = { 0 };
	char input_str[MAX_TAG_EXPRESSION_LENGTH] = { 0 };
	char input_str_tag[MAX_TAG_EXPRESSION_LENGTH] = { 0 };
	int i = 0;
	int j = 0;
	for (i = history_index; i <= history_index + MAX_HISTORY_NUM - 1; i++) {
		j = i % MAX_HISTORY_NUM;
		if (strlen(history[j].expression) > 0) {
			_calc_add_tag(history[j].expression, tag_text);
			strncat(content, tag_text, strlen(tag_text));
			memset(tag_text, 0, sizeof(tag_text));
			snprintf(buf, MAX_RESULT_LENGTH,
				  "<br><align=right><+ font_size=%d><color=#FF6101FF>=",
				  cur_fontsize);
			strncat(content, buf, strlen(buf));
			memset(result_buf, 0, sizeof(result_buf));
			calc_expr_num_format_result(history[j].result, result_buf);
			memset(result_format, 0, sizeof(result_format));
			calc_expr_format_expression(result_buf, result_format);
			if (result_buf[0] == '-') {
				string_replace(result_format, "\xe2\x88\x92", "-");
			}
			snprintf(buf, MAX_RESULT_LENGTH,
				  "<align=right><color=#FF6101FF><+ font_size=%d>%s<br>",
					cur_fontsize, result_format);
			strncat(content, buf, strlen(buf));
		 }
	}
	calculator_state_t tmp_state = calculator_get_state();
	if (tmp_state != CALCULATOR_CALCULATED) {
		calc_expr_format_expression(calculator_input_str, input_str);
		_calc_add_tag(input_str, input_str_tag);
		strncat(content, input_str_tag, strlen(input_str_tag));
	}
	elm_entry_entry_set(entry, content);
	CALC_FUN_END();
}

/**
* @descrption
* @Clear calculate history "Clear History" Button is clicked
* @param[in]	entry	The history view entry
* @return	void
* @exception
*/
void _calc_view_clear_histroy(Evas_Object *entry)
{
	CALC_FUN_BEG();
	if (entry == NULL) {
		return;
	}
	int i = 0;
	for (i = history_index + MAX_HISTORY_NUM - 1; i >= history_index; --i) {
		int j = i % MAX_HISTORY_NUM;
		if (strlen(history[j].expression) > 0) {
			memset(history[j].expression, 0,
				sizeof(history[j].expression));
			history[j].result = 0;
			}
		}

	memset(calculator_input_str, 0, MAX_EXPRESSION_LENGTH);
	calculator_cursor_pos = 0;
	elm_entry_entry_set(entry, "");
	CALC_FUN_END();
}

void _calc_view_show_history_cb
    (void *data, Evas_Object * o, const char *emission, const char *source)
 {
	CALC_FUN_BEG();
	calculator_pannel_show = FALSE;
	struct appdata *ad = (struct appdata *)data;
	ad->panel_show = EINA_FALSE;
	evas_object_hide(ad->input_entry);/*hide the input entry,before showing history area*/
	evas_object_hide(ad->input_scroller);
	edje_object_part_unswallow(_EDJ(ad->edje), ad->input_scroller);
	evas_object_show(ad->hist_area);
	evas_object_show(ad->hist_scroll);
	calc_view_show_histroy(ad->hist_area);
	edje_object_signal_emit(_EDJ(ad->edje), "show,hist", "");
    /* sync lan & por pannel state */
    Evas_Object * pannel =
	    !strcmp(source, "por") ? ad->lan_pannel : ad->por_pannel;
	edje_object_signal_emit(_EDJ(pannel), "pannel,down", source);
	calc_view_revise_history_scroller(ad);
	CALC_FUN_END();
}

void
_calc_view_hide_history_cb(void *data, Evas_Object * o, const char *emission,
			   const char *source)
{
	CALC_FUN_BEG();
	calculator_pannel_show = TRUE;
	struct appdata *ad = (struct appdata *)data;
	ad->panel_show = EINA_TRUE;
	edje_object_signal_emit(_EDJ(ad->edje), "hide,hist", "");
    /* sync lan & por pannel state */
    Evas_Object * pannel =
	    !strcmp(source, "por") ? ad->lan_pannel : ad->por_pannel;
	edje_object_signal_emit(_EDJ(pannel), "pannel,up", source);
	evas_object_hide(ad->hist_area);
	evas_object_hide(ad->hist_scroll);
	evas_object_show(ad->input_scroller);
	evas_object_show(ad->input_entry);  /*show the input entry,after hide history area */
	edje_object_part_swallow(_EDJ(ad->edje), "input/entry",
				 ad->input_scroller);
	elm_object_focus_set(ad->input_entry, EINA_TRUE);	//can't remove show cursor
	CALC_FUN_END();
}

static Evas_Object *
_calc_view_create_history_entry(Evas_Object * parent)
{
    CALC_FUN_BEG();
	Evas_Object * entry = elm_entry_add(parent);
	elm_entry_single_line_set(entry, EINA_FALSE);
	elm_entry_line_wrap_set(entry, ELM_WRAP_WORD);
	elm_entry_editable_set(entry, EINA_FALSE);
	elm_entry_magnifier_disabled_set(entry, EINA_TRUE);
	elm_entry_entry_set(entry, "<align=right>");
	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, 0.0);
	elm_entry_text_style_user_push(entry, "DEFAULT='right_margin=16'");
	evas_object_show(entry);
	CALC_FUN_END();
	return entry;
}

static Evas_Object *
_calc_view_create_history_scroller(Evas_Object * parent)
{
    CALC_FUN_BEG();
	Evas_Object * scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND,
					  EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL,
					 EVAS_HINT_FILL);
	evas_object_show(scroller);
	CALC_FUN_END();
	return scroller;
}


/* END ABOUT HISTORY */

#endif	/*  */
void calc_view_load_in_idle(struct appdata *ad)
{
	CALC_FUN_BEG();
#ifdef SAVE_HISTORY
	    /* history area */
	ad->hist_area = _calc_view_create_history_entry(ad->edje);
	ad->hist_scroll = _calc_view_create_history_scroller(ad->edje);
	elm_object_content_set(ad->hist_scroll, ad->hist_area);
	edje_object_part_swallow(_EDJ(ad->edje), "history/scroll",
				  ad->hist_scroll);
#endif	/*  */
	CALC_FUN_END();
}

/* pannels */
void calc_por_pannel_load(struct appdata *ad)
{
	CALC_FUN_BEG();
	if(ad->por_pannel == NULL)
    {
	 	ad->por_pannel = load_edj(ad->edje, LAYOUT_EDJ_NAME, GRP_POR_PANNEL);
		edje_object_part_swallow(_EDJ(ad->edje), "por_pannel/rect",
				 ad->por_pannel);
		_calc_view_keypad_cb_register(_EDJ(ad->por_pannel));

		edje_object_signal_callback_add(_EDJ(ad->por_pannel), "pannel,down",
						  "por", _calc_view_show_history_cb,
						  ad);
		edje_object_signal_callback_add(_EDJ(ad->por_pannel), "pannel,up",
						  "por", _calc_view_hide_history_cb,
						  ad);

    }
	if(!calculator_pannel_show){
		edje_object_signal_emit(_EDJ(ad->por_pannel), "pannel,down_i", "por");
	}
	CALC_FUN_END();
}

void calc_lans_pannel_load(struct appdata *ad)
{
	CALC_FUN_BEG();
	if(ad->lan_pannel == NULL)
    {
		ad->lan_pannel = load_edj(ad->edje, LAYOUT_EDJ_NAME, GRP_LAN_PANNEL);
		edje_object_part_swallow(_EDJ(ad->edje), "lan_pannel/rect",
					 ad->lan_pannel);
		_calc_view_keypad_cb_register(_EDJ(ad->lan_pannel));
		edje_object_signal_callback_add(_EDJ(ad->lan_pannel), "pannel,down",
					      "lan", _calc_view_show_history_cb,
					      ad);
		edje_object_signal_callback_add(_EDJ(ad->lan_pannel), "pannel,up",
						  "lan", _calc_view_hide_history_cb,
						  ad);
    }
	if(!calculator_pannel_show){
		edje_object_signal_emit(_EDJ(ad->lan_pannel), "pannel,down_i", "lan");
	}
	CALC_FUN_END();
}


