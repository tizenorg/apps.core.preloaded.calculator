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
#include <dlog.h>
#ifndef WAYLAND_PLATFORM
#include <Ecore_X.h>		/* ecore_x_window_size_get */
#include <utilX.h>		/* KEY_END */
#endif
#include <feedback.h>
#include "calc-main.h"
#include "calc-view.h"
#include <vconf.h>

extern char calculator_input_str[];
extern int calculator_cursor_pos;

extern int cur_fontsize;
extern int default_fontsize;
extern int min_len;
extern int max_len;
extern int small_fontsize;
extern int scientific_result_len;

extern void calc_xxx(struct appdata *ap);	/* will be removed */
extern void _calc_entry_text_set_rotate(struct appdata *ad);
extern void calc_por_pannel_load(struct appdata *ad);
extern void calc_lans_pannel_load(struct appdata *ad);
Evas_Object *load_edj(Evas_Object * parent, const char *file, const char *group)
{
	CALC_FUN_BEG();
	Evas_Object *eo;
	int r;
	eo = elm_layout_add(parent);
	if (eo) {
		r = elm_layout_file_set(eo, file, group);
		if (!r) {
			evas_object_del(eo);
			return NULL;
		}
		evas_object_size_hint_weight_set(eo, EVAS_HINT_EXPAND,
						 EVAS_HINT_EXPAND);
	}
	CALC_FUN_END();
	return eo;
}

/**
* @describe
* @When rotate to landscape view (Scientific calculation)
* @param[in]	ad	The appdata
* @param[in]	angle	The rotate angle 90 or 270
* @return	void
* @exception
*/
static void __to_landscape(struct appdata *ad, int angle)
{
	CALC_FUN_BEG();
	if (ad == NULL) {
		return;
	}
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	calc_lans_pannel_load(ad);
	edje_object_signal_emit(_EDJ(ad->edje), "landscape", "");
	elm_win_rotation_with_resize_set(ad->win, angle);
	//elm_win_rotation_with_resize_set(ad->eo, angle);/*when rotating, resize the window eo*/
	default_fontsize = 98;
	cur_fontsize = 98;
	small_fontsize = 98;
	min_len = 30;
	max_len = 37;
	scientific_result_len = 13;
	_calc_entry_text_set_rotate(ad);
	if (!ad->panel_show) {
		calc_view_show_histroy(ad->hist_area);
		edje_object_signal_emit(_EDJ(ad->edje), "show,hist", "");
	}
	CALC_FUN_END();
}

/**
* @describe
* @When rotate to protrait view (Basic calculation)
* @param[in]	ad	The appdata
* @param[in]	angle	The rotate angle 0 or 180
* @return	void
* @exception
*/
static void __to_portrait(struct appdata *ad, int angle)
{
    CALC_FUN_BEG();
	if (ad == NULL) {
		return;
	}
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	calc_por_pannel_load(ad);
	edje_object_signal_emit(_EDJ(ad->edje), "portrait", "");
	elm_win_rotation_with_resize_set(ad->win, angle);
	//elm_win_rotation_with_resize_set(ad->eo, angle);/*when rotating, resize the window eo*/
	default_fontsize = 70;
	cur_fontsize = 70;
	small_fontsize = 58;
	min_len = 13;
	max_len = 16;
	scientific_result_len = 8;
	_calc_entry_text_set_rotate(ad);
	if (!ad->panel_show) {
		calc_view_show_histroy(ad->hist_area);
		edje_object_signal_emit(_EDJ(ad->edje), "show,hist", "");
	}
	CALC_FUN_END();
}

