/**
 * @file dm_binder.h
 * @brief Application Binder – bridges the protocol layer and the LVGL UI.
 *
 * Overrides the weak handler stubs defined in dm_protocol.c.
 * Translates validated protocol commands into UI operations on dm_ui_pages.
 */
#ifndef DM_BINDER_H
#define DM_BINDER_H

#include "../core/dm_platform.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialise the application binder.
 *
 * Must be called after dm_init() and after LVGL has been initialised
 * by the board layer.
 *
 * @param plat  Platform interface (same pointer passed to dm_init()).
 */
void dm_binder_init(dm_platform_t *plat);

#ifdef __cplusplus
}
#endif

#endif /* DM_BINDER_H */
