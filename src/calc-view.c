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
 


#include <Elementary.h>
#include <stdbool.h>
#include "calc-main.h"
#include "calc-expression.h"
#include "calc-view.h"
#include <Ecore_X.h>

extern char decimal_ch;
extern char separator_ch;
extern char calculator_input_str[];
extern void _calc_add_tag(char *string, char *format_string);
extern int get_max_fontsize();
extern calculator_state_t calculator_get_state();

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

	    /* Because return ECORE_CALLBACK_CANCEL, the timer will be deleted automatically */
	    ecore_timer_add(0.05, _calc_view_revise_input_scroller_timer_cb,
			    ad);
}

/* END INPUT SCROLLER REVISE */
void  calc_view_show_result(const char *result, struct appdata *ad)
{
	char buf[MAX_RESULT_LENGTH] = { 0 };
	char temp[MAX_RESULT_LENGTH] = { 0 };
	snprintf(buf, MAX_RESULT_LENGTH,
		  "<br><+ font_size=%d><color=#855B11FF><yellow>=</yellow>",
		  get_max_fontsize());
	snprintf(temp, MAX_RESULT_LENGTH,
		  "<+ font_size=%d><color=#000000ff><yellow>%s</yellow>",
		  get_max_fontsize(), result);
	strncat(buf, temp, strlen(temp));

#if 0
	    elm_entry_entry_set(entry, buf);

#else	/*  */
	    elm_entry_cursor_end_set(ad->input_entry);
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
	for (i = 0; i < len; ++i) {
		if (buf[i] == separator_ch){
			count_comma++;
		} else if (buf[i] == '\xcf' || buf[i] == '\xc3') {
			len += 1;
		} else if (buf[i] == '\xe2') {
			len += 2;
			count_sqrt++;
		}
	}
	SFREE(ss);
	return cur_pos + count_sqrt * 3 - count_comma;
}

int calc_view_cursor_get_position(Evas_Object * entry)
{
	int pos = elm_entry_cursor_pos_get(entry);
	return _calc_view_cursor_correct_position(entry, pos);
}

void calc_view_cursor_set_position(Evas_Object * entry, int pos)
{
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

		else if (expr[i] == '*' && input[j] == 'x')
			 {
			cur_pos++;
			i++;
			j++;
			}

		else
			 {
			PLOG("ERROR %s [%c %c]\n", __func__, expr[i],
			      input[j]);
			break;
			}
		}
	elm_entry_cursor_pos_set(entry, cur_pos);
	elm_object_focus_set(entry, EINA_TRUE);
	SFREE(expr);
}

/* END ABOUT CURSOR */

#ifdef SAVE_HISTORY

/* BEGIN ABOUT HISTORY */
static struct history_item history[MAX_HISTORY_NUM];
static int history_index = 0;
void calc_view_save_history(struct history_item item)
{
	CONV_FUNC_IN();
	history[history_index] = item;
	history_index = (1 + history_index) % MAX_HISTORY_NUM;
	CONV_FUNC_OUT();
}
void _calc_view_show_newest_histroy(Evas_Object * entry)
{
	CONV_FUNC_IN();
	char content[MAX_TAG_EXPRESSION_LENGTH + MAX_RESULT_LENGTH] = { 0 };
	char tag_text[MAX_TAG_EXPRESSION_LENGTH] = { 0 };
	char buf[MAX_RESULT_LENGTH] = { 0 };
	int new_index = (history_index - 1) % MAX_HISTORY_NUM;
	_calc_add_tag(history[new_index].expression, tag_text);
	strncat(content, tag_text, strlen(tag_text));
	snprintf(buf, MAX_RESULT_LENGTH,
		  "<br><align=right><+ font_size=%d><color=#855B11FF><yellow>=</yellow>",
		  get_max_fontsize());
	strncat(content, buf, strlen(buf));
	snprintf(buf, MAX_RESULT_LENGTH,
		  "<align=right><color=#000000ff><+ font_size=%d><yellow>%s</yellow><br>",
		  get_max_fontsize(), history[new_index].result);
	strncat(content, buf, strlen(buf));
	elm_entry_entry_set(entry, content);
	CONV_FUNC_OUT();
}
static void _calc_view_show_histroy(Evas_Object * entry)
{
	CONV_FUNC_IN();
	char content[(MAX_TAG_EXPRESSION_LENGTH + MAX_TAG_EXPRESSION_LENGTH) *
		      MAX_HISTORY_NUM] = { 0 };
	char tag_text[MAX_TAG_EXPRESSION_LENGTH] = { 0 };
	char buf[MAX_RESULT_LENGTH] = { 0 };
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
				  "<br><align=right><+ font_size=%d><color=#855B11FF><yellow>=</yellow>",
				  get_max_fontsize());
			strncat(content, buf, strlen(buf));
			snprintf(buf, MAX_RESULT_LENGTH,
				  "<align=right><color=#000000ff><+ font_size=%d><yellow>%s</yellow><br><+ font_size=20> <br>",
				  get_max_fontsize(), history[j].result);
			strncat(content, buf, strlen(buf));
		 }
	}
	calculator_state_t tmp_state = calculator_get_state();
	if (tmp_state != CALCULATOR_CALCULATED){
		calc_expr_format_expression(calculator_input_str, input_str);
		_calc_add_tag(input_str, input_str_tag);
		strncat(content, input_str_tag, strlen(input_str_tag));
	 }
	elm_entry_entry_set(entry, content);
	CONV_FUNC_OUT();
}