/**
* @describe
*
*
* @param    mode
* @param    data
* @return    int
* @exception
*/
static  void _rotate(app_device_orientation_e  mode, void *data)
{
	CALC_FUN_BEG();
	if (data == NULL) {
		return;
	}
	struct appdata *ad = (struct appdata *)data;

	switch (mode) {
	case APP_DEVICE_ORIENTATION_0:	//[1]0
		__to_portrait(ad, 0);
		break;

	case APP_DEVICE_ORIENTATION_180:	//[2]180
		__to_portrait(ad, 180);
		break;

	case APP_DEVICE_ORIENTATION_270:	//[3]-90
		__to_landscape(ad, 270);
		break;

	case APP_DEVICE_ORIENTATION_90:	//[4]90
		__to_landscape(ad, 90);
		break;

	default:
		break;
	}
		if (!ad->panel_show) {
			calc_view_revise_history_scroller(ad);
		} else {
			calc_view_revise_input_scroller(ad);
		}

	/* When rotate, the size of input scroller will be changed. So it should adjust to cursor position. */
	calc_view_revise_input_scroller(ad);
	CALC_FUN_END();
}

/**
* @describe
*
*
* @param    data
* @param    obj
* @param    event_info
* @return    void
* @exception
*/
static void _win_del(void *data, Evas_Object * obj, void *event_info)
{
	elm_exit();
}

/**
* @describe
*
*
* @param    data
* @return    int
* @exception
*/
static int _set_input_entry_focus(void *data)
{
	CALC_FUN_BEG();
	struct appdata *ad = (struct appdata *)data;
	elm_object_focus_set(ad->input_entry, EINA_TRUE);	//set focus
	_calc_entry_clear(ad->input_entry);
	CALC_FUN_END();
	return ECORE_CALLBACK_CANCEL;	//0
}

/**
* @describe
*
*
* @param    data
* @return    int
* @exception
*/
static int _load_idle_view(void *data)
{
	CALC_FUN_BEG();
	struct appdata *ad = (struct appdata *)data;
	calc_view_load_in_idle(ad);
	CALC_FUN_END();
	return ECORE_CALLBACK_CANCEL;	//0
}

/**
* @describe
*
*
* @param    name
* @return    Evas_Object*
* @exception
*/
static Evas_Object *_create_win(const char *name)
{
    CALC_FUN_BEG();
	Evas_Object *eo;
	int w, h;
	eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
	if (eo) {
		elm_win_title_set(eo, name);
		evas_object_smart_callback_add(eo, "delete,request", _win_del,
					       NULL);
		#ifndef WAYLAND_PLATFORM
		ecore_x_window_size_get(ecore_x_window_root_first_get(), &w,
					&h);
		#endif
		evas_object_resize(eo, w, h);
	}
	CALC_FUN_END();
	return eo;
}

/**
* @describe
*
*
* @param    ad
* @return    void
* @exception
*/
static void _on_exit(struct appdata *ad)
{
	CALC_FUN_BEG();
	if (ad->por_pannel) {
		evas_object_del(ad->por_pannel);
		ad->por_pannel = NULL;
	}

	if (ad->lan_pannel) {
		evas_object_del(ad->lan_pannel);
		ad->lan_pannel = NULL;
	}

	if (ad->input_entry) {
		evas_object_del(ad->input_entry);
		ad->input_entry = NULL;
	}
	if (ad->clear_item) {
		elm_object_item_del(ad->clear_item);
		ad->clear_item = NULL;
	}
	if (ad->more_btn_popup) {
		evas_object_del(ad->more_btn_popup);
		ad->more_btn_popup = NULL;
	}
	if (ad->more_btn) {
		evas_object_del(ad->more_btn);
		ad->more_btn = NULL;
	}
	if (ad->navi_it) {
		elm_object_item_del(ad->navi_it);
		ad->navi_it = NULL;
	}

	if (ad->nf) {
		evas_object_del(ad->nf);
		ad->nf = NULL;
	}
	 if (ad->window_icon) {
		evas_object_del(ad->window_icon);
		ad->window_icon = NULL;
	}
#ifdef SAVE_HISTORY
	if (ad->hist_area) {
		evas_object_del(ad->hist_area);
		ad->hist_area = NULL;
	}

	if (ad->hist_scroll) {
		evas_object_del(ad->hist_scroll);
		ad->hist_scroll = NULL;
	}
#endif

	if (ad->edje) {
		evas_object_del(ad->edje);
		ad->edje = NULL;
	}

	if (ad->layout) {
		evas_object_del(ad->layout);
		ad->layout = NULL;
	}

	if (ad->bg) {
		evas_object_del(ad->bg);
		ad->bg = NULL;
	}

	if (ad->win) {
		evas_object_del(ad->win);
		ad->win = NULL;
	}

	/* delete timer */
	if (ad->calc_timer) {
		ecore_timer_del(ad->calc_timer);
		ad->calc_timer = NULL;
	}
	if (ad->wrong_timer) {
		ecore_timer_del(ad->wrong_timer);
		ad->wrong_timer = NULL;
	}

	feedback_deinitialize();

	CALC_FUN_END();
}

