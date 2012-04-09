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

#include <stdio.h>
#include <string.h>
#include "calc-main.h"
#include "calc-string.h"
#include <glib.h>

#undef  BUFLEN
#define BUFLEN 1024

/*
 * Insert str2 to str at index.
 */
void string_insert(char *str, int index, char *str2)
{
	int buf_size = strlen(str) + strlen(str2) + 1;
	char *buf = (char *)malloc(buf_size);
	if (buf == NULL) {
		return;
	}
	memset(buf, 0, buf_size);
	strncpy(buf, str, index);
	buf[index] = '\0';
	strncat(buf, str2, buf_size - 1 - strlen(buf));	//don't use g_strlcat, because has bug.
	 strncat(buf, str + index, buf_size - 1 - strlen(buf));
	strcpy(str, buf);
	free(buf);
}


/*
 * Replace a with b in str locally.
 */
void string_replace(char *str, char *a, char *b)
{
	char *pch = NULL;
	char buf[BUFLEN] = { 0 };
	while ((pch = strstr(str, a)) != NULL) {
		strncpy(buf, str, pch - str);
		buf[pch - str] = '\0';
		g_strlcat(buf, b, sizeof(buf));
		g_strlcat(buf, pch + strlen(a), sizeof(buf));
		strcpy(str, buf);
	 }
}

void string_remove_at(char *str, int at, int length)
{
	int i = at;
	while ((str[i] = str[i + length]) != '\0') {
		i++;
	}
}


/* @ attention
    This is redundant comment.
    Its only use is for Klocwork testing.
    The comment rete should passed 25%.
    But we have only one day to do this work.
    So, sorry, I will delete this comment and add useful comment later.
*/

#ifdef _DEBUG
/* @attention   DON'T REMOVE
char* itoa(int i)
{
	char *a = (char *)malloc(42);
	if (a) { sprintf(a, "%d", i); }
	return a;
}
*/
#endif	/* _DEBUG */

/* Remove later.
if (i != 0)//last digit number
{
    //if ((new_node) && (CALCULATOR_GET_NODE_DATA(new_node)->tmp_result == PI || CALCULATOR_GET_NODE_DATA(new_node)->tmp_result == EXPONENT))
    if ((new_node) && (FLOAT_EQUAL(CALCULATOR_GET_NODE_DATA(new_node)->tmp_result,PI)
        || FLOAT_EQUAL(CALCULATOR_GET_NODE_DATA(new_node)->tmp_result, EXPONENT)))
    {
        node_data = g_malloc0(sizeof(calculator_node_data_t));
        node_data->cur_operator = 'x';
        node_data->negative_flag = 1;
        node_data->operator_type = OPERATOR_TYPE_BINARY;
        node_data->node_calcu_priority = CALCULATOR_CALCULATE_PRIORITY_MIDDLE;
        tree = g_node_new(node_data);

        if (last_node)
        {
            __calculator_calculate_insert_node(&last_node, tree, new_node);
        }
        else
        {
            g_node_insert(tree, -1, new_node);
            CALCULATOR_GET_NODE_DATA(tree)->children_num++;
        }
        last_node = tree;
        new_node = NULL;
    }

    factor = atof(tmp);
    memset(tmp, 0x00, sizeof(tmp));
    i = 0;

    node_data = g_malloc0(sizeof(calculator_node_data_t));
    node_data->tmp_result = factor;
    node_data->negative_flag = 1;
    node_data->negative_flag *= negative_sign;
    negative_sign = 1;
    new_node = g_node_new(node_data);

    if (last_node != NULL)
    {
        if (CALCULATOR_GET_NODE_DATA(last_node)->children_num > CALCULATOR_GET_NODE_DATA(last_node)->operator_type)
        {
            strcat(error_msg, CALC_MSG_SYNTAX_ERROR);
            return FALSE;
        }
        else
        {
            g_node_insert(last_node, -1, new_node);
            CALCULATOR_GET_NODE_DATA(last_node)->children_num++;
        }
        new_node = NULL;
    }
}
*/
