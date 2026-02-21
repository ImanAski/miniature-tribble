/**
 * @file ui_pages.c
 * @brief LVGL UI page implementation.
 *
 * This file creates two demo pages:
 *   Page 0: Home    – title label (idx 0), status label (idx 1), OK button (idx 2)
 *   Page 1: Slider demo – label (idx 3), slider (idx 4)
 *
 * Extend this file to add more pages.  Widget callbacks emit protocol
 * events back to the host via dm_packet helpers.
 *
 * NOTE: This file depends on LVGL (lvgl/lvgl.h).  It must only be
 * compiled as part of a board target that provides an LVGL port.
 */
#include "ui_pages.h"
#include "../../core/dm_packet.h"
#include "../../core/dm_config.h"

/* LVGL is provided by the board's CMake target */
#include "lvgl.h"

#include <string.h>
#include <stddef.h>

/* ── Widget registry ─────────────────────────────────────────────────────── */

#define WIDGET_TABLE_SIZE 16

typedef enum {
    WIDGET_LABEL = 0,
    WIDGET_SLIDER,
    WIDGET_BUTTON,
} widget_type_t;

typedef struct {
    lv_obj_t    *obj;
    widget_type_t type;
} widget_entry_t;

static widget_entry_t s_widgets[WIDGET_TABLE_SIZE];
static uint8_t        s_widget_count = 0;

/* Pages */
static lv_obj_t *s_pages[DM_MAX_PAGES];
static uint8_t   s_page_count = 0;
static uint8_t   s_current_page = 0xFF;

/* Platform reference for event callbacks */
static const dm_platform_t *s_plat = NULL; /* Set via ui_pages_set_platform() – optional */

/* ── Internal helpers ─────────────────────────────────────────────────────── */

static uint8_t register_widget(lv_obj_t *obj, widget_type_t type)
{
    if (s_widget_count >= WIDGET_TABLE_SIZE) return 0xFF;
    uint8_t idx = s_widget_count++;
    s_widgets[idx].obj  = obj;
    s_widgets[idx].type = type;
    return idx;
}

/* Button event callback */
static void btn_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    lv_obj_t *btn = lv_event_get_target(e);

    /* Find widget index */
    for (uint8_t i = 0; i < s_widget_count; i++) {
        if (s_widgets[i].obj == btn && s_plat) {
            dm_packet_send_button_pressed(i, s_plat);
            break;
        }
    }
}

/* Slider event callback */
static void slider_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
    lv_obj_t *slider = lv_event_get_target(e);
    int16_t   val    = (int16_t)lv_slider_get_value(slider);

    for (uint8_t i = 0; i < s_widget_count; i++) {
        if (s_widgets[i].obj == slider && s_plat) {
            dm_packet_send_slider_changed(i, val, s_plat);
            break;
        }
    }
}

/* ── Page builders ────────────────────────────────────────────────────────── */

static void build_home_page(lv_obj_t *page)
{
    /* Title label (widget idx 0) */
    lv_obj_t *lbl_title = lv_label_create(page);
    lv_label_set_text(lbl_title, "hmic Display Manager");
    lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 16);
    register_widget(lbl_title, WIDGET_LABEL);

    /* Status label (widget idx 1) */
    lv_obj_t *lbl_status = lv_label_create(page);
    lv_label_set_text(lbl_status, "Waiting for host...");
    lv_obj_align(lbl_status, LV_ALIGN_CENTER, 0, 0);
    register_widget(lbl_status, WIDGET_LABEL);

    /* OK button (widget idx 2) */
    lv_obj_t *btn = lv_btn_create(page);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -16);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "OK");
    register_widget(btn, WIDGET_BUTTON);
}

static void build_slider_page(lv_obj_t *page)
{
    /* Label (widget idx 3) */
    lv_obj_t *lbl = lv_label_create(page);
    lv_label_set_text(lbl, "Adjust value:");
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, -40);
    register_widget(lbl, WIDGET_LABEL);

    /* Slider (widget idx 4) */
    lv_obj_t *slider = lv_slider_create(page);
    lv_obj_align(slider, LV_ALIGN_CENTER, 0, 0);
    lv_slider_set_range(slider, 0, 100);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    register_widget(slider, WIDGET_SLIDER);
}

/* ── Public API ───────────────────────────────────────────────────────────── */

void ui_pages_init(void)
{
    s_widget_count = 0;
    s_page_count   = 0;
    s_current_page = 0xFF;

    /* Page 0: Home */
    lv_obj_t *home = lv_obj_create(NULL);
    s_pages[s_page_count++] = home;
    build_home_page(home);

    /* Page 1: Slider Demo */
    lv_obj_t *slider_page = lv_obj_create(NULL);
    s_pages[s_page_count++] = slider_page;
    build_slider_page(slider_page);

    /* Show home page by default */
    ui_pages_show(0);
}

bool ui_pages_show(uint8_t page_id)
{
    if (page_id >= s_page_count) return false;
    lv_scr_load(s_pages[page_id]);
    s_current_page = page_id;
    return true;
}

bool ui_pages_set_text(uint8_t widget_idx, const char *text)
{
    if (widget_idx >= s_widget_count) return false;
    widget_entry_t *w = &s_widgets[widget_idx];
    if (w->type == WIDGET_LABEL) {
        lv_label_set_text(w->obj, text);
        return true;
    }
    if (w->type == WIDGET_BUTTON) {
        /* Set text on the first child label of the button */
        lv_obj_t *lbl = lv_obj_get_child(w->obj, 0);
        if (lbl) { lv_label_set_text(lbl, text); return true; }
    }
    return false;
}

bool ui_pages_set_value(uint8_t widget_idx, int16_t value)
{
    if (widget_idx >= s_widget_count) return false;
    widget_entry_t *w = &s_widgets[widget_idx];
    if (w->type == WIDGET_SLIDER) {
        lv_slider_set_value(w->obj, value, LV_ANIM_ON);
        return true;
    }
    return false;
}

void ui_pages_set_visible(uint8_t widget_idx, bool visible)
{
    if (widget_idx >= s_widget_count) return;
    if (visible) {
        lv_obj_clear_flag(s_widgets[widget_idx].obj, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(s_widgets[widget_idx].obj, LV_OBJ_FLAG_HIDDEN);
    }
}

void ui_pages_set_enabled(uint8_t widget_idx, bool enabled)
{
    if (widget_idx >= s_widget_count) return;
    if (enabled) {
        lv_obj_clear_state(s_widgets[widget_idx].obj, LV_STATE_DISABLED);
    } else {
        lv_obj_add_state(s_widgets[widget_idx].obj, LV_STATE_DISABLED);
    }
}