/**
* @describe
*
*
* @param    data
* @return    int
* @exception
*/
static bool app_create(void *data)
{
	CALC_FUN_BEG();
	struct appdata *ad = (struct appdata *)data;

	/*  Use elm_theme_extension_add() API before creating any widgets */
	elm_theme_extension_add(NULL, CALCULATOR_THEME);

	/* main widnow */
	ad->win = _create_win(PACKAGE);
	if (ad->win == NULL) {
		return FALSE;
	}
	evas_object_smart_callback_add(ad->win, "profile,changed", win_profile_changed_cb, ad);
	evas_object_show(ad->win);

	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);

	/* will be removed */
	calc_xxx(ad);

	/* load main view */
	calc_view_load(ad);
	ad->panel_show = EINA_TRUE;

	int  value = 1;
	int ret = vconf_get_bool(VCONFKEY_SETAPPL_ROTATE_LOCK_BOOL, &value);
	if (ret == 0) {
		if (value == 1) {
			_rotate(APP_DEVICE_ORIENTATION_0, ad);
		} else {
			app_device_orientation_e curr = app_get_device_orientation();
			_rotate(curr, ad);
		}
	} else {
		app_device_orientation_e curr = app_get_device_orientation();
		_rotate(curr, ad);
	}
	/* register callback */
	ecore_idler_add((Ecore_Task_Cb) _set_input_entry_focus, ad);
	ecore_idler_add((Ecore_Task_Cb) _load_idle_view, ad);
	CALC_FUN_END();
	return TRUE;		//EXIT_SUCCESS

}

/**
* @describe
*
*
* @param    data
* @return    int
* @exception
*/
static void app_terminate(void *data)
{
	// Release all resources
	struct appdata *ad = (struct appdata *)data;
	_on_exit(ad);
}

/**
* @describe
*
*
* @param    data
* @return    int
* @exception
*/
static void app_pause(void *data)
{
	// Take necessary actions when application becomes invisible
}

/**
* @describe
*
*
* @param    data
* @return    int
* @exception
*/
static void app_resume(void *data)
{
	// Take necessary actions when application becomes visible.
	struct appdata *ad = (struct appdata *)data;
	elm_object_focus_set(ad->input_entry, EINA_TRUE);
}

/**
* @describe
*   The entry of the program
*
* @param    argc
* @param    argv
* @param    int
* @exception
*/
int main(int argc, char *argv[])
{
    CALC_FUN_BEG();
	struct appdata ad;

	app_event_callback_s event_callback;

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.service = NULL;
	event_callback.low_memory = NULL;
	event_callback.low_battery = NULL;
	event_callback.device_orientation = _rotate;
	event_callback.language_changed = NULL;
	event_callback.region_format_changed = NULL;

	memset(&ad, 0x0, sizeof(struct appdata));
	CALC_FUN_END();
	return app_efl_main(&argc, &argv, &event_callback, &ad);
}

