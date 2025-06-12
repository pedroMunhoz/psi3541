#ifndef WIFI_DEBUG_H
#define WIFI_DEBUG_H

#include "project.h"
#include <stdarg.h>

#define WIFI_DEBUG_PORT 3333

void wifi_debug_init(void);
void wifi_debug_printf(const char *format, ...);


#endif // WIFI_DEBUG_H