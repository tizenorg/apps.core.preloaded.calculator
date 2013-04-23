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

#ifndef __DEF_CALC_VIEW_H_
#define __DEF_CALC_VIEW_H_

#include "calc-main.h"

/**
* @struct	history_item
* @brief	store the calculation history
*/
struct history_item  {
	char expression[MAX_EXPRESSION_LENGTH];	/**<Store the express in string format */
	double result;	/**<Store the calculate result in double format */
};

/**
* @describe
*
*
* @param    ad
* @return    void
* @exception
*/
void calc_view_load(struct appdata *ad);

/**
* @describe
*
*
* @param    ad
* @return    void
* @exception
*/
void calc_view_load_in_idle(struct appdata *ad);

/**
* @describe
* When window changed between phone and monitor
*
* @param[in]	data	The callback data
* @param[in]	obj	The object which register callback
* @param[in]	event_info	The event info which occured
* @return    void
* @exception
*/
void win_profile_changed_cb(void *data, Evas_Object * obj, void *event_info);

/**
* @describe
*
*
* @param    ad
* @return    void
* @exception
*/
void  calc_view_revise_history_scroller(struct appdata *ad);

/**
* @describe
*
*
* @param    ad
* @return    void
* @exception
*/
void calc_view_revise_input_scroller(struct appdata *ad);

/**
* @describe
*
*
* @param    result
* @param    ad
* @return    void
* @exception
*/
//void calc_view_show_result(const char *result, struct appdata *ad, int font_size);
void calc_view_show_result(const char *result, struct appdata *ad);


/**
* @describe
*
*
* @param    entry
* @return    int
* @exception
*/
int calc_view_cursor_get_position(Evas_Object * entry);

/**
* @describe
*
*
* @param    entry
* @param    pos
* @return    void
* @exception
*/
void calc_view_cursor_set_position(Evas_Object * entry, int pos);

/**
* @describe
*
*
* @param    entry
* @param    pos
* @return    void
* @exception
*/
void calc_view_save_history(struct history_item *item);

void calc_view_show_histroy(Evas_Object *entry);
#endif	/* __DEF_CALC_VIEW_H_ */

