/*
*
* Copyright 2012  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.tizenopensource.org/license
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#ifndef __DEF_CALCULATOR_H_
#define __DEF_CALCULATOR_H_

#include <Elementary.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <dlog.h>
#include <app.h>

/* Path & Name */
#define PACKAGE 				"calculator"
#define LAYOUT_EDJ_NAME			EDJDIR"/calculator.edj"
#define CALCULATOR_THEME		EDJDIR"/calculator_theme.edj"

#define GRP_MAIN 				"main"
#define GRP_POR_PANNEL 			"por_pannel"
#define GRP_LAN_PANNEL 			"lan_pannel"

//#define COM_SAMSUNG_S           "com.samsung.%s"

#define SAVE_HISTORY

/* Max */
#define MAX_FONT_SIZE           44
#define NUMBER_LENGTH 1024
#define MAX_EXPRESSION_LENGTH   1024
#define MAX_RESULT_LENGTH       1024
#define MAX_ERROR_MESSAGE_LENGTH  256

//#define MAX_TAG_EXPRESSION_LENGTH 4096
#define MAX_TAG_EXPRESSION_LENGTH 420*48

//#define MAX_RESULT_LENGTH       128

#define MAX_HISTORY_NUM         10
#define MAX_DECIMAL_NUM         5
#define MAX_NUM_LENGTH          15
#define MAX_OPERATOR_NUM        20
#define MAX_PARENTHESES_NUM     64

/* Math */
#define PI  					3.1415926535897932384626433832795
#define EXPONENT				2.718281828459045235360287471352662497757
#define RADIAN_FACTOR 			(PI/180)
#define DEGREEN_FACTOR 			(180/PI)
#define PI_STR  				"3.1415926535897932384626433832795"
#define EXPONENT_STR			"2.718281828459045235360287471352662497757"

/* Function */
#ifndef _EDJ
#define _EDJ(x) (Evas_Object *)elm_layout_edje_get(x)
#endif				/* _EDJ */

#ifndef _
#define _(str) gettext(str)
#endif				/* _ */

#ifndef gettext_noop
#define gettext_noop(str) (str)
#endif				/* gettext_noop */

#ifndef N_
#define N_(str) gettext_noop(str)
#endif				/* N_ */

#define SFREE(var)\
	if(var != NULL){\
		free((void *)var);\
		var = NULL;\
		}\

#define CALCULATOR_CONTENT_LEN					32

#define CALCULATOR_MAX_RESULT_SUM2 				(10e+100)				/**<maximum num of result sum*/
#define CALCULATOR_MIN_RESULT_SUM2 				(-10e+100)				/**<minimum num of result sum*/

/* BEGIN DEBUG LOG MACRO */
#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "CALCULATOR"

#define CALC_INFO(fmt, arg...) LOGD("[%s:%d] "fmt,__FILE__, __LINE__, ##arg);
#define FONT_COLOR_RESET    "\033[0m"
#define FONT_COLOR_RED      "\033[31m"
#define FONT_COLOR_GREEN    "\033[32m"
#define FONT_COLOR_YELLOW   "\033[33m"
#define FONT_COLOR_BLUE     "\033[34m"
#define FONT_COLOR_PURPLE   "\033[35m"
#define FONT_COLOR_CYAN     "\033[36m"
#define FONT_COLOR_GRAY     "\033[37m"

#define CALC_INFO_RED(fmt, arg...) CALC_INFO(FONT_COLOR_RED fmt FONT_COLOR_RESET, ##arg)
#define CALC_INFO_GREEN(fmt, arg...) CALC_INFO(FONT_COLOR_GREEN fmt FONT_COLOR_RESET, ##arg)
#define CALC_INFO_YELLOW(fmt, arg...) CALC_INFO(FONT_COLOR_YELLOW fmt FONT_COLOR_RESET, ##arg)
#define CALC_INFO_BLUE(fmt, arg...) CALC_INFO(FONT_COLOR_BLUE fmt FONT_COLOR_RESET, ##arg)
#define CALC_INFO_PURPLE(fmt, arg...) CALC_INFO(FONT_COLOR_PURPLE fmt FONT_COLOR_RESET, ##arg)
#define CALC_INFO_CYAN(fmt, arg...) CALC_INFO(FONT_COLOR_CYAN fmt FONT_COLOR_RESET, ##arg)
#define CALC_INFO_GRAY(fmt, arg...) CALC_INFO(FONT_COLOR_GRAY fmt FONT_COLOR_RESET, ##arg)


