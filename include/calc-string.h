/*
*
* Copyright 2012  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.1 (the License);
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

