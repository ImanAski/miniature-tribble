/**
 * @file ui_pages.h
 * @brief LVGL UI page management API.
 *
 * Provides an index-based widget table so protocol commands can address
 * labels, sliders, and buttons by a numeric index without the binder
 * needing to know LVGL internals.
 *
 * All functions must be called from the LVGL task context (i.e. from
 * dm_process() or lv_timer_handler(), never from an ISR).
 */
#ifndef UI_PAGES_H
#define UI_PAGES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialise the UI page system.
 * Creates LVGL pages and registers widgets in the widget table.
 * Must be called after lv_init() and the display driver are ready.
 */
void ui_pages_init(void);

/**
 * @brief Switch to a page by ID.
 * @param page_id  zero-based page index.
 * @return true on success, false if page_id is out of range.
 */
bool ui_pages_show(uint8_t page_id);

/**
 * @brief Set the text of a label widget.
 * @param widget_idx  Widget table index.
 * @param text        Null-terminated string.
 * @return true on success.
 */
bool ui_pages_set_text(uint8_t widget_idx, const char *text);

/**
 * @brief Set the value of a slider widget.
 * @param widget_idx  Widget table index.
 * @param value       New value.
 * @return true on success.
 */
bool ui_pages_set_value(uint8_t widget_idx, int16_t value);

/**
 * @brief Show / hide a widget.
 * @param widget_idx  Widget table index.
 * @param visible     true = visible, false = hidden.
 */
void ui_pages_set_visible(uint8_t widget_idx, bool visible);

/**
 * @brief Enable / disable a widget.
 * @param widget_idx  Widget table index.
 * @param enabled     true = enabled, false = disabled.
 */
void ui_pages_set_enabled(uint8_t widget_idx, bool enabled);

#ifdef __cplusplus
}
#endif

#endif /* UI_PAGES_H */
