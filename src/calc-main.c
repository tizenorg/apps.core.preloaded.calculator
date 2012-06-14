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
 

#include <app.h>
#include <dlog.h>
#include <Ecore_X.h>		/* ecore_x_window_size_get */
#include <utilX.h>		/* KEY_END */
#include <svi.h>
#include "calc-main.h"
#include "calc-view.h"

extern char calculator_input_str[];
extern int calculator_cursor_pos;

static int max_fontsize = 67;
extern void calc_xxx(struct appdata *ap);	/* will be removed */

Evas_Object *load_edj(Evas_Object * parent, const char *file, const char *group)
{
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
	return eo;
}

/**
* @describe
*
*
* @param    ad
* @param    angle
* @return    void
* @exception
*/
static void _to_portrait(struct appdata *ad, int angle)
{
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	edje_object_signal_emit(_EDJ(ad->edje), "portrait", "");
	elm_win_rotation_with_resize_set(ad->win, angle);
	elm_win_rotation_with_resize_set(ad->eo, angle);/*when rotating, resize the window eo*/
	max_fontsize = 67;
}

int get_max_fontsize()
{
	return max_fontsize;
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

#if 0
/**
* @describe
*
*
* @param    data
* @return    int
* @exception
*/
static int _capture_idle_image(void *data)
{
	struct appdata *ad = (struct appdata *)data;

	char name[128];
	//snprintf(name, sizeof(name), COM_SAMSUNG_S, PACKAGE);
	snprintf(name, sizeof(name), "com.%s.%s", VENDOR, PACKAGE);
	Evas *evas = evas_object_evas_get(ad->win);

	if (ui_idlecapture_exists(name) == EINA_FALSE) {
		ui_idlecapture_set(evas, name);
	}

	return ECORE_CALLBACK_CANCEL;	//0
}
#endif
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
	struct appdata *ad = (struct appdata *)data;
	elm_object_focus_set(ad->input_entry, EINA_TRUE);	//set focus
	_calc_entry_clear(ad->input_entry);

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
	struct appdata *ad = (struct appdata *)data;
	calc_view_load_in_idle(ad);

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
	Evas_Object *eo;
	int w, h;
	eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
	if (eo) {
		elm_win_title_set(eo, name);
		elm_win_borderless_set(eo, EINA_TRUE);
		evas_object_smart_callback_add(eo, "delete,request", _win_del,
					       NULL);
		ecore_x_window_size_get(ecore_x_window_root_first_get(), &w,
					&h);
		evas_object_resize(eo, w, h);
	}
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
	//ui_bgimg_fini_noti();

	if (ad->popup) {
		evas_object_del(ad->popup);
		ad->popup = NULL;
	}

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

	if (ad->win) {
		evas_object_del(ad->win);
		ad->win = NULL;
	}

	/* delete timer */
	if (ad->calc_timer) {
		ecore_timer_del(ad->calc_timer);
		ad->calc_timer = NULL;
	}
	if (ad->svi_handle) {
		svi_fini(ad->svi_handle);
	}
}

static bool app_launch(void *data)
{
	CONV_FUNC_IN();
	struct appdata *ad = (struct appdata *)data;

	/*	Use elm_theme_extension_add() API before creating any widgets */
	//elm_theme_overlay_add(NULL, CALCULATOR_THEME);
	elm_theme_extension_add(NULL, CALCULATOR_THEME);

	/* main widnow */
	ad->win = _create_win(PACKAGE);
	if (ad->win == NULL) {
		return FALSE;
	}
	evas_object_show(ad->win);

	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);

	/* will be removed */
	calc_xxx(ad);

	/* load main view */
	calc_view_load(ad);
	_to_portrait(ad, 0);

	/* register callback */
	//ecore_idler_add((Ecore_Task_Cb)_capture_idle_image, ad);
	ecore_idler_add((Ecore_Task_Cb) _set_input_entry_focus, ad);
	ecore_idler_add((Ecore_Task_Cb) _load_idle_view, ad);

	CONV_FUNC_OUT();
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
static bool app_create(void *data)
{
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
}

static void app_service(service_h service, void *data)
{
	CONV_FUNC_IN();
	struct appdata *ad = (struct appdata *)data;

	if (ad->win != NULL) { /* calculator has already launced. */
		elm_win_activate(ad->win);
		return;
	}
	app_launch(ad);
	evas_object_show(ad->win);
	CONV_FUNC_OUT();
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
	struct appdata ad;

	app_event_callback_s event_callback;

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.service = app_service;
	event_callback.low_memory = NULL;
	event_callback.low_battery = NULL;
	event_callback.device_orientation = NULL;
	event_callback.language_changed = NULL;
	event_callback.region_format_changed = NULL;

	memset(&ad, 0x0, sizeof(struct appdata));

	return app_efl_main(&argc, &argv, &event_callback, &ad);
}

