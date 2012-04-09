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

#ifndef __DEF_CALC_STRING_H_
#define __DEF_CALC_STRING_H_

#ifdef __cplusplus
extern "C" {

#endif	/* __cplusplus */

/**
* @describe
*
*
* @param    str
* @param    index
* @param    str2
* @return    void
* @exception
*/
	void string_insert(char *str, int index, char *str2);

/**
* @describe
*
*
* @param    str
* @param    a
* @param    b
* @return    void
* @exception
*/
	void string_replace(char *str, char *a, char *b);

/**
* @describe
*
*
* @param    str
* @param    at
* @param    length
* @return    void
* @exception
*/
	void string_remove_at(char *str, int at, int length);

#ifdef _DEBUG
/**
* @describe
*
*
* @param    i
* @param    at
* @return    char*
* @exception
*/

/* DON'T REMOVE
char* itoa(int i);
*/
#endif	/* _DEBUG */

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* __DEF_CALC_STRING_H_ */

