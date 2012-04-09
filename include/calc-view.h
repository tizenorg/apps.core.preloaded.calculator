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

#ifndef __DEF_CALC_VIEW_H_
#define __DEF_CALC_VIEW_H_

#include "calc-main.h"
struct history_item  {
	char expression[MAX_EXPRESSION_LENGTH];
	 char result[MAX_RESULT_LENGTH];
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
*
*
* @param    msg
* @param    ad
* @return    void
* @exception
*/
void calc_view_show_popup(char *msg, struct appdata *ad);

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
void calc_view_save_history(struct history_item item);

#endif	/* __DEF_CALC_VIEW_H_ */