void _calc_view_clear_histroy(Evas_Object * entry)
{
	CONV_FUNC_IN();
	int i = 0;
	for (i = history_index + MAX_HISTORY_NUM - 1; i >= history_index; --i) {
		int j = i % MAX_HISTORY_NUM;
		if (strlen(history[j].expression) > 0)
			 {
			memset(history[j].expression, 0,
				sizeof(history[j].expression));
			memset(history[j].result, 0,
				sizeof(history[j].result));
			}
		}
	_calc_entry_clear(entry);
	CONV_FUNC_OUT();
}

static void _calc_view_show_history_cb
    (void *data, Evas_Object * o, const char *emission, const char *source)
 {
	CONV_FUNC_IN();
	struct appdata *ad = (struct appdata *)data;
	evas_object_hide(ad->input_entry);/*hide the input entry,before showing history area*/
	_calc_view_show_histroy(ad->hist_area);
	edje_object_signal_emit(_EDJ(ad->edje), "show,hist", "");

    /* sync lan & por pannel state */
    Evas_Object * pannel =
	    !strcmp(source, "por") ? ad->lan_pannel : ad->por_pannel;
	edje_object_signal_emit(_EDJ(pannel), "pannel,down", source);
	CONV_FUNC_OUT();
}

static void
_calc_view_hide_history_cb(void *data, Evas_Object * o, const char *emission,
			   const char *source)
{
	CONV_FUNC_IN();
	struct appdata *ad = (struct appdata *)data;
	edje_object_signal_emit(_EDJ(ad->edje), "hide,hist", "");

    /* sync lan & por pannel state */
    Evas_Object * pannel =
	    !strcmp(source, "por") ? ad->lan_pannel : ad->por_pannel;
	edje_object_signal_emit(_EDJ(pannel), "pannel,up", source);
	evas_object_show(ad->input_entry);  /*show the input entry,after hide history area */
	elm_object_focus_set(ad->input_entry, EINA_TRUE);	//can't remove show cursor
	CONV_FUNC_OUT();
}

static Evas_Object *
_calc_view_create_history_entry(Evas_Object * parent)
{
	Evas_Object * entry = elm_entry_add(parent);
	elm_entry_single_line_set(entry, EINA_FALSE);
	elm_entry_line_wrap_set(entry, ELM_WRAP_WORD);
	elm_entry_editable_set(entry, EINA_FALSE);
	elm_entry_entry_set(entry, "<align=right>");
	elm_entry_magnifier_disabled_set(entry, EINA_TRUE);
	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, 0.0);
	evas_object_show(entry);
	return entry;
}

static Evas_Object *
_calc_view_create_history_scroller(Evas_Object * parent)
{
	Evas_Object * scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND,
					  EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL,
					 EVAS_HINT_FILL);
	evas_object_show(scroller);
	return scroller;
}


/* END ABOUT HISTORY */

#endif	/*  */
void calc_view_load_in_idle(struct appdata *ad)
{
	CONV_FUNC_IN();
	PLOG("%s run.\n", __func__);

#ifdef SAVE_HISTORY
	    /* history area */
	ad->hist_area = _calc_view_create_history_entry(ad->edje);
	ad->hist_scroll = _calc_view_create_history_scroller(ad->edje);
	elm_object_content_set(ad->hist_scroll, ad->hist_area);
	edje_object_part_swallow(_EDJ(ad->edje), "history/scroll",
				  ad->hist_scroll);

	    /* callbacks */
	    edje_object_signal_callback_add(_EDJ(ad->por_pannel), "pannel,down",
					    "por", _calc_view_show_history_cb,
					    ad);
	edje_object_signal_callback_add(_EDJ(ad->por_pannel), "pannel,up",
					 "por", _calc_view_hide_history_cb,
					 ad);
#endif	/*  */
	    CONV_FUNC_OUT();
}
