#ifndef LV_CONF_H
#define LV_CONF_H

#ifndef __ASSEMBLY__
#include <stdint.h>
#endif

/* Use a simple configuration for the simulator */
#define LV_USE_LOG      1
#define LV_LOG_LEVEL    LV_LOG_LEVEL_INFO
#define LV_LOG_PRINTF   1

#define LV_COLOR_DEPTH  32

/* Disable Arm specific optimizations for simulator */
#define LV_USE_DRAW_SW_ASM    0
#define LV_USE_DRAW_SW_HELIUM 0
#define LV_USE_DRAW_SW_NEON   0

#define LV_MEM_SIZE     (128 * 1024U)

#define LV_USE_DISPLAY  1
#define LV_USE_INDEV    1

/* Enable some features needed by our UI */
#define LV_USE_LABEL    1
#define LV_USE_BTN      1
#define LV_USE_SLIDER   1
#define LV_USE_OBJ_ID   0

/* Default display resolution for simulator */
#define LV_HOR_RES_MAX  800
#define LV_VER_RES_MAX  480

#endif /* LV_CONF_H */