//#define PERFORMANCE_LOG
#ifdef PERFORMANCE_LOG
#define DBG(fmt, arg...) SLOGD("%s(%d) " fmt, __FUNCTION__, __LINE__, ##arg)
#define CALC_FUN_BEG() DBG("START")
#define CALC_FUN_END() DBG("END")
#else
#define CALC_FUN_BEG()
#define CALC_FUN_END()
#endif

/* Help String of UI Guideline */
#define CALC_MSG_MAX_DIGIT		    _("IDS_CCL_POP_UP_TO_15_DIGITS_AVAILABLE")
#define CALC_MSG_MAX_DEC_DIGIT		_("IDS_CCL_POP_UP_TO_5_DECIMALS_AVAILABLE")
#define CALC_MSG_MAX_OP			    _("IDS_CCL_POP_UP_TO_20_OPERATORS_AVAILABLE")
#define CALC_MSG_DIVIDE_BY_ZERO		_("IDS_CCL_POP_UNABLE_TO_DIVIDE_BY_ZERO")
#define CALC_MSG_NUM_FIRST		    _("IDS_CCL_POP_NO_NUMBER_ERROR")
#define CALC_MSG_OUT_OF_RANGE		_("IDS_CCL_POP_ERROR")
#define CALC_MSG_NUM_AFTER_OP		_("IDS_CCL_POP_ENTER_NUMBER_AFTER_OPERATOR")
#define CALC_MSG_INVALID_SQUARE		_("IDS_CCL_BODY_INVALID_INPUT_FOR_SQUARE_ROOT_FUNCTION")
#define CALC_MSG_INVALID_LOG		_("IDS_CCL_BODY_INVALID_INPUT_FOR_LOG_FUNCTION")
#define CALC_MSG_INVALID_FAC		_("IDS_CCL_BODY_NATURAL_NUMBER_ONLY_FOR_X_E_FUNCTION")
#define CALC_MSG_NUM_FIRST_FAC		_("IDS_CCL_BODY_ENTER_NUMBER_BEFORE_INPUTTING_X_E_FUNCTION")
#define CALC_MSG_NUM_FIRST_RECIP	_("IDS_CCL_BODY_ENTER_NUMBER_BEFORE_INPUTTING_1_X_FUNCTION")
#define CALC_MSG_NUM_FIRST_X2		_("IDS_CCL_BODY_ENTER_NUMBER_BEFORE_INPUTTING_X2_FUNCTION")
#define CALC_MSG_NUM_FIRST_XY		_("IDS_CCL_BODY_ENTER_NUMBER_BEFORE_INPUTTING_XY_FUNCTION")

/* Custtom Error Message */
#define CALC_MSG_OP_FIRST		    _("IDS_CCL_POP_NO_OPERATOR_ERROR")
#define CALC_MSG_SYNTAX_ERROR       _("IDS_CCL_POP_SYNTAX_ERROR")
#if 0
#define CALC_MSG_INVALID_LN         _("Invalid input for ln function")
#define CALC_MSG_INVALID_XY         _("Invalid param for x^y")
#define CALC_MSG_INVALID_TAN        _("Invalid param for tan")
#define CALC_MSG_ENTRY_LIMIT        _("Already had decimal in digit")
#endif
#define ARRAY_SIZE(array)  (sizeof(array)/sizeof(array[0]))

struct appdata {
	Evas_Object *win;	//main window
	Evas_Object *bg;
	Evas_Object *layout;
	Evas_Object *edje;
	Evas_Object *eo;

	Evas_Object *input_scroller;
	Evas_Object *input_entry;
	Evas_Object *por_pannel;
	Evas_Object *lan_pannel;
	Evas_Object *btn;

	Ecore_Timer *calc_timer;
	Ecore_Timer *wrong_timer;
	int svi_handle;

	Evas_Object *nf;
	Elm_Object_Item *navi_it;
	Evas_Object *tool_bar;
	Elm_Object_Item *clear_btn;
	Elm_Object_Item *invalid_btn;
#ifdef SAVE_HISTORY
	Evas_Object *hist_scroll;
	Evas_Object *hist_area;
#endif
};

/**
* @describe
*       Load edj file from group to elm_layout.
*
* @param    parent
* @param    file
* @param    group
* @return
* @exception
*/
Evas_Object *load_edj(Evas_Object * parent, const char *file,
		      const char *group);
void _calc_entry_clear(Evas_Object * entry);

#endif				/* __DEF_CALCULATOR_H__ */
